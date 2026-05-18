#include <optional>
#include <span>
#include <string_view>
#include <cstddef>
#include <cctype> 

#include "../../includes/parsing/Parsing.h"
#include "../../includes/Commands.h"
#include "../../includes/parsing/Types.h"

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
    if (IsEquals(cmd, kSet)) return TryParse<SetArgs, Key, Value>(args);
    if (IsEquals(cmd, kGet)) return TryParse<GetArgs, Key>(args);
    if (IsEquals(cmd, kStrlen)) return TryParse<StrlenArgs, Key>(args);
    if (IsEquals(cmd, kAppend)) return TryParse<AppendArgs, Key, Value>(args);
    if (IsEquals(cmd, kExpire)) return TryParse<ExpireArgs, Key, Int64>(args);
    if (IsEquals(cmd, kTtl)) return TryParse<TtlArgs, Key>(args);

    // List
    if (IsEquals(cmd, kLpush)) return TryParse<LpushArgs, Key, VarArgs<Value>>(args);
    if (IsEquals(cmd, kRpush)) return TryParse<RpushArgs, Key, VarArgs<Value>>(args);
    if (IsEquals(cmd, kLpop)) return TryParse<LpopArgs, Key, OptionalInt64>(args);
    if (IsEquals(cmd, kRpop)) return TryParse<RpopArgs, Key, OptionalInt64>(args);
    if (IsEquals(cmd, kLlen)) return TryParse<LlenArgs, Key>(args);
    if (IsEquals(cmd, kLrange)) return TryParse<LrangeArgs, Key, Int64, Int64>(args);
    if (IsEquals(cmd, kLindex)) return TryParse<LindexArgs, Key, Int64>(args);
    if (IsEquals(cmd, kLset)) return TryParse<LsetArgs, Key, Int64, Value>(args);
    if (IsEquals(cmd, kLinsert)) return TryParse<LinsertArgs, Key, BeforeAfter, Value, Value>(args);

    // Set
    if (IsEquals(cmd, kSadd)) return TryParse<SaddArgs, Key, VarArgs<Value>>(args);
    if (IsEquals(cmd, kSrem)) return TryParse<SremArgs, Key, VarArgs<Value>>(args);
    if (IsEquals(cmd, kSismember)) return TryParse<SismemberArgs, Key, Value>(args);
    if (IsEquals(cmd, kSmembers)) return TryParse<SmembersArgs, Key>(args);
    if (IsEquals(cmd, kScard)) return TryParse<ScardArgs, Key>(args);
    if (IsEquals(cmd, kSunion)) return TryParse<SunionArgs, VarArgs<Key>>(args);
    if (IsEquals(cmd, kSinter)) return TryParse<SinterArgs, VarArgs<Key>>(args);
    if (IsEquals(cmd, kSdiff)) return TryParse<SdiffArgs, VarArgs<Key>>(args);
    if (IsEquals(cmd, kSmove)) return TryParse<SmoveArgs, Key, Key, Value>(args);

    // Geo
    if (IsEquals(cmd, kGeoadd)) return TryParse<GeoaddArgs, Key, VarArgs<GeoPoint>>(args);
    if (IsEquals(cmd, kGeopos)) return TryParse<GeoposArgs, Key, VarArgs<Value>>(args);
    if (IsEquals(cmd, kGeodist)) return TryParse<GeodistArgs, Key, Value, Value, Unit>(args);
    if (IsEquals(cmd, kGeosearch)) return TryParse<GeosearchArgs, Key, Double, Double, Double, Unit, BeforeAfter, OptionalInt64>(args);
    if (IsEquals(cmd, kGeosearchstore)) return TryParse<GeosearchstoreArgs, Key, Key, Double, Double, Double, Unit, BeforeAfter, OptionalInt64>(args);

    // Generic
    if (IsEquals(cmd, kType)) return TryParse<TypeArgs, Key>(args);
    if (IsEquals(cmd, kDel)) return TryParse<DelArgs, VarArgs<Key>>(args);
    if (IsEquals(cmd, kKeys)) return TryParse<KeysArgs, Value>(args);
    if (IsEquals(cmd, kFlushdb)) return TryParse<FlushdbArgs>(args);
    if (IsEquals(cmd, kDbsize)) return TryParse<DbsizeArgs>(args);

    if (IsEquals(cmd, kConfig) && args.size() >= 1) {
        if (IsEquals(args[0], kSet) && args.size() == 3)
            return TryParse<ConfigSetArgs, Value, Int64>(args.subspan(1));
        if (IsEquals(args[0], kGet) && args.size() == 2)
            return TryParse<ConfigGetArgs, Value>(args.subspan(1));
    }
    if (IsEquals(cmd, kMemory)) {
        if (args.size() == 2 && IsEquals(args[0], "USAGE"))
            return TryParse<MemoryUsageArgs, Key>(args.subspan(1));
    }

    return std::nullopt;
}
} // namespace daikon::parsing
