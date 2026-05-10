#include "DaikonDB.h"

namespace daikon {

DbResult<void> DaikonDataBase::Lpush(std::string_view key, const std::vector<std::string_view>& values) {
    auto* object = db_.Get(key);
    if (object && object->GetType() != core::types::DataType::kList) {
        return std::unexpected(LogicError::kWrongType);
    }
    if (!object) {
        int64_t delta = PredictKeyOverhead(key) + ListPredictor::PredictPushNew(values);
        if (WillExceedLimit(delta)) {
            return std::unexpected(LogicError::kOom);
        }
        auto new_list = core::types::DataObject::CreateList(&db_);
        new_list.AsList().PushLeft(values);
        db_.Set(key, std::move(new_list));
    } else {
        auto& list = object->AsList();
        if (WillExceedLimit(ListPredictor::PredictPush(list, values))) {
            return std::unexpected(LogicError::kOom);
        }
        list.PushLeft(values);
    }
    return {};
}

DbResult<void> DaikonDataBase::Rpush(std::string_view key, const std::vector<std::string_view>& values) {
    auto* object = db_.Get(key);
    if (object && object->GetType() != core::types::DataType::kList) {
        return std::unexpected(LogicError::kWrongType);
    }
    if (!object) {
        int64_t delta = PredictKeyOverhead(key) + ListPredictor::PredictPushNew(values);
        if (WillExceedLimit(delta)) {
            return std::unexpected(LogicError::kOom);
        }
        auto new_list = core::types::DataObject::CreateList(&db_);
        new_list.AsList().PushRight(values);
        db_.Set(key, std::move(new_list));
    } else {
        auto& list = object->AsList();
        if (WillExceedLimit(ListPredictor::PredictPush(list, values))) {
            return std::unexpected(LogicError::kOom);
        }
        list.PushRight(values);
    }
    return {};
}

DbResult<results::ListViewResult> DaikonDataBase::Lpop(std::string_view key, int64_t count) {
    auto* object = db_.Get(key);
    if (!object) {
        return results::ListViewResult{};
    }
    if (object->GetType() != core::types::DataType::kList) {
        return std::unexpected(LogicError::kWrongType);
    }
    auto& list_obj = object->AsList();
    auto val = list_obj.PopLeft(static_cast<size_t>(count));
    if (list_obj.values.empty()) {
        db_.Delete(key);
    }
    return results::ListViewResult{std::move(val)};
}

DbResult<results::ListViewResult> DaikonDataBase::Rpop(std::string_view key, int64_t count) {
    auto* object = db_.Get(key);
    if (!object) {
        return results::ListViewResult{};
    }
    if (object->GetType() != core::types::DataType::kList) {
        return std::unexpected(LogicError::kWrongType);
    }
    auto& list_obj = object->AsList();
    auto val = list_obj.PopRight(static_cast<size_t>(count));
    if (list_obj.values.empty()) {
        db_.Delete(key);
    }
    return results::ListViewResult{std::move(val)};
}

DbResult<results::IntResult> DaikonDataBase::Llen(std::string_view key) {
    auto* object = db_.Get(key);
    if (!object) {
        return std::unexpected(LogicError::kNotFound);
    }
    if (object->GetType() != core::types::DataType::kList) {
        return std::unexpected(LogicError::kWrongType);
    }
    return results::IntResult{static_cast<int64_t>(object->AsList().Len())};
}

DbResult<results::ListViewResult> DaikonDataBase::Lrange(std::string_view key, int64_t start, int64_t stop) {
    auto* object = db_.Get(key);
    if (!object) {
        return std::unexpected(LogicError::kNotFound);
    }
    if (object->GetType() != core::types::DataType::kList) {
        return std::unexpected(LogicError::kWrongType);
    }
    auto views = object->AsList().GetRange(start, stop);
    std::vector<std::string> elements;
    elements.reserve(views.size());
    for (auto sv : views) {
        elements.emplace_back(sv);
    }
    return results::ListViewResult{std::move(elements)};
}

DbResult<results::ViewResult> DaikonDataBase::Lindex(std::string_view key, int64_t index) {
    auto* object = db_.Get(key);
    if (!object) {
        return std::unexpected(LogicError::kNotFound);
    }
    if (object->GetType() != core::types::DataType::kList) {
        return std::unexpected(LogicError::kWrongType);
    }
    auto opt = object->AsList().GetByIndex(index);
    if (!opt.has_value()) {
        return std::unexpected(LogicError::kNotFound);
    }
    return results::ViewResult{std::string(*opt)};
}

DbResult<void> DaikonDataBase::Lset(std::string_view key, int64_t index, std::string_view value) {
    auto* object = db_.Get(key);
    if (!object) {
        return std::unexpected(LogicError::kNotFound);
    }
    if (object->GetType() != core::types::DataType::kList) {
        return std::unexpected(LogicError::kWrongType);
    }
    auto& list = object->AsList();
    if (WillExceedLimit(ListPredictor::PredictSet(list, index, value))) {
        return std::unexpected(LogicError::kOom);
    }
    list.Set(index, value);
    return {};
}

DbResult<results::IntResult> DaikonDataBase::Linsert(std::string_view key, bool before,
                                                     std::string_view pivot, std::string_view value) {
    auto* object = db_.Get(key);
    if (!object) {
        return std::unexpected(LogicError::kNotFound);
    }
    if (object->GetType() != core::types::DataType::kList) {
        return std::unexpected(LogicError::kWrongType);
    }
    auto& list = object->AsList();
    if (WillExceedLimit(ListPredictor::PredictInsert(list, value))) {
        return std::unexpected(LogicError::kOom);
    }
    int64_t result = list.Insert(before, pivot, value);
    if (result == -1) {
        return std::unexpected(LogicError::kNotFound);
    }
    return results::IntResult{result};
}

} // namespace daikon