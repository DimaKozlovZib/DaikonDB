#pragma once
#include <string_view>
#include "BaseDataObject.h"
#include "../memory/TrackingAllocator.h"

namespace daikon::core::types {

struct StringObject final : public BaseDataObject {
    using TrackingString = std::basic_string<char, std::char_traits<char>, mem::TrackingAllocator<char>>;

    TrackingString value;

    StringObject(mem::IMemoryTracker* tracker, std::string_view sv)
        : BaseDataObject(DataType::kString),
          value(sv.data(), sv.size(), mem::TrackingAllocator<char>(tracker, &allocated_size))
    {}

    StringObject* AsString() override { return this; }

    std::string_view Get() const { return value; }
    size_t Strlen() const { return value.size(); }
    void Append(std::string_view sv) { value.append(sv.data(), sv.size()); }

    struct MemoryView {
        const StringObject& obj;
        int64_t EstimateSet(std::string_view new_val) const {
            return static_cast<int64_t>(new_val.size()) - static_cast<int64_t>(obj.value.size());
        }
        int64_t EstimateAppend(std::string_view sv) const {
            return static_cast<int64_t>(sv.size());
        }
    };

    MemoryView GetMemoryView() const { return MemoryView{*this}; }

    static int64_t EstimateCreate(std::string_view value) {
        return sizeof(StringObject) + value.size() + 32; 
    }
};

} // namespace daikon::core::types