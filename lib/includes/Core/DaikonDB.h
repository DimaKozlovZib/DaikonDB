#pragma once

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstddef>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "../Commands.h"
#include "storage/database.h"
#include "results.h"
#include "types/Geo.h"
#include "types/List.h"
#include "types/Set.h"
#include "types/String.h"

namespace daikon {

class DaikonDatabase {
   public:
    explicit DaikonDatabase(size_t maxmemory = 0)
        : maxmemory_(maxmemory) {}

    void SetMaxMemory(size_t mem) { maxmemory_ = mem; }
    size_t GetMaxMemory() const { return maxmemory_; }

    DbResult<results::StatusResult> Set(std::string_view key, std::string_view value);
    DbResult<results::ViewResult> Get(std::string_view key);
    DbResult<results::IntResult> Strlen(std::string_view key);
    DbResult<void> Append(std::string_view key, std::string_view value);
    //
    DbResult<void> Lpush(std::string_view key, const std::vector<std::string_view>& values);
    DbResult<void> Rpush(std::string_view key, const std::vector<std::string_view>& values);
    DbResult<results::ListViewResult> Lpop(std::string_view key, int64_t count = 1);
    DbResult<results::ListViewResult> Rpop(std::string_view key, int64_t count = 1);

    DbResult<results::IntResult> Llen(std::string_view key);
    DbResult<results::ListViewResult> Lrange(std::string_view key, int64_t start, int64_t stop);
    DbResult<results::ViewResult> Lindex(std::string_view key, int64_t index);
    DbResult<void> Lset(std::string_view key, int64_t index, std::string_view value);
    DbResult<results::IntResult> Linsert(std::string_view key, bool before,
                                         std::string_view pivot, std::string_view value);
    //
    DbResult<results::IntResult> Sadd(std::string_view key, const std::vector<std::string_view>& members);
    DbResult<results::IntResult> Srem(std::string_view key, const std::vector<std::string_view>& members);
    DbResult<results::IntResult> Sismember(std::string_view key, std::string_view member);
    DbResult<results::ListViewResult> Smembers(std::string_view key);
    DbResult<results::IntResult> Scard(std::string_view key);
    //
    DbResult<results::ListViewResult> Sunion(const std::vector<std::string_view>& keys);
    DbResult<results::ListViewResult> Sinter(const std::vector<std::string_view>& keys);
    DbResult<results::ListViewResult> Sdiff(const std::vector<std::string_view>& keys);

    DbResult<results::StatusResult> Smove(std::string_view source, std::string_view destination,
                                          std::string_view member);
    //
    DbResult<void> Geoadd(std::string_view key, const std::vector<commands::GeoPoint>& points);
    DbResult<std::vector<std::optional<std::pair<double, double>>>> Geopos(std::string_view key,
                                                                           const std::vector<std::string_view>& members);
    DbResult<double> Geodist(std::string_view key, std::string_view member1,
                                         std::string_view member2, std::string_view unit = "m");
    DbResult<results::ListViewResult> Geosearch(std::string_view key, double lon, double lat,
                                                double radius, std::string_view unit, bool asc,
                                                int64_t count = -1);
    DbResult<results::ListViewResult> Geosearchstore(std::string_view dest, std::string_view source,
                                                     double lon, double lat, double radius,
                                                     std::string_view unit, bool asc,
                                                     int64_t count = -1);
    //
    results::ViewResult Type(std::string_view key);
    results::IntResult Del(const std::vector<std::string_view>& keys);
    results::IntResult Exists(const std::vector<std::string_view>& keys);
    results::StatusResult ConfigSetMaxmemory(int64_t bytes);
    results::ListViewResult Keys(std::string_view pattern);
    void Flushdb();
    results::IntResult ConfigGetMaxmemory();
    results::IntResult Dbsize();
    results::IntResult MemoryUsage(std::string_view key);
    results::StatusResult Expire(std::string_view key, int64_t seconds);
    results::IntResult Ttl(std::string_view key);

    size_t GetTotalRamUsage() const {
        return db_.GetTotalRamUsage();
    }

    bool IsMemoryFull() {
        return maxmemory_ > 0 && db_.GetTotalRamUsage() >= maxmemory_;
    }

   private:
    core::DBase db_;
    size_t maxmemory_;

    bool WillExceedLimit(int64_t delta) const {
        if (maxmemory_ == 0) return false;
        return (db_.GetTotalRamUsage() + delta) > maxmemory_;
    }

    int64_t PredictKeyOverhead(std::string_view key) {
        return static_cast<int64_t>(key.size() + 32);
    }
};

} // namespace daikon