#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>

#include "../memory/TrackingAllocator.h"
#include "../types/BaseDataObject.h"
#include "string_hash.h"

namespace daikon::core {

struct DataObjectDeleter {
    mem::IMemoryTracker* tracker;
    size_t size;

    void operator()(types::BaseDataObject* p) const {
        if (p) {
            p->~BaseDataObject();
            mem::TrackingAllocator<char> alloc(tracker, nullptr);
            alloc.deallocate(reinterpret_cast<char*>(p), size);
        }
    }
};

using ObjectPtr = std::unique_ptr<types::BaseDataObject, DataObjectDeleter>;

class DBase {
   public:
    using TrackingString = std::basic_string<char, std::char_traits<char>, mem::TrackingAllocator<char>>;
    using MapAllocator = mem::TrackingAllocator<std::pair<const TrackingString, ObjectPtr>>;

    DBase() : storage_(MapAllocator(&tracker_)) {}

    void Set(std::string_view key, ObjectPtr value);
    types::BaseDataObject* Get(std::string_view key);
    bool Delete(std::string_view key);
    bool Expire(std::string_view key, std::chrono::seconds ttl);

    template <typename T, typename... Args>
    ObjectPtr CreateObject(Args&&... args) {
        using AllocTraits = std::allocator_traits<mem::TrackingAllocator<T>>;
        mem::TrackingAllocator<T> alloc(&tracker_, nullptr);

        T* p = AllocTraits::allocate(alloc, 1);
        try {
            AllocTraits::construct(alloc, p, &tracker_, std::forward<Args>(args)...);
        } catch (...) {
            AllocTraits::deallocate(alloc, p, 1);
            throw;
        }

        return ObjectPtr(p, DataObjectDeleter{&tracker_, sizeof(T)});
    }

    size_t GetTotalRamUsage() const { return tracker_.GetTotalRamUsage(); }
    size_t Size() const { return storage_.size(); }

    void Clear() {
        storage_.clear();
        storage_.rehash(0);
    }

    void ForEachKey(std::function<void(std::string_view)> callback) const {
        for (const auto& [key, _] : storage_)
            callback(std::string_view(key.data(), key.size()));
    }

    mem::IMemoryTracker* GetMemoryTracker() { return &tracker_; }

   private:
    mem::MemoryTracker tracker_;
    std::unordered_map<TrackingString, ObjectPtr, StringHash, std::equal_to<>, MapAllocator> storage_;
};

inline void DBase::Set(std::string_view key, ObjectPtr value) {
    auto it = storage_.find(key);
    if (it != storage_.end())
        it->second = std::move(value);
    else
        storage_.emplace(key, std::move(value));
}

inline types::BaseDataObject* DBase::Get(std::string_view key) {
    auto it = storage_.find(key);
    if (it == storage_.end()) return nullptr;
    auto& obj = it->second;
    if (obj->IsExpired()) {
        storage_.erase(it);
        return nullptr;
    }
    return obj.get();
}

inline bool DBase::Delete(std::string_view key) {
    auto it = storage_.find(key);
    if (it == storage_.end()) return false;
    storage_.erase(it);
    return true;
}

inline bool DBase::Expire(std::string_view key, std::chrono::seconds ttl) {
    auto it = storage_.find(key);
    if (it == storage_.end()) return false;
    it->second->SetTTL(ttl);
    return true;
}

} // namespace daikon::core