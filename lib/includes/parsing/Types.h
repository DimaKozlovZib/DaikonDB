#pragma once

#include <charconv>
#include <concepts>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include "../Commands.h"
#include "CommandNames.h"

namespace daikon::parsing::args {

struct Key {};
struct Value {};
struct Int64 {};
struct Double {};
struct Unit {
    static constexpr std::string_view kDefaultValue = "m";
};
struct BeforeAfter {};
struct OptionalInt64 {};

template <typename ElemTag>
struct VarArgs {
    using IsVarArgsTag = void;
    using ElementTag = ElemTag;
};

struct GeoPoint {};
struct AscDesc {};
struct FromLonLat {};
struct ByRadius {};
struct GeoCount {};

} // namespace daikon::parsing::args

namespace daikon::parsing::traits {

using dci = daikon::commands::names::CommandInfo;

template <typename T>
concept ParsableArg = requires(std::span<const std::string_view> args, size_t& idx) {
    typename T::value_type;
    { T::Extract(args, idx) } -> std::same_as<std::optional<typename T::value_type>>;
};

template <typename Tag>
struct ArgTraits;

template <>
struct ArgTraits<args::Key> {
    using value_type = std::string_view;
    static std::optional<value_type> Extract(std::span<const std::string_view> args, size_t& idx) {
        if (idx >= args.size()) return std::nullopt;
        return args[idx++];
    }
};

template <>
struct ArgTraits<args::Value> {
    using value_type = std::string_view;
    static std::optional<value_type> Extract(std::span<const std::string_view> args, size_t& idx) {
        if (idx >= args.size()) return std::nullopt;
        return args[idx++];
    }
};

template <>
struct ArgTraits<args::Int64> {
    using value_type = int64_t;
    static std::optional<value_type> Extract(std::span<const std::string_view> args, size_t& idx) {
        if (idx >= args.size()) return std::nullopt;
        int64_t v;
        auto [ptr, ec] = std::from_chars(args[idx].data(), args[idx].data() + args[idx].size(), v);
        if (ec != std::errc{}) return std::nullopt;
        ++idx;
        return value_type{v};
    }
};

template <>
struct ArgTraits<args::Double> {
    using value_type = double;
    static std::optional<value_type> Extract(std::span<const std::string_view> args, size_t& idx) {
        if (idx >= args.size()) return std::nullopt;
        double v;
        auto [ptr, ec] = std::from_chars(args[idx].data(), args[idx].data() + args[idx].size(), v);
        if (ec != std::errc{}) return std::nullopt;
        ++idx;
        return v;
    }
};

template <>
struct ArgTraits<args::Unit> {
    using value_type = std::string_view;
    static std::optional<value_type> Extract(std::span<const std::string_view> args, size_t& idx) {
        if (idx >= args.size()) {
            return args::Unit::kDefaultValue;
        }
        std::string s;
        for (char c : args[idx])
            s += std::tolower(static_cast<unsigned char>(c));
        if (s == "m" || s == "km" || s == "mi" || s == "ft") {
            return args[idx++];
        }
        return args::Unit::kDefaultValue;
    }
};

template <>
struct ArgTraits<args::AscDesc> {
    using value_type = bool;
    static std::optional<value_type> Extract(std::span<const std::string_view> args, size_t& idx) {
        if (idx >= args.size()) return true;
        auto s = args[idx++];

        if (dci::IsEquals(s, "ASC")) return true;
        if (dci::IsEquals(s, "DESC")) return false;

        return std::nullopt;
    }
};

template <>
struct ArgTraits<args::BeforeAfter> {
    using value_type = bool;
    static std::optional<value_type> Extract(std::span<const std::string_view> args, size_t& idx) {
        if (idx >= args.size()) return std::nullopt;
        auto s = args[idx++];
        if (dci::IsEquals(s, "BEFORE")) return true;
        if (dci::IsEquals(s, "AFTER")) return false;
        return std::nullopt;
    }
};

template <>
struct ArgTraits<args::OptionalInt64> {
    using value_type = int64_t;
    static std::optional<value_type> Extract(std::span<const std::string_view> args, size_t& idx) {
        if (idx >= args.size()) return -1;
        int64_t v;
        auto [ptr, ec] = std::from_chars(args[idx].data(), args[idx].data() + args[idx].size(), v);
        if (ec != std::errc{}) return std::nullopt;
        ++idx;
        return v;
    }
};

template <>
struct ArgTraits<args::GeoPoint> {
    using value_type = commands::GeoPoint;
    static std::optional<value_type> Extract(std::span<const std::string_view> args, size_t& idx) {
        if (idx + 2 >= args.size()) return std::nullopt;
        double lon, lat;
        if (auto r1 = ArgTraits<args::Double>::Extract(args, idx); !r1)
            return std::nullopt;
        else
            lon = *r1;
        if (auto r2 = ArgTraits<args::Double>::Extract(args, idx); !r2)
            return std::nullopt;
        else
            lat = *r2;
        auto member = args[idx++];
        return commands::GeoPoint{lon, lat, member};
    }
};

template <typename ElemTag>
struct ArgTraits<args::VarArgs<ElemTag>> {
    using value_type = std::vector<typename ArgTraits<ElemTag>::value_type>;

    static std::optional<value_type> Extract(std::span<const std::string_view> args, size_t& idx) {
        value_type res;
        while (idx < args.size()) {
            auto elem = ArgTraits<ElemTag>::Extract(args, idx);
            if (!elem) return std::nullopt;
            res.push_back(std::move(*elem));
        }
        return res;
    }
};

template <>
struct ArgTraits<args::FromLonLat> {
    using value_type = uint8_t;
    static std::optional<value_type> Extract(std::span<const std::string_view> args, size_t& idx) {
        if (idx >= args.size()) return std::nullopt;
        if (dci::IsEquals(args[idx], "FROMLONLAT")) {
            ++idx;
            return 1;
        }
        return std::nullopt;
    }
};

template <>
struct ArgTraits<args::ByRadius> {
    using value_type = uint8_t;
    static std::optional<value_type> Extract(std::span<const std::string_view> args, size_t& idx) {
        if (idx >= args.size()) return std::nullopt;
        if (dci::IsEquals(args[idx], "BYRADIUS")) {
            ++idx;
            return 1;
        }
        return std::nullopt;
    }
};

template <>
struct ArgTraits<args::GeoCount> {
    using value_type = int64_t;
    static std::optional<value_type> Extract(std::span<const std::string_view> args, size_t& idx) {
        if (idx >= args.size()) return -1;

        if (dci::IsEquals(args[idx], "COUNT")) {
            ++idx;
            if (idx >= args.size()) return std::nullopt;

            auto val = ArgTraits<args::Int64>::Extract(args, idx);
            if (!val) return std::nullopt;
            return *val;
        }
        return -1;
    }
};

} // namespace daikon::parsing::traits