#include <charconv>
#include <optional>
#include <span>
#include <string>
#include <type_traits>

#include "../../includes/Parsing/Parsing.h"

namespace daikon::parsing {

bool CommandParser::IsEquals(std::string_view a, std::string_view b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i) {
        if (std::toupper(a[i]) != std::toupper(b[i])) return false;
    }
    return true;
}

std::optional<commands::Command>
CommandParser::Dispatch(std::string_view cmd, std::span<const std::string_view> args) {
    using namespace daikon::parsing::args;
    using namespace daikon::commands;
    using namespace daikon::commands::names;

    // String
    if (IsEquals(cmd, SET)) return TryParse<SetArgs, Key, Value>(args);
    if (IsEquals(cmd, GET)) return TryParse<GetArgs, Key>(args);
    if (IsEquals(cmd, STRLEN)) return TryParse<StrlenArgs, Key>(args);
    if (IsEquals(cmd, APPEND)) return TryParse<AppendArgs, Key, Value>(args);
    if (IsEquals(cmd, EXPIRE)) return TryParse<ExpireArgs, Key, Int64>(args);
    if (IsEquals(cmd, TTL)) return TryParse<TtlArgs, Key>(args);

    // List
    if (IsEquals(cmd, LPUSH)) return TryParse<LpushArgs, Key, VarArgs<Value>>(args);
    if (IsEquals(cmd, RPUSH)) return TryParse<RpushArgs, Key, VarArgs<Value>>(args);
    if (IsEquals(cmd, LPOP)) return TryParse<LpopArgs, Key, OptionalInt64>(args);
    if (IsEquals(cmd, RPOP)) return TryParse<RpopArgs, Key, OptionalInt64>(args);
    if (IsEquals(cmd, LLEN)) return TryParse<LlenArgs, Key>(args);
    if (IsEquals(cmd, LRANGE)) return TryParse<LrangeArgs, Key, Int64, Int64>(args);
    if (IsEquals(cmd, LINDEX)) return TryParse<LindexArgs, Key, Int64>(args);
    if (IsEquals(cmd, LSET)) return TryParse<LsetArgs, Key, Int64, Value>(args);
    if (IsEquals(cmd, LINSERT)) return TryParse<LinsertArgs, Key, BeforeAfter, Value, Value>(args);

    // Set
    if (IsEquals(cmd, SADD)) return TryParse<SaddArgs, Key, VarArgs<Value>>(args);
    if (IsEquals(cmd, SREM)) return TryParse<SremArgs, Key, VarArgs<Value>>(args);
    if (IsEquals(cmd, SISMEMBER)) return TryParse<SismemberArgs, Key, Value>(args);
    if (IsEquals(cmd, SMEMBERS)) return TryParse<SmembersArgs, Key>(args);
    if (IsEquals(cmd, SCARD)) return TryParse<ScardArgs, Key>(args);
    if (IsEquals(cmd, SUNION)) return TryParse<SunionArgs, VarArgs<Key>>(args);
    if (IsEquals(cmd, SINTER)) return TryParse<SinterArgs, VarArgs<Key>>(args);
    if (IsEquals(cmd, SDIFF)) return TryParse<SdiffArgs, VarArgs<Key>>(args);
    if (IsEquals(cmd, SMOVE)) return TryParse<SmoveArgs, Key, Key, Value>(args);

    // Geo
    if (IsEquals(cmd, GEOADD)) return TryParse<GeoaddArgs, Key, VarArgs<GeoPoint>>(args);
    if (IsEquals(cmd, GEOPOS)) return TryParse<GeoposArgs, Key, VarArgs<Value>>(args);
    if (IsEquals(cmd, GEODIST)) return TryParse<GeodistArgs, Key, Value, Value, Unit>(args);
    if (IsEquals(cmd, GEOSEARCH)) return TryParse<GeosearchArgs, Key, Double, Double, Double, Unit, BeforeAfter, OptionalInt64>(args);
    if (IsEquals(cmd, GEOSEARCHSTORE)) return TryParse<GeosearchstoreArgs, Key, Key, Double, Double, Double, Unit, BeforeAfter, OptionalInt64>(args);

    // Generic
    if (IsEquals(cmd, TYPE)) return TryParse<TypeArgs, Key>(args);
    if (IsEquals(cmd, DEL)) return TryParse<DelArgs, VarArgs<Key>>(args);
    if (IsEquals(cmd, KEYS)) return TryParse<KeysArgs, Value>(args);
    if (IsEquals(cmd, FLUSHDB)) return TryParse<FlushdbArgs>(args);
    if (IsEquals(cmd, DBSIZE)) return TryParse<DbsizeArgs>(args);

    if (IsEquals(cmd, CONFIG) && args.size() >= 1) {
        if (IsEquals(args[0], "SET") && args.size() == 3)
            return TryParse<ConfigSetArgs, Value, Value>(args.subspan(1));
        if (IsEquals(args[0], "GET") && args.size() == 2)
            return TryParse<ConfigGetArgs, Value>(args.subspan(1));
    }
    if (IsEquals(cmd, MEMORY)) {
        if (args.size() == 2 && IsEquals(args[0], "USAGE"))
            return TryParse<MemoryUsageArgs, Key>(args.subspan(1));
    }

    return std::nullopt;
}
} // namespace daikon::parsing
