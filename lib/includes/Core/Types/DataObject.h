#pragma once

#include <chrono>
#include <cstdint>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

#include "../TrackingAllocator.h"
#include "BaseDataObject.h"
#include "Geo.h"
#include "List.h"
#include "Set.h"
#include "String.h"

namespace daikon::core::types {

using Clock = std::chrono::system_clock;
using TimePoint = Clock::time_point;

class DataObject {
   public:
    DataObject() : ptr(nullptr) {}
    ~DataObject() { FreeData(); }

    DataObject(DataObject&& other) noexcept : ptr(other.ptr) { other.ptr = nullptr; }
    DataObject& operator=(DataObject&& other) noexcept {
        if (this != &other) {
            FreeData();
            ptr = other.ptr;
            other.ptr = nullptr;
        }
        return *this;
    }
    DataObject(const DataObject&) = delete;
    DataObject& operator=(const DataObject&) = delete;

    static DataObject CreateString(mem::IMemoryTracker* tracker, std::string_view sv) {
        DataObject obj;
        obj.ptr = new (tracker) StringObject(tracker, sv);
        return obj;
    }
    static DataObject CreateList(mem::IMemoryTracker* tracker) {
        DataObject obj;
        obj.ptr = new (tracker) ListObject(tracker);
        return obj;
    }
    static DataObject CreateSet(mem::IMemoryTracker* tracker) {
        DataObject obj;
        obj.ptr = new (tracker) SetObject(tracker);
        return obj;
    }
    static DataObject CreateGeo(mem::IMemoryTracker* tracker) {
        DataObject obj;
        obj.ptr = new (tracker) GeoObject(tracker);
        return obj;
    }

    DataType GetType() const { return ptr->type; }

    bool HasTtl() const {
        if (!ptr) return false;
        return ptr->expires_at != TimePoint{};
    }
    bool IsExpired() const {
        if (!HasTtl()) return false;
        return Clock::now() >= ptr->expires_at;
    }
    void SetTTL(std::chrono::seconds seconds) {
        if (ptr) ptr->expires_at = Clock::now() + seconds;
    }
    TimePoint GetTtl() const { return ptr->expires_at; }

    BaseDataObject& GetRef() {
        if (!ptr) throw std::exception("nullptr");
        return *ptr;
    }

    StringObject& AsString() {
        CheckType(DataType::kString);
        return *static_cast<StringObject*>(ptr);
    }
    const StringObject& AsString() const {
        CheckType(DataType::kString);
        return *static_cast<const StringObject*>(ptr);
    }

    ListObject& AsList() {
        CheckType(DataType::kList);
        return *static_cast<ListObject*>(ptr);
    }
    const ListObject& AsList() const {
        CheckType(DataType::kList);
        return *static_cast<const ListObject*>(ptr);
    }

    SetObject& AsSet() {
        CheckType(DataType::kSet);
        return *static_cast<SetObject*>(ptr);
    }
    const SetObject& AsSet() const {
        CheckType(DataType::kSet);
        return *static_cast<const SetObject*>(ptr);
    }

    GeoObject& AsGeo() {
        CheckType(DataType::kGeo);
        return *static_cast<GeoObject*>(ptr);
    }
    const GeoObject& AsGeo() const {
        CheckType(DataType::kGeo);
        return *static_cast<const GeoObject*>(ptr);
    }

   private:
    BaseDataObject* ptr = nullptr;

    void FreeData() {
        if (!ptr) return;
        auto* tracker = GetTracker();
        std::size_t size = GetObjectSize(ptr->type);
        switch (ptr->type) {
            case DataType::kString:
                delete static_cast<StringObject*>(ptr);
                break;
            case DataType::kList:
                delete static_cast<ListObject*>(ptr);
                break;
            case DataType::kSet:
                delete static_cast<SetObject*>(ptr);
                break;
            case DataType::kGeo:
                delete static_cast<GeoObject*>(ptr);
                break;
            default:
                break;
        }
        if (tracker) {
            tracker->OnDeallocate(size);
        }
        ptr = nullptr;
    }

    mem::IMemoryTracker* GetTracker() const {
        if (!ptr) return nullptr;
        switch (ptr->type) {
            case DataType::kString:
                return static_cast<StringObject*>(ptr)->value.get_allocator().tracker_;
            case DataType::kList:
                return static_cast<ListObject*>(ptr)->values.get_allocator().tracker_;
            case DataType::kSet:
                return static_cast<SetObject*>(ptr)->members.get_allocator().tracker_;
            case DataType::kGeo:
                return static_cast<GeoObject*>(ptr)->points.get_allocator().tracker_;
            default:
                return nullptr;
        }
    }

    static std::size_t GetObjectSize(DataType type) {
        switch (type) {
            case DataType::kString:
                return sizeof(StringObject);
            case DataType::kList:
                return sizeof(ListObject);
            case DataType::kSet:
                return sizeof(SetObject);
            case DataType::kGeo:
                return sizeof(GeoObject);
            default:
                return 0;
        }
    }

    void CheckType(DataType expected) const {
        if (!ptr || ptr->type != expected)
            throw std::runtime_error("WRONGTYPE");
    }
};

} // namespace daikon::core::types