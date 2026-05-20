#include <cstddef>
#include <cstdint>
#include <expected>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "../../includes/core/DaikonDB.h"
#include "../../includes/core/results.h"
#include "../../includes/core/types/List.h"

namespace daikon {

DbResult<void> DaikonDatabase::Lpush(std::string_view key, const std::vector<std::string_view>& values) {
    if (has_active_del_) db_.ExpireCycle(3);

    auto* obj = db_.Get(key);
    auto* list = obj ? obj->AsList() : nullptr;
    if (obj && !list) return std::unexpected(LogicError::kWrongType);

    if (!list) {
        int64_t delta = PredictKeyOverhead(key) + core::types::ListObject::EstimateCreate() + core::types::ListObject::MemoryView::EstimatePushNew(values);
        if (WillExceedLimit(delta)) return std::unexpected(LogicError::kOom);

        auto new_list = db_.CreateObject<core::types::ListObject>();
        new_list->AsList()->PushLeft(values);
        db_.Set(key, std::move(new_list));
        return {};
    }

    auto view = list->GetMemoryView();
    if (WillExceedLimit(view.EstimatePush(values))) return std::unexpected(LogicError::kOom);
    list->PushLeft(values);
    return {};
}

DbResult<void> DaikonDatabase::Rpush(std::string_view key, const std::vector<std::string_view>& values) {
    if (has_active_del_) db_.ExpireCycle(3);

    auto* obj = db_.Get(key);
    auto* list = obj ? obj->AsList() : nullptr;
    if (obj && !list) return std::unexpected(LogicError::kWrongType);

    if (!list) {
        int64_t delta = PredictKeyOverhead(key) + core::types::ListObject::EstimateCreate() + core::types::ListObject::MemoryView::EstimatePushNew(values);
        if (WillExceedLimit(delta)) return std::unexpected(LogicError::kOom);

        auto new_list = db_.CreateObject<core::types::ListObject>();
        new_list->AsList()->PushRight(values);
        db_.Set(key, std::move(new_list));
        return {};
    }

    auto view = list->GetMemoryView();
    if (WillExceedLimit(view.EstimatePush(values))) return std::unexpected(LogicError::kOom);
    list->PushRight(values);
    return {};
}

DbResult<results::ListViewResult> DaikonDatabase::Lpop(std::string_view key, int64_t count) {
    auto* obj = db_.Get(key);
    if (!obj) return results::ListViewResult{};
    auto* list = obj->AsList();
    if (!list) return std::unexpected(LogicError::kWrongType);
    auto vals = list->PopLeft(static_cast<size_t>(count));
    if (list->values.empty()) db_.Delete(key);
    return results::ListViewResult{std::move(vals)};
}

DbResult<results::ListViewResult> DaikonDatabase::Rpop(std::string_view key, int64_t count) {
    auto* obj = db_.Get(key);
    if (!obj) return results::ListViewResult{};
    auto* list = obj->AsList();
    if (!list) return std::unexpected(LogicError::kWrongType);
    auto vals = list->PopRight(static_cast<size_t>(count));
    if (list->values.empty()) db_.Delete(key);
    return results::ListViewResult{std::move(vals)};
}

DbResult<results::IntResult> DaikonDatabase::Llen(std::string_view key) {
    auto* obj = db_.Get(key);
    if (!obj) return std::unexpected(LogicError::kNotFound);
    auto* list = obj->AsList();
    if (!list) return std::unexpected(LogicError::kWrongType);
    return list->Len();
}

DbResult<results::ListViewResult> DaikonDatabase::Lrange(std::string_view key, int64_t start, int64_t stop) {
    auto* obj = db_.Get(key);
    if (!obj) return std::unexpected(LogicError::kNotFound);
    auto* list = obj->AsList();
    if (!list) return std::unexpected(LogicError::kWrongType);
    auto views = list->GetRange(start, stop);

    std::vector<std::string> elements;
    elements.reserve(views.size());
    for (auto sv : views)
        elements.emplace_back(sv);
    return results::ListViewResult{std::move(elements)};
}

DbResult<results::ViewResult> DaikonDatabase::Lindex(std::string_view key, int64_t index) {
    auto* obj = db_.Get(key);
    if (!obj) return std::unexpected(LogicError::kNotFound);
    auto* list = obj->AsList();
    if (!list) return std::unexpected(LogicError::kWrongType);
    auto opt = list->GetByIndex(index);
    if (!opt) return std::unexpected(LogicError::kNotFound);
    return *opt;
}

DbResult<void> DaikonDatabase::Lset(std::string_view key, int64_t index, std::string_view value) {
    if (has_active_del_) db_.ExpireCycle(3);

    auto* obj = db_.Get(key);
    if (!obj) return std::unexpected(LogicError::kNotFound);
    auto* list = obj->AsList();
    if (!list) return std::unexpected(LogicError::kWrongType);
    auto view = list->GetMemoryView();
    if (WillExceedLimit(view.EstimateSet(index, value))) return std::unexpected(LogicError::kOom);
    list->Set(index, value);
    return {};
}

DbResult<results::IntResult> DaikonDatabase::Linsert(std::string_view key, bool before,
                                                     std::string_view pivot, std::string_view value) {
    if (has_active_del_) db_.ExpireCycle(3);

    auto* obj = db_.Get(key);

    if (!obj) return std::unexpected(LogicError::kNotFound);
    auto* list = obj->AsList();

    if (!list) return std::unexpected(LogicError::kWrongType);
    auto view = list->GetMemoryView();

    if (WillExceedLimit(view.EstimateInsert(value))) return std::unexpected(LogicError::kOom);

    int64_t result = list->Insert(before, pivot, value);
    if (result == -1) return std::unexpected(LogicError::kNotFound);
    return result;
}

} // namespace daikon