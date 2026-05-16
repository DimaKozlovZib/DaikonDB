#include "../../includes/core/DaikonDB.h"
#include "../../includes/core/types/Set.h"

namespace daikon {

DbResult<results::IntResult> DaikonDatabase::Sadd(std::string_view key,
                                                  const std::vector<std::string_view>& members) {
    auto* obj = db_.Get(key);
    auto* set = obj ? obj->AsSet() : nullptr;
    if (obj && !set) return std::unexpected(LogicError::kWrongType);

    if (!set) {
        int64_t delta = PredictKeyOverhead(key) + core::types::SetObject::EstimateCreate() + core::types::SetObject::MemoryView::EstimateAddNew(members);
        if (WillExceedLimit(delta)) return std::unexpected(LogicError::kOom);

        auto new_set = db_.CreateObject<core::types::SetObject>();
        size_t added = new_set->AsSet()->Add(members);
        db_.Set(key, std::move(new_set));
        return results::IntResult{static_cast<int64_t>(added)};
    }

    auto view = set->GetMemoryView();
    if (WillExceedLimit(view.EstimateAdd(members))) return std::unexpected(LogicError::kOom);
    size_t added = set->Add(members);
    return results::IntResult{static_cast<int64_t>(added)};
}

DbResult<results::IntResult> DaikonDatabase::Srem(std::string_view key,
                                                  const std::vector<std::string_view>& members) {
    auto* obj = db_.Get(key);
    if (!obj) return results::IntResult{0};
    auto* set = obj->AsSet();
    if (!set) return std::unexpected(LogicError::kWrongType);
    size_t removed = set->Remove(members);
    return results::IntResult{static_cast<int64_t>(removed)};
}

DbResult<results::IntResult> DaikonDatabase::Sismember(std::string_view key,
                                                       std::string_view member) {
    auto* obj = db_.Get(key);
    if (!obj) return results::IntResult{0};
    auto* set = obj->AsSet();
    if (!set) return std::unexpected(LogicError::kWrongType);
    return results::IntResult{set->IsMember(member) ? 1 : 0};
}

DbResult<results::ListViewResult> DaikonDatabase::Smembers(std::string_view key) {
    auto* obj = db_.Get(key);
    if (!obj) return std::unexpected(LogicError::kNotFound);
    auto* set = obj->AsSet();
    if (!set) return std::unexpected(LogicError::kWrongType);
    return results::ListViewResult{set->AllMembers()};
}

DbResult<results::IntResult> DaikonDatabase::Scard(std::string_view key) {
    auto* obj = db_.Get(key);
    if (!obj) return std::unexpected(LogicError::kNotFound);
    auto* set = obj->AsSet();
    if (!set) return std::unexpected(LogicError::kWrongType);
    return results::IntResult{static_cast<int64_t>(set->Card())};
}

DbResult<results::ListViewResult> DaikonDatabase::Sunion(const std::vector<std::string_view>& keys) {
    std::vector<core::types::SetObject*> sets;
    for (const auto& key : keys) {
        auto* obj = db_.Get(key);
        if (obj) {
            auto* set = obj->AsSet();
            if (!set) return std::unexpected(LogicError::kWrongType);
            sets.push_back(set);
        }
    }
    if (sets.empty()) return results::ListViewResult{};
    return results::ListViewResult{core::types::SetObject::UnionSets(sets)};
}

DbResult<results::ListViewResult> DaikonDatabase::Sinter(const std::vector<std::string_view>& keys) {
    if (keys.empty()) return results::ListViewResult{};
    std::vector<core::types::SetObject*> sets;
    for (const auto& key : keys) {
        auto* obj = db_.Get(key);
        if (!obj) return results::ListViewResult{};
        auto* set = obj->AsSet();
        if (!set) return std::unexpected(LogicError::kWrongType);
        sets.push_back(set);
    }
    return results::ListViewResult{core::types::SetObject::InterSets(sets)};
}

DbResult<results::ListViewResult> DaikonDatabase::Sdiff(const std::vector<std::string_view>& keys) {
    if (keys.empty()) return results::ListViewResult{};
    std::vector<core::types::SetObject*> sets;
    for (const auto& key : keys) {
        auto* obj = db_.Get(key);
        if (!obj) {
            if (sets.empty()) return results::ListViewResult{};
            continue;
        }
        auto* set = obj->AsSet();
        if (!set) return std::unexpected(LogicError::kWrongType);
        sets.push_back(set);
    }
    if (sets.empty()) return results::ListViewResult{};
    return results::ListViewResult{core::types::SetObject::DiffSets(sets)};
}

DbResult<results::StatusResult> DaikonDatabase::Smove(std::string_view source, std::string_view destination,
                                                      std::string_view member) {
    auto* src = db_.Get(source);
    auto* dst = db_.Get(destination);

    if (!src || !dst) return results::StatusResult{false};
    auto* src_set = src->AsSet();
    auto* dst_set = dst->AsSet();

    if (!src_set || !dst_set) return std::unexpected(LogicError::kWrongType);
    auto view = dst_set->GetMemoryView();

    if (WillExceedLimit(view.EstimateMove(member))) return std::unexpected(LogicError::kOom);
    return results::StatusResult{src_set->MoveMember(*dst_set, member)};
}

} // namespace daikon