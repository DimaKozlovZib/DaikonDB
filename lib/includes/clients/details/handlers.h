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

namespace daikon::handlers {
inline void Handle(DaikonDatabase& db, const commands::SetArgs& a, std::ostream& os) { detail::WriteResult(db.Set(a.key, a.value), os); }
inline void Handle(DaikonDatabase& db, const commands::GetArgs& a, std::ostream& os) { detail::WriteResult(db.Get(a.key), os); }
inline void Handle(DaikonDatabase& db, const commands::StrlenArgs& a, std::ostream& os) { detail::WriteResult(db.Strlen(a.key), os); }
inline void Handle(DaikonDatabase& db, const commands::AppendArgs& a, std::ostream& os) { detail::WriteResult(db.Append(a.key, a.value), os); }
inline void Handle(DaikonDatabase& db, const commands::ExpireArgs& a, std::ostream& os) { detail::WriteResult(db.Expire(a.key, a.seconds), os); }
inline void Handle(DaikonDatabase& db, const commands::TtlArgs& a, std::ostream& os) { detail::WriteResult(db.Ttl(a.key), os); }
inline void Handle(DaikonDatabase& db, const commands::LpushArgs& a, std::ostream& os) { detail::WriteResult(db.Lpush(a.key, a.values), os); }
inline void Handle(DaikonDatabase& db, const commands::RpushArgs& a, std::ostream& os) { detail::WriteResult(db.Rpush(a.key, a.values), os); }
inline void Handle(DaikonDatabase& db, const commands::LpopArgs& a, std::ostream& os) { detail::WriteResult(db.Lpop(a.key, a.count.value_or(1)), os); }
inline void Handle(DaikonDatabase& db, const commands::RpopArgs& a, std::ostream& os) { detail::WriteResult(db.Rpop(a.key, a.count.value_or(1)), os); }
inline void Handle(DaikonDatabase& db, const commands::LlenArgs& a, std::ostream& os) { detail::WriteResult(db.Llen(a.key), os); }
inline void Handle(DaikonDatabase& db, const commands::LrangeArgs& a, std::ostream& os) { detail::WriteResult(db.Lrange(a.key, a.start, a.stop), os); }
inline void Handle(DaikonDatabase& db, const commands::LindexArgs& a, std::ostream& os) { detail::WriteResult(db.Lindex(a.key, a.index), os); }
inline void Handle(DaikonDatabase& db, const commands::LsetArgs& a, std::ostream& os) { detail::WriteResult(db.Lset(a.key, a.index, a.value), os); }
inline void Handle(DaikonDatabase& db, const commands::LinsertArgs& a, std::ostream& os) { detail::WriteResult(db.Linsert(a.key, a.before, a.pivot, a.value), os); }
inline void Handle(DaikonDatabase& db, const commands::SaddArgs& a, std::ostream& os) { detail::WriteResult(db.Sadd(a.key, a.members), os); }
inline void Handle(DaikonDatabase& db, const commands::SremArgs& a, std::ostream& os) { detail::WriteResult(db.Srem(a.key, a.members), os); }
inline void Handle(DaikonDatabase& db, const commands::SismemberArgs& a, std::ostream& os) { detail::WriteResult(db.Sismember(a.key, a.member), os); }
inline void Handle(DaikonDatabase& db, const commands::SmembersArgs& a, std::ostream& os) { detail::WriteResult(db.Smembers(a.key), os); }
inline void Handle(DaikonDatabase& db, const commands::ScardArgs& a, std::ostream& os) { detail::WriteResult(db.Scard(a.key), os); }
inline void Handle(DaikonDatabase& db, const commands::SunionArgs& a, std::ostream& os) { detail::WriteResult(db.Sunion(a.keys), os); }
inline void Handle(DaikonDatabase& db, const commands::SinterArgs& a, std::ostream& os) { detail::WriteResult(db.Sinter(a.keys), os); }
inline void Handle(DaikonDatabase& db, const commands::SdiffArgs& a, std::ostream& os) { detail::WriteResult(db.Sdiff(a.keys), os); }
inline void Handle(DaikonDatabase& db, const commands::SmoveArgs& a, std::ostream& os) { detail::WriteResult(db.Smove(a.source, a.destination, a.member), os); }
inline void Handle(DaikonDatabase& db, const commands::GeoaddArgs& a, std::ostream& os) { detail::WriteResult(db.Geoadd(a.key, a.points), os); }
inline void Handle(DaikonDatabase& db, const commands::GeoposArgs& a, std::ostream& os) { detail::WriteResult(db.Geopos(a.key, a.members), os); }
inline void Handle(DaikonDatabase& db, const commands::GeodistArgs& a, std::ostream& os) { detail::WriteResult(db.Geodist(a.key, a.member1, a.member2, a.unit), os); }
inline void Handle(DaikonDatabase& db, const commands::GeosearchArgs& a, std::ostream& os) {
    detail::WriteResult(db.Geosearch(a.key, a.longitude, a.latitude, a.radius, a.unit, a.asc, a.count.value_or(-1)), os);
}
inline void Handle(DaikonDatabase& db, const commands::GeosearchstoreArgs& a, std::ostream& os) {
    detail::WriteResult(db.Geosearchstore(a.destination, a.source, a.longitude, a.latitude, a.radius, a.unit, a.asc, a.count.value_or(-1)), os);
}
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