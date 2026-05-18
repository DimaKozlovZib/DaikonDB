#pragma once

#include <functional>
#include <iostream>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "../../Commands.h"
#include "../../core/DaikonDB.h"
#include "../../parsing/CommandNames.h"
#include "../../parsing/Parsing.h"
#include "../../parsing/Types.h"
#include "output.h"

#define DEFINE_HANDLE(Cmd, Method, ...)                                                      \
    inline void Handle(DaikonDatabase& db, const commands::Cmd##Args& a, std::ostream& os) { \
        detail::WriteResult(db.Method(__VA_ARGS__), os);                                     \
    }

namespace daikon::handlers {
DEFINE_HANDLE(Get, Get, a.key)
DEFINE_HANDLE(Strlen, Strlen, a.key)
DEFINE_HANDLE(Ttl, Ttl, a.key)
DEFINE_HANDLE(Llen, Llen, a.key)
DEFINE_HANDLE(Smembers, Smembers, a.key)
DEFINE_HANDLE(Scard, Scard, a.key)

DEFINE_HANDLE(Sunion, Sunion, a.keys)
DEFINE_HANDLE(Sinter, Sinter, a.keys)
DEFINE_HANDLE(Sdiff, Sdiff, a.keys)

DEFINE_HANDLE(Set, Set, a.key, a.value)
DEFINE_HANDLE(Append, Append, a.key, a.value)
DEFINE_HANDLE(Expire, Expire, a.key, a.seconds)
DEFINE_HANDLE(Lpush, Lpush, a.key, a.values)
DEFINE_HANDLE(Rpush, Rpush, a.key, a.values)
DEFINE_HANDLE(Sadd, Sadd, a.key, a.members)
DEFINE_HANDLE(Srem, Srem, a.key, a.members)
DEFINE_HANDLE(Geopos, Geopos, a.key, a.members)
DEFINE_HANDLE(Geoadd, Geoadd, a.key, a.points)

DEFINE_HANDLE(Lindex, Lindex, a.key, a.index)
DEFINE_HANDLE(Lset, Lset, a.key, a.index, a.value)
DEFINE_HANDLE(Sismember, Sismember, a.key, a.member)

DEFINE_HANDLE(Lpop, Lpop, a.key, a.count.value_or(1))
DEFINE_HANDLE(Rpop, Rpop, a.key, a.count.value_or(1))
DEFINE_HANDLE(Lrange, Lrange, a.key, a.start, a.stop)
DEFINE_HANDLE(Linsert, Linsert, a.key, a.before, a.pivot, a.value)
DEFINE_HANDLE(Smove, Smove, a.source, a.destination, a.member)

DEFINE_HANDLE(Geodist, Geodist, a.key, a.member1, a.member2, a.unit)
DEFINE_HANDLE(Geosearch, Geosearch, a.key, a.longitude, a.latitude, a.radius, a.unit, a.asc, a.count.value_or(-1))
DEFINE_HANDLE(Geosearchstore, Geosearchstore, a.destination, a.source, a.longitude, a.latitude, a.radius, a.unit, a.asc, a.count.value_or(-1))

#undef DEFINE_HANDLE

inline void Handle(DaikonDatabase& db, const commands::TypeArgs& a, std::ostream& os) { os << db.Type(a.key).data; }
inline void Handle(DaikonDatabase& db, const commands::DelArgs& a, std::ostream& os) { os << db.Del(a.keys).value; }
inline void Handle(DaikonDatabase& db, const commands::ExistsArgs& a, std::ostream& os) { os << db.Exists(a.keys).value; }
inline void Handle(DaikonDatabase& db, const commands::KeysArgs& a, std::ostream& os) {
    auto res = db.Keys(a.pattern);
    if (res.elements.empty())
        os << "(empty array)";
    else
        for (size_t i = 0; i < res.elements.size(); ++i)
            os << (i ? " " : "") << res.elements[i];
}
inline void Handle(DaikonDatabase& db, const commands::FlushdbArgs&, std::ostream& os) {
    db.Flushdb();
    os << "OK";
}
inline void Handle(DaikonDatabase& db, const commands::DbsizeArgs&, std::ostream& os) { os << db.Dbsize().value; }
inline void Handle(DaikonDatabase& db, const commands::MemoryUsageArgs& a, std::ostream& os) {
    auto res = db.MemoryUsage(a.key);
    if (res.value > 0)
        os << res.value;
    else
        os << (res.value == 0 ? "(nil)" : "ERR key not found");
}
inline void Handle(DaikonDatabase& db, const commands::ConfigSetArgs& a, std::ostream& os) {
    if (a.parameter == "maxmemory") {
        db.SetMaxMemory(static_cast<size_t>(a.value));
        os << "OK";
    } else
        os << "ERR unknown config param";
}
inline void Handle(DaikonDatabase& db, const commands::ConfigGetArgs& a, std::ostream& os) {
    if (a.parameter == "maxmemory")
        os << db.GetMaxMemory();
    else
        os << "ERR unknown config param";
}
} // namespace daikon::handlers