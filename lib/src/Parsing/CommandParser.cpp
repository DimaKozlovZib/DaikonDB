#include <cctype>
#include <cstddef>
#include <optional>
#include <span>
#include <string_view>

#include "../../includes/Commands.h"
#include "../../includes/parsing/CommandNames.h"
#include "../../includes/parsing/Parsing.h"
#include "../../includes/parsing/Types.h"

namespace daikon::parsing {

std::optional<commands::Command>
CommandParser::Dispatch(std::string_view cmd, std::span<const std::string_view> args) {
    using namespace daikon::parsing::args;
    using namespace daikon::commands;
    using namespace daikon::commands::names;

    CommandInfo current_cmd = cmd;

    // String
    if (current_cmd == kSet) return TryParse<SetArgs, Key, Value>(args);
    if (current_cmd == kGet) return TryParse<GetArgs, Key>(args);
    if (current_cmd == kStrlen) return TryParse<StrlenArgs, Key>(args);
    if (current_cmd == kAppend) return TryParse<AppendArgs, Key, Value>(args);

    // List
    if (current_cmd == kLpush) return TryParse<LpushArgs, Key, VarArgs<Value>>(args);
    if (current_cmd == kRpush) return TryParse<RpushArgs, Key, VarArgs<Value>>(args);
    if (current_cmd == kLpop) return TryParse<LpopArgs, Key, OptionalInt64>(args);
    if (current_cmd == kRpop) return TryParse<RpopArgs, Key, OptionalInt64>(args);
    if (current_cmd == kLlen) return TryParse<LlenArgs, Key>(args);
    if (current_cmd == kLrange) return TryParse<LrangeArgs, Key, Int64, Int64>(args);
    if (current_cmd == kLindex) return TryParse<LindexArgs, Key, Int64>(args);
    if (current_cmd == kLset) return TryParse<LsetArgs, Key, Int64, Value>(args);
    if (current_cmd == kLinsert) return TryParse<LinsertArgs, Key, BeforeAfter, Value, Value>(args);

    // Set
    if (current_cmd == kSadd) return TryParse<SaddArgs, Key, VarArgs<Value>>(args);
    if (current_cmd == kSrem) return TryParse<SremArgs, Key, VarArgs<Value>>(args);
    if (current_cmd == kSismember) return TryParse<SismemberArgs, Key, Value>(args);
    if (current_cmd == kSmembers) return TryParse<SmembersArgs, Key>(args);
    if (current_cmd == kScard) return TryParse<ScardArgs, Key>(args);
    if (current_cmd == kSunion) return TryParse<SunionArgs, VarArgs<Key>>(args);
    if (current_cmd == kSinter) return TryParse<SinterArgs, VarArgs<Key>>(args);
    if (current_cmd == kSdiff) return TryParse<SdiffArgs, VarArgs<Key>>(args);
    if (current_cmd == kSmove) return TryParse<SmoveArgs, Key, Key, Value>(args);

    // Geo
    if (current_cmd == kGeoadd) return TryParse<GeoaddArgs, Key, VarArgs<GeoPoint>>(args);
    if (current_cmd == kGeopos) return TryParse<GeoposArgs, Key, VarArgs<Value>>(args);
    if (current_cmd == kGeodist) return TryParse<GeodistArgs, Key, Value, Value, Unit>(args);
    if (current_cmd == kGeosearch) return TryParse<GeosearchArgs, Key, FromLonLat, Double, Double, ByRadius, Double, Unit, AscDesc, GeoCount>(args);
    if (current_cmd == kGeosearchstore) return TryParse<GeosearchstoreArgs, Key, Key, FromLonLat, Double, Double, ByRadius, Double, Unit, AscDesc, GeoCount>(args);

    // Generic
    if (current_cmd == kType) return TryParse<TypeArgs, Key>(args);
    if (current_cmd == kDel) return TryParse<DelArgs, VarArgs<Key>>(args);
    if (current_cmd == kKeys) return TryParse<KeysArgs, Value>(args);
    if (current_cmd == kFlushdb) return TryParse<FlushdbArgs>(args);
    if (current_cmd == kDbsize) return TryParse<DbsizeArgs>(args);
    if (current_cmd == kExists) return TryParse<ExistsArgs, VarArgs<Key>>(args);
    if (current_cmd == kExpire) return TryParse<ExpireArgs, Key, Int64>(args);
    if (current_cmd == kTtl) return TryParse<TtlArgs, Key>(args);

    if (current_cmd == kConfig && args.size() >= 1) {
        if (kSet == args[0] && args.size() == 3)
            return TryParse<ConfigSetArgs, Value, Int64>(args.subspan(1));
        if (kGet == args[0] && args.size() == 2)
            return TryParse<ConfigGetArgs, Value>(args.subspan(1));
    }
    if (current_cmd == kMemory) {
        if (args.size() == 2 && CommandInfo::IsEquals(args[0], "USAGE"))
            return TryParse<MemoryUsageArgs, Key>(args.subspan(1));
    }

    return std::nullopt;
}
} // namespace daikon::parsing
