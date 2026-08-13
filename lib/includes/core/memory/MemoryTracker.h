#pragma once

#include <cstddef>
#include <memory>

namespace daikon::core::mem {

struct IMemoryTracker {
    virtual void OnAllocate(size_t bytes) = 0;
    virtual void OnDeallocate(size_t bytes) = 0;
    virtual ~IMemoryTracker() = default;
};

class MemoryTracker : public IMemoryTracker {
public:
    MemoryTracker() : total_ram_usage_(0) {}
    
    void OnAllocate(size_t bytes) override { total_ram_usage_ += bytes; }
    void OnDeallocate(size_t bytes) override { total_ram_usage_ -= bytes; }
    
    size_t GetTotalRamUsage() const { return total_ram_usage_; }
    void Reset() { total_ram_usage_ = 0; }
    
private:
    size_t total_ram_usage_ = 0;
};

} // namespace daikon::core::mem