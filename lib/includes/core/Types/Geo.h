#pragma once

#include <algorithm>
#include <memory>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <iostream>

#include "../../Commands.h"
#include "../memory/TrackingAllocator.h"
#include "../storage/string_hash.h"
#include "BaseDataObject.h"
#include "details/haversine.h"
#include "details/quadtree.h"

namespace daikon::core::types {

struct GeoPoint {
    double longitude;
    double latitude;
};

struct GeoObject final : public BaseDataObject {
    using TrackingString = std::basic_string<char, std::char_traits<char>, mem::TrackingAllocator<char>>;
    using MapAllocator = mem::TrackingAllocator<std::pair<const TrackingString, GeoPoint>>;
    using IndexPoint = detail::Quadtree<TrackingString, GeoPoint, MapAllocator>::IndexedPoint;

    std::unordered_map<TrackingString, GeoPoint, StringHash, std::equal_to<>, MapAllocator> points;

   private:
    using QuadtreeType = detail::Quadtree<TrackingString, GeoPoint, MapAllocator>;
    using NodeAlloc = typename std::allocator_traits<MapAllocator>::template rebind_alloc<QuadtreeType>;

    std::unique_ptr<QuadtreeType> spatial_index_;
    NodeAlloc node_alloc_;

   public:
    explicit GeoObject(mem::IMemoryTracker* tracker)
        : BaseDataObject(DataType::kGeo),
          points(MapAllocator(tracker, &allocated_size)),
          node_alloc_(NodeAlloc(tracker, &allocated_size)),
          spatial_index_(std::make_unique<QuadtreeType>(points.get_allocator(), 8, 20, 10.0)) {}

    GeoObject* AsGeo() override { return this; }

    void Add(double lon, double lat, std::string_view member) {
        auto key = TrackingString(member.data(), member.size(),
                                  points.get_allocator());
        auto it = points.find(key);

        if (it == points.end()) {
            auto inserted = points.emplace(key, GeoPoint{lon, lat});
            spatial_index_->Insert({inserted.first->first, inserted.first->second});
        } else {
            it->second = GeoPoint{lon, lat};
            spatial_index_->Insert({it->first, it->second});
        }
    }

    std::vector<std::optional<std::pair<double, double>>> Pos(const std::vector<std::string_view>& members) const {
        std::vector<std::optional<std::pair<double, double>>> result;
        result.reserve(members.size());
        for (const auto& m : members) {
            auto it = points.find(m);
            if (it != points.end())
                result.emplace_back(std::make_pair(it->second.longitude, it->second.latitude));
            else
                result.emplace_back(std::nullopt);
        }
        return result;
    }

    std::optional<double> Dist(std::string_view member1, std::string_view member2, std::string_view unit = "m") const {
        auto it1 = points.find(member1);
        auto it2 = points.find(member2);
        if (it1 == points.end() || it2 == points.end()) return std::nullopt;
        return detail::Haversine(it1->second.longitude, it1->second.latitude,
                                 it2->second.longitude, it2->second.latitude, unit);
    }

    std::vector<std::string> Search(double lon, double lat, double radius, std::string_view unit,
                                         bool asc = false, std::optional<int64_t> count = std::nullopt) const {
        double radius_m;
        if (unit == "km")
            radius_m = radius * 1000.0;
        else if (unit == "mi")
            radius_m = radius * 1609.344;
        else if (unit == "ft")
            radius_m = radius * 0.3048;
        else
            radius_m = radius;

        std::vector<std::pair<std::string_view, double>> results;
        auto candidates = spatial_index_->RadiusSearch(lon, lat, radius_m);

        for (const auto& idx_pt : candidates) {
            auto it = points.find(idx_pt.name);
            if (it != points.end()) {
                double d = detail::Haversine(lon, lat, it->second.longitude, it->second.latitude, unit);
                if (d <= radius)
                    results.emplace_back(std::string_view(idx_pt.name.data(), idx_pt.name.size()), d);
            }
        }

        auto comp = [asc](const auto& a, const auto& b) {
            return asc ? a.second < b.second : a.second > b.second;
        };

        if (count.has_value() && *count > 0 && results.size() > static_cast<size_t>(*count)) {
            std::partial_sort(results.begin(), results.begin() + *count, results.end(), comp);
            results.resize(*count);
        } else {
            std::sort(results.begin(), results.end(), comp);
        }

        std::vector<std::string> out;
        out.reserve(results.size());
        for (auto& [name, _] : results)
            out.emplace_back(std::move(name));
        return out;
    }

    std::vector<std::pair<std::string, GeoPoint>> SearchPoints(double lon, double lat, double radius,
                                                                    std::string_view unit, bool asc = false,
                                                                    std::optional<int64_t> count = std::nullopt) const {
        double radius_m;
        if (unit == "km")
            radius_m = radius * 1000.0;
        else if (unit == "mi")
            radius_m = radius * 1609.344;
        else if (unit == "ft")
            radius_m = radius * 0.3048;
        else
            radius_m = radius;

        std::vector<std::pair<std::string_view, std::pair<double, GeoPoint>>> tmp;
        auto candidates = spatial_index_->RadiusSearch(lon, lat, radius_m);

        for (const auto& idx_pt : candidates) {
            auto it = points.find(idx_pt.name);
            if (it != points.end()) {
                double d = detail::Haversine(lon, lat, it->second.longitude, it->second.latitude, unit);
                if (d <= radius)
                    tmp.emplace_back(std::string_view(idx_pt.name.data(), idx_pt.name.size()),
                                     std::make_pair(d, it->second));
            }
        }

        auto comp = [asc](const auto& a, const auto& b) {
            return asc ? a.second.first < b.second.first : a.second.first > b.second.first;
        };

        if (count.has_value() && *count > 0 && tmp.size() > static_cast<size_t>(*count)) {
            std::partial_sort(tmp.begin(), tmp.begin() + *count, tmp.end(), comp);
            tmp.resize(*count);
        } else {
            std::sort(tmp.begin(), tmp.end(), comp);
        }

        std::vector<std::pair<std::string, GeoPoint>> out;
        out.reserve(tmp.size());
        for (auto& [name, dist_pt] : tmp)
            out.emplace_back(std::move(name), dist_pt.second);
        return out;
    }

    struct MemoryView {
        const GeoObject& obj;
        int64_t EstimateAdd(std::string_view member) const {
            return member.size() + sizeof(GeoPoint) + 48 + 200;
        }

        static int64_t EstimateAddNew(std::string_view member) {
            return member.size() + sizeof(GeoPoint) + 48 + 200;
        }


        static int64_t EstimateCreateWithPoints(const std::vector<::daikon::commands::GeoPoint>& points) {
            int64_t delta = EstimateCreate();
            for (const auto& pt : points)
                delta += EstimateAddNew(pt.member);
            return delta;
        }

        static int64_t EstimateCreateWithPoints(const std::vector<std::pair<std::string, GeoPoint>>& points) {
            int64_t delta = EstimateCreate();
            for (const auto& [name, _] : points)
                delta += EstimateAddNew(name);
            return delta;
        }
    };

    MemoryView GetMemoryView() const { return MemoryView{*this}; }

    static int64_t EstimateCreate() {
        return sizeof(GeoObject);
    }
};

} // namespace daikon::core::types