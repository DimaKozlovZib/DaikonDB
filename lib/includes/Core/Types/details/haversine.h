#pragma once
#include <cmath>
#include <string_view>

namespace daikon::core::types::detail {

inline constexpr double kEarthRadiusKm = 6372.8;
inline constexpr double kPi = 3.14159265358979323846;

inline double Haversine(double lon1, double lat1, double lon2, double lat2, std::string_view unit) {
    double dlon = (lon2 - lon1) * kPi / 180.0;
    double dlat = (lat2 - lat1) * kPi / 180.0;
    double a = std::sin(dlat / 2) * std::sin(dlat / 2) + 
               std::cos(lat1 * kPi / 180.0) * std::cos(lat2 * kPi / 180.0) *
               std::sin(dlon / 2) * std::sin(dlon / 2);
    double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));
    double km = kEarthRadiusKm * c;
    if (unit == "km") return km;
    if (unit == "mi") return km * 0.621371;
    if (unit == "ft") return km * 3280.84;
    return km * 1000.0;
}

inline double HaversineMeters(double lon1, double lat1, double lon2, double lat2) {
    return Haversine(lon1, lat1, lon2, lat2, "m");
}

} // namespace daikon::core::types::detail