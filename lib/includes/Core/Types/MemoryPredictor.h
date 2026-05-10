#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

#include "Types/String.h"
#include "Types/List.h"
#include "Types/Set.h"
#include "Types/Geo.h"

namespace daikon::core::types {

template <typename T>
struct MemoryPredictor;

template <>
struct MemoryPredictor<StringObject> {
    static int64_t PredictCreate(std::string_view value) {
        return static_cast<int64_t>(sizeof(StringObject) + value.size());
    }

    static int64_t PredictAppend(const StringObject& obj, std::string_view value) {
        return obj.PredictAppendDelta(value);
    }
};

template <>
struct MemoryPredictor<ListObject> {
    static int64_t PredictCreate() {
        return static_cast<int64_t>(sizeof(ListObject));
    }

    static int64_t PredictPush(const ListObject& obj, const std::vector<std::string_view>& values) {
        return obj.PredictPushDelta(values);
    }

    static int64_t PredictPushNew(const std::vector<std::string_view>& values) {
        int64_t delta = static_cast<int64_t>(sizeof(ListObject));
        for (const auto& sv : values) {
            delta += sv.size() + 32;
        }
        return delta;
    }

    static int64_t PredictSet(const ListObject& obj, int64_t index, std::string_view value) {
        return obj.PredictSetDelta(index, value);
    }

    static int64_t PredictInsert(const ListObject& obj, std::string_view value) {
        return obj.PredictInsertDelta(value);
    }
};

template <>
struct MemoryPredictor<SetObject> {
    static int64_t PredictCreate() {
        return static_cast<int64_t>(sizeof(SetObject));
    }

    static int64_t PredictAdd(const SetObject& obj, const std::vector<std::string_view>& members) {
        return obj.PredictAddDelta(members);
    }

    static int64_t PredictAddNew(const std::vector<std::string_view>& members) {
        int64_t delta = static_cast<int64_t>(sizeof(SetObject));
        for (const auto& m : members) {
            delta += m.size() + 64;
        }
        return delta;
    }

    static int64_t PredictMove(const SetObject& obj, std::string_view member) {
        return obj.PredictMoveDelta(member);
    }
};

template <>
struct MemoryPredictor<GeoObject> {
    static int64_t PredictCreate() {
        return static_cast<int64_t>(sizeof(GeoObject));
    }

    static int64_t PredictAdd(std::string_view member) {
        return GeoObject::PredictAddDelta(member);
    }
};

} // namespace daikon::core::types