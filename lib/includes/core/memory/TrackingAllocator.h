#pragma once

#include <cstddef>
#include <memory>

namespace daikon::core::mem {

struct IMemoryTracker {
    virtual void OnAllocate(size_t bytes) = 0;
    virtual void OnDeallocate(size_t bytes) = 0;
    virtual ~IMemoryTracker() = default;
};

template <typename T>
class TrackingAllocator {
   public:
    using value_type = T;

    TrackingAllocator() noexcept : tracker_(nullptr), root_object_(nullptr) {}
    explicit TrackingAllocator(IMemoryTracker* tracker, uint32_t* root = nullptr) noexcept
        : tracker_(tracker), root_object_(root) {}

    template <typename U>
    TrackingAllocator(const TrackingAllocator<U>& other) noexcept
        : tracker_(other.tracker_), root_object_(other.root_object_) {}

    T* allocate(size_t n) {
        size_t bytes = n * sizeof(T);
        if (tracker_) {
            tracker_->OnAllocate(bytes);
        }
        if (root_object_) {
            *root_object_ += bytes;
        }
        return std::allocator<T>().allocate(n);
    }

    void deallocate(T* p, size_t n) noexcept {
        size_t bytes = n * sizeof(T);
        if (tracker_) {
            tracker_->OnDeallocate(bytes);
        }
        if (root_object_) {
            *root_object_ -= bytes;
        }
        std::allocator<T>().deallocate(p, n);
    }

    IMemoryTracker* tracker_;
    uint32_t* root_object_;
};

template <typename T, typename U>
bool operator==(const TrackingAllocator<T>& a, const TrackingAllocator<U>& b) {
    return a.tracker_ == b.tracker_ && a.root_object_ == b.root_object_;
}

template <typename T, typename U>
bool operator!=(const TrackingAllocator<T>& a, const TrackingAllocator<U>& b) {
    return !(a == b);
}

} // namespace daikon::core::mem