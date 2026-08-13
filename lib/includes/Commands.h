#pragma once

#include <optional>
#include <string>
#include <variant>
#include <vector>
#include <cstdint>

namespace daikon::commands {
struct SetArgs {
    std::string_view key;
    std::string_view value;
};
struct GetArgs {
    std::string_view key;
};
struct StrlenArgs {
    std::string_view key;
};
struct AppendArgs {
    std::string_view key;
    std::string_view value;
};
struct ExpireArgs {
    std::string_view key;
    int64_t seconds;
};
struct TtlArgs {
    std::string_view key;
};

//
struct LpushArgs {
    std::string_view key;
    std::vector<std::string_view> values;
};
struct RpushArgs {
    std::string_view key;
    std::vector<std::string_view> values;
};
struct LpopArgs {
    std::string_view key;
    std::optional<int64_t> count;
};
struct RpopArgs {
    std::string_view key;
    std::optional<int64_t> count;
};
struct LlenArgs {
    std::string_view key;
};
struct LrangeArgs {
    std::string_view key;
    int64_t start;
    int64_t stop;
};
struct LindexArgs {
    std::string_view key;
    int64_t index;
};
struct LsetArgs {
    std::string_view key;
    int64_t index;
    std::string_view value;
};
struct LinsertArgs {
    std::string_view key;
    bool before;
    std::string_view pivot;
    std::string_view value;
};

//
struct SaddArgs {
    std::string_view key;
    std::vector<std::string_view> members;
};
struct SremArgs {
    std::string_view key;
    std::vector<std::string_view> members;
};
struct SismemberArgs {
    std::string_view key;
    std::string_view member;
};
struct SmembersArgs {
    std::string_view key;
};
struct ScardArgs {
    std::string_view key;
};
struct SunionArgs {
    std::vector<std::string_view> keys;
};
struct SinterArgs {
    std::vector<std::string_view> keys;
};
struct SdiffArgs {
    std::vector<std::string_view> keys;
};
struct SmoveArgs {
    std::string_view source;
    std::string_view destination;
    std::string_view member;
};

//
struct GeoPoint {
    double longitude;
    double latitude;
    std::string_view member;
};
struct GeoaddArgs {
    std::string_view key;
    std::vector<GeoPoint> points;
};
struct GeoposArgs {
    std::string_view key;
    std::vector<std::string_view> members;
};
struct GeodistArgs {
    std::string_view key;
    std::string_view member1;
    std::string_view member2;
    std::string_view unit;
};
struct GeosearchArgs {
    std::string_view key;
    uint8_t coord;
    double longitude;
    double latitude;
    uint8_t type_zone;
    double radius;
    std::string_view unit;
    bool asc;
    std::optional<int64_t> count;
};
struct GeosearchstoreArgs {
    std::string_view destination;
    std::string_view source;
    uint8_t coord;
    double longitude;
    double latitude;
     uint8_t type_zone;
    double radius;
    std::string_view unit;
    bool asc;
    std::optional<int64_t> count;
};

//
struct TypeArgs {
    std::string_view key;
};
struct DelArgs {
    std::vector<std::string_view> keys;
};
struct ExistsArgs {
    std::vector<std::string_view> keys;
};
struct KeysArgs {
    std::string_view pattern;
};
struct FlushdbArgs {};
struct ConfigSetArgs {
    std::string_view parameter;
    int64_t value;
};
struct ConfigGetArgs {
    std::string_view parameter;
};
struct DbsizeArgs {};
struct MemoryUsageArgs {
    std::string_view key;
};

using Command = std::variant<TypeArgs,
                             DelArgs,
                             ExistsArgs,
                             KeysArgs,
                             FlushdbArgs,
                             ConfigSetArgs,
                             ConfigGetArgs,
                             DbsizeArgs,
                             MemoryUsageArgs,
                             GeoaddArgs,
                             GeoposArgs,
                             GeodistArgs,
                             GeosearchArgs,
                             GeosearchstoreArgs,
                             SaddArgs,
                             SremArgs,
                             SismemberArgs,
                             SmembersArgs,
                             ScardArgs,
                             SunionArgs,
                             SinterArgs,
                             SdiffArgs,
                             SmoveArgs,
                             LpushArgs,
                             RpushArgs,
                             LpopArgs,
                             RpopArgs,
                             LlenArgs,
                             LrangeArgs,
                             LindexArgs,
                             LsetArgs,
                             LinsertArgs,
                             SetArgs,
                             GetArgs,
                             StrlenArgs,
                             AppendArgs,
                             ExpireArgs,
                             TtlArgs>;

} // namespace daikon::commands
