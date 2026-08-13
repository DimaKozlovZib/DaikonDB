#pragma once
#include <chrono>
#include <cstdint>
#include <string_view>

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

inline std::string_view DataTypeToString(DataType type) {
    switch (type) {
        case DataType::kNone:   return "none";
        case DataType::kString: return "string";
        case DataType::kList:   return "list";
        case DataType::kSet:    return "set";
        case DataType::kGeo:    return "geo";
        default: return "none";
    }
}

struct BaseDataObject {
   protected:
    DataType type_ = DataType::kNone;
    uint32_t allocated_size_ = 0;
    TimePoint expires_at_;

   public:
    explicit BaseDataObject(DataType t) : type_(t), expires_at_{} {}
    virtual ~BaseDataObject() = default;

    bool HasTtl() const { return expires_at_ != TimePoint{}; }
    bool IsExpired() const {
        return HasTtl() && Clock::now() >= expires_at_;
    }
    void SetTTL(std::chrono::seconds seconds) {
        expires_at_ = Clock::now() + seconds;
    }

    DataType GetType() const { return type_; }
    TimePoint GetTtl() const { return expires_at_; }
    uint32_t GetMemoryUsage() const { return allocated_size_; }

    virtual struct StringObject* AsString() { return nullptr; }
    virtual struct ListObject* AsList() { return nullptr; }
    virtual struct SetObject* AsSet() { return nullptr; }
    virtual struct GeoObject* AsGeo() { return nullptr; }
};

} // namespace daikon::core::types