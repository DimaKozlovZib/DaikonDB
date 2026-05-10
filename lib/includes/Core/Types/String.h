#pragma once

#include <string>
#include <string_view>

#include "BaseDataObject.h"
#include "../TrackingAllocator.h"

namespace daikon::core::types {

struct StringObject : public BaseDataObject {
    using TrackingString =
        std::basic_string<char, std::char_traits<char>, mem::TrackingAllocator<char>>;
    using Allocator = mem::TrackingAllocator<char>;

    TrackingString value;

    explicit StringObject(mem::IMemoryTracker* tracker, std::string_view sv)
        : BaseDataObject(DataType::kString),
          value(sv.data(), sv.size(), Allocator(tracker)) {}

    std::string_view Get() const { return value; }
    size_t Strlen() const { return value.size(); }

    int64_t PredictAppendDelta(std::string_view sv) const {
        return static_cast<int64_t>(sv.size());
    }

    void Append(std::string_view sv) {
        value.append(sv.data(), sv.size());
    }
};

} // namespace daikon::core::types