#pragma once

#include <chrono>
#include <cstdint>
#include <string_view>
#include "../TrackingAllocator.h"

namespace daikon::core::types {

using Clock = std::chrono::system_clock;
using TimePoint = Clock::time_point;

enum class DataType : uint8_t {
    kNone = 0,
    kString,
    kList,
    kSet,
    kGeo
};

struct BaseDataObject {
    DataType type = DataType::kNone;
    uint32_t allocated_size = 0;
    TimePoint expires_at;

    explicit BaseDataObject(DataType t) : type(t), expires_at{} {}

    static void* operator new(std::size_t size, mem::IMemoryTracker* tracker) {
        if (tracker) {
            tracker->OnAllocate(size);
        }
        return ::operator new(size);
    }

    static void operator delete(void* ptr, std::size_t size) {
        ::operator delete(ptr);
    }

    static void operator delete(void* ptr, mem::IMemoryTracker* tracker) {
        if (tracker) {
            tracker->OnDeallocate(sizeof(BaseDataObject));
        }
        ::operator delete(ptr);
    }
};

} // namespace daikon::core::types