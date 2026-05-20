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

using std::ostream;
using namespace daikon::commands;
using detail::WriteResult;
using DDB = DaikonDatabase;

inline void Handle(DDB& db, const SetArgs& a, ostream& os) { WriteResult(db.Set(a.key, a.value), os); }
inline void Handle(DDB& db, const GetArgs& a, ostream& os) { WriteResult(db.Get(a.key), os); }
inline void Handle(DDB& db, const StrlenArgs& a, ostream& os) { WriteResult(db.Strlen(a.key), os); }
inline void Handle(DDB& db, const AppendArgs& a, ostream& os) { WriteResult(db.Append(a.key, a.value), os); }

inline void Handle(DDB& db, const LpushArgs& a, ostream& os) { WriteResult(db.Lpush(a.key, a.values), os); }
inline void Handle(DDB& db, const RpushArgs& a, ostream& os) { WriteResult(db.Rpush(a.key, a.values), os); }
inline void Handle(DDB& db, const LpopArgs& a, ostream& os) { WriteResult(db.Lpop(a.key, a.count.value_or(1)), os); }
inline void Handle(DDB& db, const RpopArgs& a, ostream& os) { WriteResult(db.Rpop(a.key, a.count.value_or(1)), os); }
inline void Handle(DDB& db, const LlenArgs& a, ostream& os) { WriteResult(db.Llen(a.key), os); }
inline void Handle(DDB& db, const LrangeArgs& a, ostream& os) { WriteResult(db.Lrange(a.key, a.start, a.stop), os); }
inline void Handle(DDB& db, const LindexArgs& a, ostream& os) { WriteResult(db.Lindex(a.key, a.index), os); }
inline void Handle(DDB& db, const LsetArgs& a, ostream& os) { WriteResult(db.Lset(a.key, a.index, a.value), os); }
inline void Handle(DDB& db, const LinsertArgs& a, ostream& os) { WriteResult(db.Linsert(a.key, a.before, a.pivot, a.value), os); }

inline void Handle(DDB& db, const SaddArgs& a, ostream& os) { WriteResult(db.Sadd(a.key, a.members), os); }
inline void Handle(DDB& db, const SremArgs& a, ostream& os) { WriteResult(db.Srem(a.key, a.members), os); }
inline void Handle(DDB& db, const SismemberArgs& a, ostream& os) { WriteResult(db.Sismember(a.key, a.member), os); }
inline void Handle(DDB& db, const SmembersArgs& a, ostream& os) { WriteResult(db.Smembers(a.key), os); }
inline void Handle(DDB& db, const ScardArgs& a, ostream& os) { WriteResult(db.Scard(a.key), os); }
inline void Handle(DDB& db, const SunionArgs& a, ostream& os) { WriteResult(db.Sunion(a.keys), os); }
inline void Handle(DDB& db, const SinterArgs& a, ostream& os) { WriteResult(db.Sinter(a.keys), os); }
inline void Handle(DDB& db, const SdiffArgs& a, ostream& os) { WriteResult(db.Sdiff(a.keys), os); }
inline void Handle(DDB& db, const SmoveArgs& a, ostream& os) { WriteResult(db.Smove(a.source, a.destination, a.member), os); }

inline void Handle(DDB& db, const GeoaddArgs& a, ostream& os) { WriteResult(db.Geoadd(a.key, a.points), os); }
inline void Handle(DDB& db, const GeoposArgs& a, ostream& os) { WriteResult(db.Geopos(a.key, a.members), os); }
inline void Handle(DDB& db, const GeodistArgs& a, ostream& os) { WriteResult(db.Geodist(a.key, a.member1, a.member2, a.unit), os); }
inline void Handle(DDB& db, const GeosearchArgs& a, ostream& os) {
    WriteResult(db.Geosearch(a.key, a.longitude, a.latitude, a.radius, a.unit, a.asc, a.count.value_or(-1)), os);
}
inline void Handle(DDB& db, const GeosearchstoreArgs& a, ostream& os) {
    WriteResult(db.Geosearchstore(a.destination, a.source, a.longitude, a.latitude, a.radius, a.unit, a.asc, a.count.value_or(-1)), os);
}

inline void Handle(DDB& db, const ExpireArgs& a, ostream& os) { WriteResult(db.Expire(a.key, a.seconds), os); }
inline void Handle(DDB& db, const TtlArgs& a, ostream& os) { WriteResult(db.Ttl(a.key), os); }
inline void Handle(DDB& db, const TypeArgs& a, ostream& os) { os << db.Type(a.key).data; }
inline void Handle(DDB& db, const DelArgs& a, ostream& os) { os << db.Del(a.keys).value; }
inline void Handle(DDB& db, const ExistsArgs& a, ostream& os) { os << db.Exists(a.keys).value; }
inline void Handle(DDB& db, const FlushdbArgs&, ostream& os) {
    db.Flushdb();
    os << "OK";
}
inline void Handle(DDB& db, const DbsizeArgs&, ostream& os) { os << db.Dbsize().value; }

inline void Handle(DDB& db, const KeysArgs& a, ostream& os) {
    auto res = db.Keys(a.pattern);
    if (res.elements.empty()) return void(os << "(empty array)");
    for (size_t i = 0; i < res.elements.size(); ++i)
        os << (i ? " " : "") << res.elements[i];
}

inline void Handle(DDB& db, const MemoryUsageArgs& a, ostream& os) {
    auto res = db.MemoryUsage(a.key);
    if (res.value > 0)
        os << res.value;
    else
        os << (res.value == 0 ? "(nil)" : "ERR key not found");
}

inline void Handle(DDB& db, const ConfigSetArgs& a, ostream& os) {
    if (a.parameter == "maxmemory")
        WriteResult(db.ConfigSetMaxmemory(a.value), os);
    else
        os << "ERR Unknown option";
}

inline void Handle(DDB& db, const ConfigGetArgs& a, ostream& os) {
    if (a.parameter == "maxmemory")
        os << db.ConfigGetMaxmemory().value;
    else
        os << "ERR Unknown option";
}

} // namespace daikon::handlers