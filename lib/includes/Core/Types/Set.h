#pragma once

#include <string_view>
#include <unordered_set>
#include <vector>

#include "../TrackingAllocator.h"
#include "BaseDataObject.h"
#include "String.h"

namespace daikon::core::types {

struct SetObject : public BaseDataObject {
    using TrackingString =
        std::basic_string<char, std::char_traits<char>, mem::TrackingAllocator<char>>;
    using Allocator = mem::TrackingAllocator<TrackingString>;

    std::unordered_set<TrackingString, std::hash<TrackingString>,
                       std::equal_to<TrackingString>, Allocator>
        members;

    explicit SetObject(mem::IMemoryTracker* tracker) : BaseDataObject(DataType::kSet),
                                                       members(Allocator(tracker, this)) {}

    int64_t PredictAddDelta(const std::vector<std::string_view>& elements) const {
        int64_t delta = 0;
        for (const auto& v : elements) {
            if (members.find(TrackingString(v, members.get_allocator())) == members.end()) {
                delta += v.size() + 64;
            }
        }
        return delta;
    }

    size_t Add(const std::vector<std::string_view>& elements) {
        size_t added = 0;
        for (const auto& v : elements) {
            if (members.emplace(v, members.get_allocator()).second) ++added;
        }
        return added;
    }

    size_t Remove(const std::vector<std::string_view>& elements) {
        size_t removed = 0;
        for (const auto& v : elements) {
            removed += members.erase(TrackingString(v, members.get_allocator()));
        }
        return removed;
    }

    bool IsMember(std::string_view element) const {
        return members.count(TrackingString(element, members.get_allocator())) > 0;
    }

    std::vector<std::string> AllMembers() const {
        std::vector<std::string> result;
        for (const auto& s : members) {
            result.emplace_back(s.data(), s.size());
        }
        return result;
    }

    size_t Card() const { return members.size(); }

    static std::vector<std::string> UnionSets(const std::vector<SetObject*>& sets) {
        std::unordered_set<std::string> result;
        for (const auto* s : sets) {
            for (const auto& elem : s->members)
                result.insert(std::string(elem.data(), elem.size()));
        }
        return {result.begin(), result.end()};
    }

    static std::vector<std::string> InterSets(const std::vector<SetObject*>& sets) {
        if (sets.empty()) return {};
        std::unordered_set<std::string> result;
        for (const auto& s : sets[0]->members)
            result.insert(std::string(s.data(), s.size()));
        for (size_t i = 1; i < sets.size(); ++i) {
            const auto& s = sets[i]->members;
            for (auto it = result.begin(); it != result.end();) {
                if (s.find(TrackingString(*it, s.get_allocator())) == s.end()) {
                    it = result.erase(it);
                } else {
                    ++it;
                }
            }
        }
        return {result.begin(), result.end()};
    }

    static std::vector<std::string> DiffSets(const std::vector<SetObject*>& sets) {
        if (sets.empty()) return {};
        std::unordered_set<std::string> result;
        for (const auto& s : sets[0]->members)
            result.insert(std::string(s.data(), s.size()));
        for (size_t i = 1; i < sets.size(); ++i) {
            const auto& s = sets[i]->members;
            for (auto it = result.begin(); it != result.end();) {
                if (s.find(TrackingString(*it, s.get_allocator())) != s.end()) {
                    it = result.erase(it);
                } else {
                    ++it;
                }
            }
        }
        return {result.begin(), result.end()};
    }

    int64_t PredictMoveDelta(std::string_view element) const {
        if (members.find(TrackingString(element, members.get_allocator())) != members.end())
            return 0;
        return static_cast<int64_t>(element.size() + 64);
    }

    bool MoveMember(SetObject& dest, std::string_view element) {
        auto alloc = members.get_allocator();
        auto it = members.find(TrackingString(element, alloc));
        if (it == members.end()) return false;
        dest.members.emplace(it->data(), it->size(), dest.members.get_allocator());
        members.erase(it);
        return true;
    }
};

} // namespace daikon::core::types