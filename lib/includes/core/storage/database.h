#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <set>
#include <tuple>

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
    using TimePoint = types::Clock::time_point;
    
    using ExpireEntry = std::tuple<TimePoint, size_t, TrackingString>;
    using SetAllocator = mem::TrackingAllocator<ExpireEntry>;

    DBase() : storage_(MapAllocator(&tracker_)), expire_queue_(SetAllocator(&tracker_)) {}

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
        std::unordered_map<TrackingString, ObjectPtr, StringHash, std::equal_to<>, MapAllocator> empty{MapAllocator(&tracker_)};
        storage_.swap(empty);
        expire_queue_.clear();
    }

    void ForEachKey(std::function<void(std::string_view)> callback) const {
        for (const auto& [key, _] : storage_)
            callback(std::string_view(key.data(), key.size()));
    }

    mem::IMemoryTracker* GetMemoryTracker() { return &tracker_; }

    void ExpireCycle(size_t max_keys = 5);

private:
    mem::MemoryTracker tracker_;
    std::unordered_map<TrackingString, ObjectPtr, StringHash, std::equal_to<>, MapAllocator> storage_;
    
    std::set<ExpireEntry, std::less<ExpireEntry>, SetAllocator> expire_queue_;
    StringHash hash_fn_;
};

inline void DBase::Set(std::string_view key, ObjectPtr value) {
    auto it = storage_.find(key);
    if (it != storage_.end()) {
        if (it->second->HasTtl()) {
            size_t hash = hash_fn_(key);
            expire_queue_.erase(std::make_tuple(it->second->GetTtl(), hash, TrackingString(key, mem::TrackingAllocator<char>(&tracker_))));
        }
        it->second = std::move(value);
    } else {
        storage_.emplace(key, std::move(value));
    }
}

inline types::BaseDataObject* DBase::Get(std::string_view key) {
    auto it = storage_.find(key);
    if (it == storage_.end()) return nullptr;
    auto& obj = it->second;
    if (obj->IsExpired()) {
        size_t hash = hash_fn_(key);
        expire_queue_.erase(std::make_tuple(obj->GetTtl(), hash, TrackingString(key, mem::TrackingAllocator<char>(&tracker_))));
        storage_.erase(it);
        return nullptr;
    }
    return obj.get();
}

inline bool DBase::Delete(std::string_view key) {
    auto it = storage_.find(key);
    if (it == storage_.end()) return false;
    
    if (it->second->HasTtl()) {
        size_t hash = hash_fn_(key);
        expire_queue_.erase(std::make_tuple(it->second->GetTtl(), hash, TrackingString(key, mem::TrackingAllocator<char>(&tracker_))));
    }
    storage_.erase(it);
    return true;
}

inline bool DBase::Expire(std::string_view key, std::chrono::seconds ttl) {
    auto it = storage_.find(key);
    if (it == storage_.end()) return false;

    size_t hash = hash_fn_(key);

    if (it->second->HasTtl()) {
        expire_queue_.erase(std::make_tuple(it->second->GetTtl(), hash, TrackingString(key, mem::TrackingAllocator<char>(&tracker_))));
    }

    it->second->SetTTL(ttl);

    expire_queue_.emplace(it->second->GetTtl(), hash, TrackingString(key, mem::TrackingAllocator<char>(&tracker_)));
    return true;
}

inline void DBase::ExpireCycle(size_t max_keys) {
    auto now = types::Clock::now();
    size_t removed = 0;

    auto it = expire_queue_.begin();
    while (it != expire_queue_.end() && removed < max_keys) {
        if (std::get<0>(*it) > now) break;

        const auto& key = std::get<2>(*it);
        auto storage_it = storage_.find(key);
        
        if (storage_it != storage_.end()) {
            storage_.erase(storage_it);
            ++removed;
        }
        
        it = expire_queue_.erase(it);
    }
}

} // namespace daikon::core