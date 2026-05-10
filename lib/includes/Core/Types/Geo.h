#pragma once

#include <algorithm>
#include <cmath>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "../TrackingAllocator.h"
#include "BaseDataObject.h"

namespace daikon::core::types {

struct GeoPoint {
    double longitude;
    double latitude;
};

namespace detail {

inline constexpr double kEarthRadiusKm = 6372.8;
inline constexpr double kPi = 3.14159265358979323846264338327950288;

inline double Haversine(double lon1, double lat1, double lon2, double lat2,
                        std::string_view unit) {
    double dlon = (lon2 - lon1) * kPi / 180.0;
    double dlat = (lat2 - lat1) * kPi / 180.0;
    double a = std::sin(dlat / 2) * std::sin(dlat / 2) + std::cos(lat1 * kPi / 180.0) * std::cos(lat2 * kPi / 180.0) * std::sin(dlon / 2) * std::sin(dlon / 2);
    double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));
    double km = kEarthRadiusKm * c;
    if (unit == "km") return km;
    if (unit == "mi") return km * 0.621371;
    if (unit == "ft") return km * 3280.84;
    return km * 1000.0;
}

} // namespace detail

struct GeoObject : public BaseDataObject {
    using TrackingString =
        std::basic_string<char, std::char_traits<char>, mem::TrackingAllocator<char>>;
    using Allocator = mem::TrackingAllocator<std::pair<const TrackingString, GeoPoint>>;

    std::unordered_map<TrackingString, GeoPoint, std::hash<TrackingString>, std::equal_to<TrackingString>, Allocator>
        points;

    explicit GeoObject(mem::IMemoryTracker* tracker)
        : BaseDataObject(DataType::kGeo),
          points(Allocator(tracker, &this->allocated_size)) {}

    static int64_t PredictAddDelta(std::string_view member) {
        return static_cast<int64_t>(member.size() + sizeof(GeoPoint) + 48);
    }

    void Add(double lon, double lat, std::string_view member) {
        points.try_emplace(
            TrackingString(member, points.get_allocator()),
            GeoPoint{lon, lat});
    }

    std::vector<std::optional<std::pair<double, double>>> Pos(
        const std::vector<std::string_view>& members) const {
        std::vector<std::optional<std::pair<double, double>>> result;
        result.reserve(members.size());
        for (const auto& m : members) {
            auto it = points.find(TrackingString(m, points.get_allocator()));
            if (it != points.end()) {
                result.emplace_back(std::make_pair(it->second.longitude, it->second.latitude));
            } else {
                result.emplace_back(std::nullopt);
            }
        }
        return result;
    }

    std::optional<double> Dist(std::string_view member1, std::string_view member2,
                               std::string_view unit = "m") const {
        auto it1 = points.find(TrackingString(member1, points.get_allocator()));
        auto it2 = points.find(TrackingString(member2, points.get_allocator()));
        if (it1 == points.end() || it2 == points.end()) return std::nullopt;
        return detail::Haversine(it1->second.longitude, it1->second.latitude,
                                 it2->second.longitude, it2->second.latitude, unit);
    }

    std::vector<std::string_view> Search(double lon, double lat, double radius,
                                         std::string_view unit, bool asc = false,
                                         std::optional<int64_t> count = std::nullopt) const {
        std::vector<std::pair<std::string_view, double>> results;
        for (const auto& [name, pt] : points) {
            double d = detail::Haversine(lon, lat, pt.longitude, pt.latitude, unit);
            if (d <= radius) results.emplace_back(name, d);
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

        std::vector<std::string_view> out;
        out.reserve(results.size());
        for (auto& [name, _] : results) {
            out.emplace_back(name);
        }
        return out;
    }

    std::vector<std::pair<std::string_view, GeoPoint>> SearchPoints(
        double lon, double lat, double radius, std::string_view unit, bool asc = false,
        std::optional<int64_t> count = std::nullopt) const {
        std::vector<std::pair<std::string_view, std::pair<double, GeoPoint>>> tmp;
        for (const auto& [name, pt] : points) {
            double d = detail::Haversine(lon, lat, pt.longitude, pt.latitude, unit);
            if (d <= radius) {
                tmp.emplace_back(std::string_view(name.data(), name.size()),
                                 std::make_pair(d, pt));
            }
        }

        auto comp = [asc](const auto& a, const auto& b) {
            return asc ? a.first < b.first : a.first > b.first;
        };

        if (count.has_value() && *count > 0 && tmp.size() > static_cast<size_t>(*count)) {
            std::partial_sort(tmp.begin(), tmp.begin() + *count, tmp.end(), comp);
            tmp.resize(*count);
        } else {
            std::sort(tmp.begin(), tmp.end(), comp);
        }

        std::vector<std::pair<std::string_view, GeoPoint>> out;
        out.reserve(tmp.size());
        for (auto& [name, dist_pt] : tmp) {
            out.emplace_back(name, dist_pt.second);
        }
        return out;
    }
};

} // namespace daikon::core::types