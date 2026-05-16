#pragma once
#include <string_view>
#include <unordered_set>
#include <vector>

#include "../memory/TrackingAllocator.h"
#include "../storage/string_hash.h"
#include "BaseDataObject.h"

namespace daikon::core::types {

struct SetObject final : public BaseDataObject {
    using TrackingString = std::basic_string<char, std::char_traits<char>, mem::TrackingAllocator<char>>;
    using Allocator = mem::TrackingAllocator<TrackingString>;

    std::unordered_set<TrackingString, StringHash, std::equal_to<>, Allocator> members;

    explicit SetObject(mem::IMemoryTracker* tracker)
        : BaseDataObject(DataType::kSet),
          members(Allocator(tracker, &allocated_size)) {}

    SetObject* AsSet() override { return this; }

    size_t Add(const std::vector<std::string_view>& elements) {
        size_t added = 0;
        for (const auto& v : elements) {
            if (members.emplace(v.data(), v.size(), members.get_allocator()).second) ++added;
        }
        return added;
    }

    size_t Remove(const std::vector<std::string_view>& elements) {
        size_t removed = 0;
        for (const auto& v : elements) {
            auto it = members.find(v);
            if (it != members.end()) {
                members.erase(it);
                ++removed;
            }
        }
        return removed;
    }

    bool IsMember(std::string_view element) const {
        return members.find(element) != members.end();
    }

    std::vector<std::string> AllMembers() const {
        std::vector<std::string> result;
        result.reserve(members.size());
        for (const auto& s : members)
            result.emplace_back(s.data(), s.size());
        return result;
    }

    size_t Card() const { return members.size(); }

    static std::vector<std::string> UnionSets(const std::vector<SetObject*>& sets) {
        if (sets.empty()) return {};
        std::unordered_set<std::string_view> seen;
        std::vector<std::string> result;
        for (const auto* s : sets) {
            for (const auto& elem : s->members) {
                std::string_view sv(elem.data(), elem.size());
                if (seen.insert(sv).second)
                    result.emplace_back(sv);
            }
        }
        return result;
    }

    static std::vector<std::string> InterSets(const std::vector<SetObject*>& sets) {
        if (sets.empty()) return {};
        std::vector<std::string> result;
        for (const auto& elem : sets[0]->members) {
            std::string_view sv(elem.data(), elem.size());
            bool in_all = true;
            for (size_t i = 1; i < sets.size(); ++i) {
                if (!sets[i]->members.contains(sv)) {
                    in_all = false;
                    break;
                }
            }
            if (in_all) result.emplace_back(sv);
        }
        return result;
    }

    static std::vector<std::string> DiffSets(const std::vector<SetObject*>& sets) {
        if (sets.empty()) return {};
        std::vector<std::string> result;
        for (const auto& elem : sets[0]->members) {
            std::string_view sv(elem.data(), elem.size());
            bool in_any = false;
            for (size_t i = 1; i < sets.size(); ++i) {
                if (sets[i]->members.contains(sv)) {
                    in_any = true;
                    break;
                }
            }
            if (!in_any) result.emplace_back(sv);
        }
        return result;
    }

    bool MoveMember(SetObject& dest, std::string_view element) {
        auto it = members.find(element);
        if (it == members.end()) return false;
        dest.members.emplace(it->data(), it->size(), dest.members.get_allocator());
        members.erase(it);
        return true;
    }

    struct MemoryView {
        const SetObject& obj;
        int64_t EstimateAdd(const std::vector<std::string_view>& elements) const {
            int64_t delta = 0;
            for (const auto& v : elements) {
                if (!obj.members.contains(v))
                    delta += v.size() + 64;
            }
            return delta;
        }
        int64_t EstimateMove(std::string_view element) const {
            if (obj.members.contains(element)) return 0;
            return element.size() + 64;
        }
        static int64_t EstimateAddNew(const std::vector<std::string_view>& elements) {
            int64_t delta = 0;
            for (const auto& v : elements)
                delta += v.size() + 64;
            return delta;
        }
    };

    MemoryView GetMemoryView() const { return MemoryView{*this}; }

    static int64_t EstimateCreate() {
        return sizeof(SetObject);
    }
};

} // namespace daikon::core::types