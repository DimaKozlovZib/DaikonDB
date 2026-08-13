#pragma once

#include <cstdint>
#include <string_view>

namespace daikon::commands::names {

constexpr uint64_t ComputeHash(std::string_view str) noexcept {
    uint64_t hash = 14695981039346656037ULL;
    for (char c : str) {
        char upper = (c >= 'a' && c <= 'z') ? (c - 'a' + 'A') : c;
        hash = (hash ^ static_cast<uint64_t>(upper)) * 1099511628211ULL;
    }
    return hash;
}

struct CommandInfo {
    std::string_view name;
    uint64_t hash;

    static bool IsEquals(std::string_view a, std::string_view b) {
        if (a.size() != b.size()) return false;
        for (size_t i = 0; i < a.size(); ++i) {
            if (std::toupper(a[i]) != std::toupper(b[i])) return false;
        }
        return true;
    }

    constexpr CommandInfo(const char* str)
        : name(str), hash(ComputeHash(str)) {}

    constexpr CommandInfo(std::string_view str)
        : name(str), hash(ComputeHash(str)) {}

    bool operator==(const CommandInfo& other) {
        return other.name.size() == name.size() && other.hash == hash && IsEquals(name, other.name);
    }
};

inline bool operator==(const CommandInfo& io, const std::string_view& str) {
    return str.size() == io.name.size() && CommandInfo::IsEquals(io.name, str);
}

constexpr CommandInfo kSet{"SET"};
constexpr CommandInfo kGet{"GET"};
constexpr CommandInfo kStrlen{"STRLEN"};
constexpr CommandInfo kAppend{"APPEND"};
constexpr CommandInfo kExpire{"EXPIRE"};
constexpr CommandInfo kTtl{"TTL"};

constexpr CommandInfo kLpush{"LPUSH"};
constexpr CommandInfo kRpush{"RPUSH"};
constexpr CommandInfo kLpop{"LPOP"};
constexpr CommandInfo kRpop{"RPOP"};
constexpr CommandInfo kLlen{"LLEN"};
constexpr CommandInfo kLrange{"LRANGE"};
constexpr CommandInfo kLindex{"LINDEX"};
constexpr CommandInfo kLset{"LSET"};
constexpr CommandInfo kLinsert{"LINSERT"};

constexpr CommandInfo kSadd{"SADD"};
constexpr CommandInfo kSrem{"SREM"};
constexpr CommandInfo kSismember{"SISMEMBER"};
constexpr CommandInfo kSmembers{"SMEMBERS"};
constexpr CommandInfo kScard{"SCARD"};
constexpr CommandInfo kSunion{"SUNION"};
constexpr CommandInfo kSinter{"SINTER"};
constexpr CommandInfo kSdiff{"SDIFF"};
constexpr CommandInfo kSmove{"SMOVE"};

constexpr CommandInfo kGeoadd{"GEOADD"};
constexpr CommandInfo kGeopos{"GEOPOS"};
constexpr CommandInfo kGeodist{"GEODIST"};
constexpr CommandInfo kGeosearch{"GEOSEARCH"};
constexpr CommandInfo kGeosearchstore{"GEOSEARCHSTORE"};

constexpr CommandInfo kType{"TYPE"};
constexpr CommandInfo kDel{"DEL"};
constexpr CommandInfo kKeys{"KEYS"};
constexpr CommandInfo kExists{"EXISTS"};
constexpr CommandInfo kFlushdb{"FLUSHDB"};
constexpr CommandInfo kConfig{"CONFIG"};
constexpr CommandInfo kDbsize{"DBSIZE"};
constexpr CommandInfo kMemory{"MEMORY"};

} // namespace daikon::commands::names