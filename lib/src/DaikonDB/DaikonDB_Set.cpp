#include "DaikonDB.h"

namespace daikon {

DbResult<results::IntResult> DaikonDataBase::Sadd(std::string_view key,
                                                  const std::vector<std::string_view>& members) {
    auto* object = db_.Get(key);
    if (object && object->GetType() != core::types::DataType::kSet) {
        return std::unexpected(LogicError::kWrongType);
    }
    if (!object) {
        if (WillExceedLimit(PredictKeyOverhead(key) + SetPredictor::PredictAddNew(members))) {
            return std::unexpected(LogicError::kOom);
        }
        auto new_set = core::types::DataObject::CreateSet(&db_);
        size_t added = new_set.AsSet().Add(members);
        db_.Set(key, std::move(new_set));
        return results::IntResult{static_cast<int64_t>(added)};
    } else {
        auto& set = object->AsSet();
        if (WillExceedLimit(SetPredictor::PredictAdd(set, members))) {
            return std::unexpected(LogicError::kOom);
        }
        size_t added = set.Add(members);
        return results::IntResult{static_cast<int64_t>(added)};
    }
}

DbResult<results::IntResult> DaikonDataBase::Srem(std::string_view key,
                                                  const std::vector<std::string_view>& members) {
    auto* object = db_.Get(key);
    if (!object) {
        return results::IntResult{0};
    }
    if (object->GetType() != core::types::DataType::kSet) {
        return std::unexpected(LogicError::kWrongType);
    }
    size_t removed = object->AsSet().Remove(members);
    return results::IntResult{static_cast<int64_t>(removed)};
}

DbResult<results::IntResult> DaikonDataBase::Sismember(std::string_view key,
                                                       std::string_view member) {
    auto* object = db_.Get(key);
    if (!object) {
        return results::IntResult{0};
    }
    if (object->GetType() != core::types::DataType::kSet) {
        return std::unexpected(LogicError::kWrongType);
    }
    bool exists = object->AsSet().IsMember(member);
    return results::IntResult{exists ? 1 : 0};
}

DbResult<results::ListViewResult> DaikonDataBase::Smembers(std::string_view key) {
    auto* object = db_.Get(key);
    if (!object) {
        return std::unexpected(LogicError::kNotFound);
    }
    if (object->GetType() != core::types::DataType::kSet) {
        return std::unexpected(LogicError::kWrongType);
    }
    return results::ListViewResult{object->AsSet().AllMembers()};
}

DbResult<results::IntResult> DaikonDataBase::Scard(std::string_view key) {
    auto* object = db_.Get(key);
    if (!object) {
        return std::unexpected(LogicError::kNotFound);
    }
    if (object->GetType() != core::types::DataType::kSet) {
        return std::unexpected(LogicError::kWrongType);
    }
    return results::IntResult{static_cast<int64_t>(object->AsSet().Card())};
}

DbResult<results::ListViewResult> DaikonDataBase::Sunion(const std::vector<std::string_view>& keys) {
    std::vector<core::types::SetObject*> sets;
    for (const auto& key : keys) {
        auto* obj = db_.Get(key);
        if (obj) {
            if (obj->GetType() != core::types::DataType::kSet) {
                return std::unexpected(LogicError::kWrongType);
            }
            sets.push_back(&obj->AsSet());
        }
    }
    if (sets.empty()) {
        return results::ListViewResult{};
    }
    return results::ListViewResult{core::types::SetObject::UnionSets(sets)};
}

DbResult<results::ListViewResult> DaikonDataBase::Sinter(const std::vector<std::string_view>& keys) {
    if (keys.empty()) {
        return results::ListViewResult{};
    }
    std::vector<core::types::SetObject*> sets;
    for (const auto& key : keys) {
        auto* obj = db_.Get(key);
        if (!obj) {
            return results::ListViewResult{};
        }
        if (obj->GetType() != core::types::DataType::kSet) {
            return std::unexpected(LogicError::kWrongType);
        }
        sets.push_back(&obj->AsSet());
    }
    return results::ListViewResult{core::types::SetObject::InterSets(sets)};
}

DbResult<results::ListViewResult> DaikonDataBase::Sdiff(const std::vector<std::string_view>& keys) {
    if (keys.empty()) {
        return results::ListViewResult{};
    }
    std::vector<core::types::SetObject*> sets;
    for (const auto& key : keys) {
        auto* obj = db_.Get(key);
        if (!obj) {
            if (sets.empty()) {
                return results::ListViewResult{};
            }
            continue;
        }
        if (obj->GetType() != core::types::DataType::kSet) {
            return std::unexpected(LogicError::kWrongType);
        }
        sets.push_back(&obj->AsSet());
    }
    if (sets.empty()) {
        return results::ListViewResult{};
    }
    return results::ListViewResult{core::types::SetObject::DiffSets(sets)};
}

DbResult<results::StatusResult> DaikonDataBase::Smove(std::string_view source, std::string_view destination,
                                                      std::string_view member) {
    auto* src = db_.Get(source);
    auto* dst = db_.Get(destination);
    if (!src || !dst) return results::StatusResult{false};
    if (src->GetType() != core::types::DataType::kSet || dst->GetType() != core::types::DataType::kSet)
        return results::StatusResult{false};
    if (WillExceedLimit(SetPredictor::PredictMove(dst->AsSet(), member))) {
        return results::StatusResult{false};
    }
    return results::StatusResult{src->AsSet().MoveMember(dst->AsSet(), member)};
}

} // namespace daikon