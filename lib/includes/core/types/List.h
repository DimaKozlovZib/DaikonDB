#pragma once
#include <deque>
#include <string_view>
#include <vector>

#include "../memory/TrackingAllocator.h"
#include "BaseDataObject.h"

namespace daikon::core::types {

struct ListObject final : public BaseDataObject {
    using TrackingString = std::basic_string<char, std::char_traits<char>, mem::TrackingAllocator<char>>;
    using Allocator = mem::TrackingAllocator<TrackingString>;

    std::deque<TrackingString, Allocator> values;

    explicit ListObject(mem::IMemoryTracker* tracker)
        : BaseDataObject(DataType::kList),
          values(Allocator(tracker, &allocated_size_)) {}

    ListObject* AsList() override { return this; }

    void PushLeft(const std::vector<std::string_view>& vals) {
        for (auto& v : vals)
            values.emplace_front(v.data(), v.size(), values.get_allocator());
    }
    void PushRight(const std::vector<std::string_view>& vals) {
        for (auto& v : vals)
            values.emplace_back(v.data(), v.size(), values.get_allocator());
    }

    std::vector<std::string> PopLeft(size_t count) {
        count = std::min(count, values.size());
        std::vector<std::string> res;
        res.reserve(count);
        for (size_t i = 0; i < count; ++i) {
            res.push_back(std::string(values.front().data(), values.front().size()));
            values.pop_front();
        }
        return res;
    }
    std::vector<std::string> PopRight(size_t count) {
        count = std::min(count, static_cast<size_t>(values.size()));
        std::vector<std::string> res;
        res.reserve(count);
        for (size_t i = 0; i < count; ++i) {
            res.push_back(std::string(values.back().data(), values.back().size()));
            values.pop_back();
        }
        return res;
    }

    size_t Len() const { return values.size(); }

    std::vector<std::string_view> GetRange(int64_t start, int64_t stop) const {
        int64_t size = static_cast<int64_t>(values.size());
        if (size == 0) return {};

        if (start < 0) start = size + start;
        if (stop < 0) stop = size + stop;

        if (start < 0) start = 0;
        if (stop >= size) stop = size - 1;

        if (start > stop || start >= size) return {};

        stop = std::min(stop, size - 1);
        std::vector<std::string_view> result;
        result.reserve(stop - start + 1);

        for (auto it = values.begin() + start; it != values.begin() + stop + 1; ++it)
            result.emplace_back(std::string_view(it->data(), it->size()));
        return result;
    }

    std::optional<std::string_view> GetByIndex(int64_t i) const {
        int64_t size = static_cast<int64_t>(values.size());
        if (i < 0) i += size;
        if (i < 0 || i >= size) return std::nullopt;

        return std::string_view(values[static_cast<size_t>(i)].data(), values[static_cast<size_t>(i)].size());
    }

    void Set(int64_t i, std::string_view val) {
        int64_t size = static_cast<int64_t>(values.size());
        if (i < 0 || i >= size) return;

        values[static_cast<size_t>(i)] = TrackingString(val.data(), val.size(), values.get_allocator());
    }

    int64_t Insert(bool before, std::string_view pivot, std::string_view val) {
        auto it = std::find_if(values.begin(), values.end(),
                               [&](const TrackingString& ts) {
                                   return ts.size() == pivot.size() && std::equal(ts.begin(), ts.end(), pivot.begin());
                               });

        if (it == values.end()) return -1;
        if (before)
            values.insert(it, TrackingString(val.data(), val.size(), values.get_allocator()));
        else
            values.insert(std::next(it), TrackingString(val.data(), val.size(), values.get_allocator()));
        return static_cast<int64_t>(values.size());
    }

    struct MemoryView {
        const ListObject& obj;
        int64_t EstimatePush(const std::vector<std::string_view>& vals) const {
            int64_t delta = 0;
            for (const auto& sv : vals)
                delta += sv.size() + 32;
            return delta;
        }
        int64_t EstimateSet(int64_t index, std::string_view new_val) const {
            if (index < 0 || static_cast<size_t>(index) >= obj.values.size()) return 0;
            const auto& old = obj.values[static_cast<size_t>(index)];
            int64_t diff = static_cast<int64_t>(new_val.size()) - static_cast<int64_t>(old.capacity());
            return diff > 0 ? diff : 0;
        }
        int64_t EstimateInsert(std::string_view val) const {
            return val.size() + 32;
        }

        static int64_t EstimatePushNew(const std::vector<std::string_view>& vals) {
            int64_t delta = 0;
            for (const auto& sv : vals)
                delta += sv.size() + 32;
            return delta;
        }
    };

    MemoryView GetMemoryView() const { return MemoryView{*this}; }

    static int64_t EstimateCreate() {
        return sizeof(ListObject);
    }
};

} // namespace daikon::core::types