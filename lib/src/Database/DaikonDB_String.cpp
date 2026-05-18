#include <cstdint>
#include <expected>
#include <string>
#include <string_view>

#include "../../includes/core/DaikonDB.h"
#include "../../includes/core/results.h"
#include "../../includes/core/types/String.h"

namespace daikon {

DbResult<results::StatusResult> DaikonDatabase::Set(std::string_view key, std::string_view value) {
    auto* obj = db_.Get(key);
    auto* str = obj ? obj->AsString() : nullptr;

    if (obj && !str) return std::unexpected(LogicError::kWrongType);

    int64_t delta = PredictKeyOverhead(key) + core::types::StringObject::EstimateCreate(value);
    if (WillExceedLimit(delta)) return std::unexpected(LogicError::kOom);

    db_.Set(key, db_.CreateObject<core::types::StringObject>(value));
    return true;
}

DbResult<results::ViewResult> DaikonDatabase::Get(std::string_view key) {
    auto* obj = db_.Get(key);
    if (!obj) return std::unexpected(LogicError::kNotFound);
    auto* str = obj->AsString();
    if (!str) return std::unexpected(LogicError::kWrongType);
    return str->Get();
}

DbResult<results::IntResult> DaikonDatabase::Strlen(std::string_view key) {
    auto* obj = db_.Get(key);
    if (!obj) return std::unexpected(LogicError::kNotFound);
    auto* str = obj->AsString();
    if (!str) return std::unexpected(LogicError::kWrongType);
    return str->Strlen();
}

DbResult<void> DaikonDatabase::Append(std::string_view key, std::string_view value) {
    auto* obj = db_.Get(key);
    auto* str = obj ? obj->AsString() : nullptr;
    if (obj && !str) return std::unexpected(LogicError::kWrongType);

    if (!str) {
        int64_t delta = PredictKeyOverhead(key) + core::types::StringObject::EstimateCreate(value);
        if (WillExceedLimit(delta)) return std::unexpected(LogicError::kOom);
        db_.Set(key, db_.CreateObject<core::types::StringObject>(value));
        return {};
    }

    auto view = str->GetMemoryView();
    if (WillExceedLimit(view.EstimateAppend(value))) return std::unexpected(LogicError::kOom);
    str->Append(value);
    return {};
}

} // namespace daikon