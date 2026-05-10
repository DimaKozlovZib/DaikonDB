#pragma once

#include <string_view>

namespace daikon::commands::names {

constexpr std::string_view SET = "SET";
constexpr std::string_view GET = "GET";
constexpr std::string_view STRLEN = "STRLEN";
constexpr std::string_view APPEND = "APPEND";
constexpr std::string_view EXPIRE = "EXPIRE";
constexpr std::string_view TTL = "TTL";

constexpr std::string_view LPUSH = "LPUSH";
constexpr std::string_view RPUSH = "RPUSH";
constexpr std::string_view LPOP = "LPOP";
constexpr std::string_view RPOP = "RPOP";
constexpr std::string_view LLEN = "LLEN";
constexpr std::string_view LRANGE = "LRANGE";
constexpr std::string_view LINDEX = "LINDEX";
constexpr std::string_view LSET = "LSET";
constexpr std::string_view LINSERT = "LINSERT";

constexpr std::string_view SADD = "SADD";
constexpr std::string_view SREM = "SREM";
constexpr std::string_view SISMEMBER = "SISMEMBER";
constexpr std::string_view SMEMBERS = "SMEMBERS";
constexpr std::string_view SCARD = "SCARD";
constexpr std::string_view SUNION = "SUNION";
constexpr std::string_view SINTER = "SINTER";
constexpr std::string_view SDIFF = "SDIFF";
constexpr std::string_view SMOVE = "SMOVE";

constexpr std::string_view GEOADD = "GEOADD";
constexpr std::string_view GEOPOS = "GEOPOS";
constexpr std::string_view GEODIST = "GEODIST";
constexpr std::string_view GEOSEARCH = "GEOSEARCH";
constexpr std::string_view GEOSEARCHSTORE = "GEOSEARCHSTORE";

constexpr std::string_view TYPE = "TYPE";
constexpr std::string_view DEL = "DEL";
constexpr std::string_view KEYS = "KEYS";
constexpr std::string_view EXISTS = "EXISTS";
constexpr std::string_view FLUSHDB = "FLUSHDB";
constexpr std::string_view CONFIG = "CONFIG";
constexpr std::string_view DBSIZE = "DBSIZE";
constexpr std::string_view MEMORY = "MEMORY";
} // namespace daikon::commands::names