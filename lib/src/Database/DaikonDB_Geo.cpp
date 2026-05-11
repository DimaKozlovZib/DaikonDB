#include "../../includes/Core/Types/Geo.h"
#include "../../includes/core/DaikonDB.h"

namespace daikon {

DbResult<void> DaikonDatabase::Geoadd(std::string_view key,
                                      const std::vector<commands::GeoPoint>& points) {
    auto* obj = db_.Get(key);
    auto* geo = obj ? obj->AsGeo() : nullptr;
    if (obj && !geo) return std::unexpected(LogicError::kWrongType);

    if (!geo) {
        int64_t delta = PredictKeyOverhead(key) + core::types::GeoObject::MemoryView::EstimateCreateWithPoints(points);
        if (WillExceedLimit(delta)) return std::unexpected(LogicError::kOom);

        auto newGeo = db_.CreateObject<core::types::GeoObject>();
        for (const auto& pt : points)
            newGeo->AsGeo()->Add(pt.longitude, pt.latitude, pt.member);

        db_.Set(key, std::move(newGeo));
        return {};
    }

    auto view = geo->GetMemoryView();
    for (const auto& pt : points) {
        if (WillExceedLimit(view.EstimateAdd(pt.member))) return std::unexpected(LogicError::kOom);
        geo->Add(pt.longitude, pt.latitude, pt.member);
    }
    return {};
}

DbResult<std::vector<std::optional<std::pair<double, double>>>> DaikonDatabase::Geopos(
    std::string_view key,
    const std::vector<std::string_view>& members) {
    auto* obj = db_.Get(key);
    if (!obj) return std::vector<std::optional<std::pair<double, double>>>{};
    auto* geo = obj->AsGeo();
    if (!geo) return std::unexpected(LogicError::kWrongType);
    return geo->Pos(members);
}

DbResult<double> DaikonDatabase::Geodist(std::string_view key,
                                         std::string_view member1,
                                         std::string_view member2,
                                         std::string_view unit) {
    auto* obj = db_.Get(key);
    if (!obj) return std::unexpected(LogicError::kNotFound);
    auto* geo = obj->AsGeo();
    if (!geo) return std::unexpected(LogicError::kWrongType);
    auto dist = geo->Dist(member1, member2, unit);
    if (!dist) return std::unexpected(LogicError::kNotFound);
    return *dist;
}

DbResult<results::ListViewResult> DaikonDatabase::Geosearch(
    std::string_view key, double lon, double lat,
    double radius, std::string_view unit, bool asc,
    int64_t count) {
    auto* obj = db_.Get(key);
    if (!obj) return std::unexpected(LogicError::kNotFound);
    auto* geo = obj->AsGeo();
    if (!geo) return std::unexpected(LogicError::kWrongType);
    auto views = geo->Search(lon, lat, radius, unit, asc,
                             (count <= 0) ? std::nullopt : std::optional<int64_t>(count));
    std::vector<std::string> names;
    names.reserve(views.size());

    for (auto v : views)
        names.emplace_back(v);
    return results::ListViewResult{std::move(names)};
}

DbResult<results::ListViewResult> DaikonDatabase::Geosearchstore(
    std::string_view dest, std::string_view source,
    double lon, double lat, double radius,
    std::string_view unit, bool asc,
    int64_t count) {
    auto* src_obj = db_.Get(source);
    if (!src_obj) return std::unexpected(LogicError::kNotFound);

    auto* src_geo = src_obj->AsGeo();
    if (!src_geo) return std::unexpected(LogicError::kWrongType);

    auto points = src_geo->SearchPoints(lon, lat, radius, unit, asc,
                                        (count <= 0) ? std::nullopt : std::optional<int64_t>(count));

    std::vector<commands::GeoPoint> geo_points;
    for (const auto& [name, pt] : points) {
        geo_points.push_back({pt.longitude, pt.latitude, std::string(name)});
    }

    int64_t delta = PredictKeyOverhead(dest) + core::types::GeoObject::MemoryView::EstimateCreateWithPoints(geo_points);
    if (WillExceedLimit(delta)) return std::unexpected(LogicError::kOom);

    auto dest_geo = db_.CreateObject<core::types::GeoObject>();
    for (const auto& pt : geo_points)
        dest_geo->AsGeo()->Add(pt.longitude, pt.latitude, pt.member);
    db_.Set(dest, std::move(dest_geo));

    std::vector<std::string> names;
    names.reserve(points.size());
    for (const auto& [name, _] : points)
        names.emplace_back(name);
    return results::ListViewResult{std::move(names)};
}

} // namespace daikon