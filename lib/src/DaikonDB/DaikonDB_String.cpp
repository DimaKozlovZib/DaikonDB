#include "DaikonDB.h"

namespace daikon {

DbResult<results::StatusResult> DaikonDataBase::Set(std::string_view key, std::string_view value) {
    int64_t delta = PredictKeyOverhead(key) + StringPredictor::PredictCreate(value);
    if (WillExceedLimit(delta)) {
        return std::unexpected(LogicError::kOom);
    }
    db_.Set(std::string(key), core::types::DataObject::CreateString(&db_, value));
    return results::StatusResult{true};
}

DbResult<results::ViewResult> DaikonDataBase::Get(std::string_view key) {
    auto* object = db_.Get(key);
    if (!object) {
        return std::unexpected(LogicError::kNotFound);
    }
    if (object->GetType() != core::types::DataType::kString) {
        return std::unexpected(LogicError::kWrongType);
    }
    return results::ViewResult{object->AsString().Get()};
}

DbResult<results::IntResult> DaikonDataBase::Strlen(std::string_view key) {
    auto* object = db_.Get(key);
    if (!object) {
        return std::unexpected(LogicError::kNotFound);
    }
    if (object->GetType() != core::types::DataType::kString) {
        return std::unexpected(LogicError::kWrongType);
    }
    return results::IntResult{static_cast<int64_t>(object->AsString().Strlen())};
}

DbResult<void> DaikonDataBase::Append(std::string_view key, std::string_view value) {
    auto* object = db_.Get(key);
    if (!object) {
        int64_t delta = PredictKeyOverhead(key) + StringPredictor::PredictCreate(value);
        if (WillExceedLimit(delta)) {
            return std::unexpected(LogicError::kOom);
        }
        db_.Set(key, core::types::DataObject::CreateString(&db_, value));
        return {};
    }
    if (object->GetType() != core::types::DataType::kString) {
        return std::unexpected(LogicError::kWrongType);
    }
    auto& str = object->AsString();
    if (WillExceedLimit(StringPredictor::PredictAppend(str, value))) {
        return std::unexpected(LogicError::kOom);
    }
    str.Append(value);
    return {};
}

} // namespace daikon