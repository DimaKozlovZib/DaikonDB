#pragma once

#include <string_view>

namespace daikon::commands::names {

constexpr std::string_view kSet = "SET";
constexpr std::string_view kGet = "GET";
constexpr std::string_view kStrlen = "STRLEN";
constexpr std::string_view kAppend = "APPEND";
constexpr std::string_view kExpire = "EXPIRE";
constexpr std::string_view kTtl = "TTL";

constexpr std::string_view kLpush = "LPUSH";
constexpr std::string_view kRpush = "RPUSH";
constexpr std::string_view kLpop = "LPOP";
constexpr std::string_view kRpop = "RPOP";
constexpr std::string_view kLlen = "LLEN";
constexpr std::string_view kLrange = "LRANGE";
constexpr std::string_view kLindex = "LINDEX";
constexpr std::string_view kLset = "LSET";
constexpr std::string_view kLinsert = "LINSERT";

constexpr std::string_view kSadd = "SADD";
constexpr std::string_view kSrem = "SREM";
constexpr std::string_view kSismember = "SISMEMBER";
constexpr std::string_view kSmembers = "SMEMBERS";
constexpr std::string_view kScard = "SCARD";
constexpr std::string_view kSunion = "SUNION";
constexpr std::string_view kSinter = "SINTER";
constexpr std::string_view kSdiff = "SDIFF";
constexpr std::string_view kSmove = "SMOVE";

constexpr std::string_view kGeoadd = "GEOADD";
constexpr std::string_view kGeopos = "GEOPOS";
constexpr std::string_view kGeodist = "GEODIST";
constexpr std::string_view kGeosearch = "GEOSEARCH";
constexpr std::string_view kGeosearchstore = "GEOSEARCHSTORE";

constexpr std::string_view kType = "TYPE";
constexpr std::string_view kDel = "DEL";
constexpr std::string_view kKeys = "KEYS";
constexpr std::string_view kExists = "EXISTS";
constexpr std::string_view kFlushdb = "FLUSHDB";
constexpr std::string_view kConfig = "CONFIG";
constexpr std::string_view kDbsize = "DBSIZE";
constexpr std::string_view kMemory = "MEMORY";

} // namespace daikon::commands::names