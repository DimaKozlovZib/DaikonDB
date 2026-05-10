#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
#include <queue>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>
#include <cmath>

#include "./Types/DataObject.h"
#include "TrackingAllocator.h"

namespace daikon::core {

struct StringHash {
    using is_transparent = void;

    size_t operator()(std::string_view sv) const noexcept {
        return std::hash<std::string_view>{}(sv);
    }

    size_t operator()(const std::string& s) const noexcept {
        return std::hash<std::string>{}(s);
    }

    size_t operator()(const types::StringObject::TrackingString& s) const noexcept {
        return std::hash<std::string_view>{}(std::string_view(s.data(), s.size()));
    }
};

} // namespace daikon::core

namespace daikon::core {

class DBase : public mem::IMemoryTracker {
   public:
    using TrackingString = std::basic_string<char, std::char_traits<char>, mem::TrackingAllocator<char>>;
    using MapAllocator = mem::TrackingAllocator<std::pair<const TrackingString, types::DataObject>>;

    DBase() : total_ram_usage_(0), storage_(MapAllocator(this)) {}

    void Set(const std::string_view key, types::DataObject value);
    types::DataObject* Get(const std::string_view key);
    bool Delete(const std::string_view key);
    bool Expire(const std::string_view key, std::chrono::seconds ttl);

    size_t GetTotalRamUsage() const { return total_ram_usage_; }

    void OnAllocate(std::size_t bytes) override {
        total_ram_usage_ += bytes;
    }

    void OnDeallocate(std::size_t bytes) override {
        total_ram_usage_ -= bytes;
    }

    size_t Size() const {
        return storage_.size();
    }

    void Clear() {
        storage_.clear();
    }

    void ForEachKey(std::function<void(std::string_view)> callback) const {
        for (const auto& [key, _] : storage_) {
            callback(std::string_view(key.data(), key.size()));
        }
    }

   private:
    std::unordered_map<
        TrackingString,
        types::DataObject,
        StringHash,
        std::equal_to<>,
        MapAllocator
    > storage_;
    size_t total_ram_usage_;
};

inline void DBase::Set(const std::string_view key, types::DataObject value) {
    auto it = storage_.find(key);
    if (it != storage_.end()) {
        it->second = std::move(value);
    } else {
        storage_.emplace(key, std::move(value));
    }
}

inline types::DataObject* DBase::Get(const std::string_view key) {
    auto it = storage_.find(key);
    if (it == storage_.end()) return nullptr;

    types::DataObject& obj = it->second;
    if (obj.IsExpired()) {
        storage_.erase(it);
        return nullptr;
    }
    return &obj;
}

inline bool DBase::Delete(const std::string_view key) {
    auto it = storage_.find(key);
    if (it == storage_.end()) return false;
    storage_.erase(it);
    return true;
}

inline bool DBase::Expire(const std::string_view key, std::chrono::seconds ttl) {
    auto it = storage_.find(key);
    if (it == storage_.end()) return false;
    it->second.SetTTL(ttl);
    return true;
}

} // namespace daikon::core