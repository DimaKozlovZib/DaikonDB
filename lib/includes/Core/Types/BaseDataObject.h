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

struct BaseDataObject {
   protected:
    DataType type = DataType::kNone;
    uint32_t allocated_size = 0;
    TimePoint expires_at;

   public:
    explicit BaseDataObject(DataType t) : type(t), expires_at{} {}
    virtual ~BaseDataObject() = default;

    bool HasTtl() const { return expires_at != TimePoint{}; }
    bool IsExpired() const {
        return HasTtl() && Clock::now() >= expires_at;
    }
    void SetTTL(std::chrono::seconds seconds) {
        expires_at = Clock::now() + seconds;
    }

    DataType GetType() const { return type; }
    TimePoint GetTtl() const { return expires_at; }
    uint32_t GetMemoryUsage() const { return allocated_size; }

    virtual struct StringObject* AsString() { return nullptr; }
    virtual struct ListObject* AsList() { return nullptr; }
    virtual struct SetObject* AsSet() { return nullptr; }
    virtual struct GeoObject* AsGeo() { return nullptr; }
};

} // namespace daikon::core::types