#include "DaikonDB.h"
#include "Geo.h"

namespace daikon {

DbResult<void> DaikonDataBase::Geoadd(std::string_view key,
                                      const std::vector<commands::GeoPoint>& points) {
    auto* object = db_.Get(key);
    if (object && object->GetType() != core::types::DataType::kGeo) {
        return std::unexpected(LogicError::kWrongType);
    }
    if (!object) {
        int64_t delta = PredictKeyOverhead(key) + GeoPredictor::PredictCreate();
        for (const auto& pt : points) {
            delta += core::types::GeoObject::PredictAddDelta(pt.member);
        }
        if (WillExceedLimit(delta)) {
            return std::unexpected(LogicError::kOom);
        }
        auto new_geo = core::types::DataObject::CreateGeo(&db_);
        for (const auto& pt : points) {
            new_geo.AsGeo().Add(pt.longitude, pt.latitude, pt.member);
        }
        db_.Set(key, std::move(new_geo));
    } else {
        auto& geo = object->AsGeo();
        for (const auto& pt : points) {
            if (WillExceedLimit(GeoPredictor::PredictAdd(pt.member))) {
                return std::unexpected(LogicError::kOom);
            }
            geo.Add(pt.longitude, pt.latitude, pt.member);
        }
    }
    return {};
}

DbResult<std::vector<std::optional<std::pair<double, double>>>> DaikonDataBase::Geopos(
    std::string_view key,
    const std::vector<std::string_view>& members) {
    auto* object = db_.Get(key);
    if (!object) {
        return std::vector<std::optional<std::pair<double, double>>>();
    }
    if (object->GetType() != core::types::DataType::kGeo) {
        return std::unexpected(LogicError::kWrongType);
    }
    return object->AsGeo().Pos(members);
}

DbResult<double> DaikonDataBase::Geodist(std::string_view key,
                                         std::string_view member1,
                                         std::string_view member2,
                                         std::string_view unit) {
    auto* object = db_.Get(key);
    if (!object) {
        return std::unexpected(LogicError::kNotFound);
    }
    if (object->GetType() != core::types::DataType::kGeo) {
        return std::unexpected(LogicError::kWrongType);
    }
    auto dist = object->AsGeo().Dist(member1, member2, unit);
    if (!dist.has_value()) {
        return std::unexpected(LogicError::kNotFound);
    }
    return *dist;
}

DbResult<results::ListViewResult> DaikonDataBase::Geosearch(
    std::string_view key, double lon, double lat,
    double radius, std::string_view unit, bool asc,
    int64_t count) {
    auto* object = db_.Get(key);
    if (!object) {
        return std::unexpected(LogicError::kNotFound);
    }
    if (object->GetType() != core::types::DataType::kGeo) {
        return std::unexpected(LogicError::kWrongType);
    }
    auto views = object->AsGeo().Search(lon, lat, radius, unit, asc,
                                        (count <= 0) ? std::nullopt : std::optional<int64_t>(count));

    std::vector<std::string> names;
    names.reserve(views.size());
    for (auto v : views) {
        names.emplace_back(v);
    }
    return results::ListViewResult{std::move(names)};
}

DbResult<results::ListViewResult> DaikonDataBase::Geosearchstore(
    std::string_view dest, std::string_view source,
    double lon, double lat, double radius,
    std::string_view unit, bool asc,
    int64_t count) {
    auto* src_obj = db_.Get(source);
    if (!src_obj) {
        return std::unexpected(LogicError::kNotFound);
    }
    if (src_obj->GetType() != core::types::DataType::kGeo) {
        return std::unexpected(LogicError::kWrongType);
    }

    auto points = src_obj->AsGeo().SearchPoints(lon, lat, radius, unit, asc,
                                                (count <= 0) ? std::nullopt : std::optional<int64_t>(count));

    int64_t delta = PredictKeyOverhead(dest) + GeoPredictor::PredictCreate();
    for (const auto& [name, pt] : points) {
        delta += core::types::GeoObject::PredictAddDelta(name);
    }
    if (WillExceedLimit(delta)) {
        return std::unexpected(LogicError::kOom);
    }

    auto dest_obj = core::types::DataObject::CreateGeo(&db_);
    for (const auto& [name, pt] : points) {
        dest_obj.AsGeo().Add(pt.longitude, pt.latitude, name);
    }
    db_.Set(dest, std::move(dest_obj));

    std::vector<std::string> names;
    names.reserve(points.size());
    for (const auto& [name, pt] : points) {
        names.emplace_back(name);
    }
    return results::ListViewResult{std::move(names)};
}

} // namespace daikon