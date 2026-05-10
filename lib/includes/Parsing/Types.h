#pragma once

#include <charconv>
#include <optional>
#include <span>
#include <string>

#include "../Commands.h"

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
struct GeoPoint {};

template <typename ElemTag>
struct VarArgs {};

} // namespace daikon::parsing::args

namespace daikon::parsing::traits{

template <typename Tag>
struct ArgTraits;

template <>
struct ArgTraits<args::Key> {
    using value_type = std::string_view;
    static std::optional<value_type> extract(std::span<const std::string_view> args, size_t& idx) {
        if (idx >= args.size()) return std::nullopt;
        return args[idx++];
    }
};

template <>
struct ArgTraits<args::Value> {
    using value_type = std::string_view;
    static std::optional<value_type> extract(std::span<const std::string_view> args, size_t& idx) {
        if (idx >= args.size()) return std::nullopt;
        return args[idx++];
    }
};

template <>
struct ArgTraits<args::Int64> {
    using value_type = int64_t;
    static std::optional<value_type> extract(std::span<const std::string_view> args, size_t& idx) {
        if (idx >= args.size()) return std::nullopt;
        int64_t v;
        auto [ptr, ec] = std::from_chars(args[idx].data(), args[idx].data() + args[idx].size(), v);
        if (ec != std::errc{}) return std::nullopt;
        ++idx;
        return v;
    }
};

template <>
struct ArgTraits<args::Double> {
    using value_type = double;
    static std::optional<value_type> extract(std::span<const std::string_view> args, size_t& idx) {
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
    static std::optional<value_type> extract(std::span<const std::string_view> args, size_t& idx) {
        if (idx >= args.size()) return args::Unit::kDefaultValue; 
        return args[idx++];
    }
};

template <>
struct ArgTraits<args::BeforeAfter> {
    using value_type = bool;
    static std::optional<value_type> extract(std::span<const std::string_view> args, size_t& idx) {
        if (idx >= args.size()) return std::nullopt;
        auto s = args[idx++];
        if (s == "BEFORE") return true;
        if (s == "AFTER") return false;
        return std::nullopt;
    }
};

template <>
struct ArgTraits<args::OptionalInt64> {
    using value_type = std::optional<int64_t>;
    static std::optional<value_type> extract(std::span<const std::string_view> args, size_t& idx) {
        if (idx >= args.size()) return value_type{};
        int64_t v;
        auto [ptr, ec] = std::from_chars(args[idx].data(), args[idx].data() + args[idx].size(), v);
        if (ec != std::errc{}) return std::nullopt;
        ++idx;
        return value_type{v};
    }
};

template <>
struct ArgTraits<args::GeoPoint> {
    using value_type = commands::GeoPoint;
    static std::optional<value_type> extract(std::span<const std::string_view> args, size_t& idx) {
        if (idx + 2 >= args.size()) return std::nullopt;
        double lon, lat;
        if (auto r1 = ArgTraits<args::Double>::extract(args, idx); !r1)
            return std::nullopt;
        else
            lon = *r1;
        if (auto r2 = ArgTraits<args::Double>::extract(args, idx); !r2)
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

    static std::optional<value_type> extract(std::span<const std::string_view> args, size_t& idx) {
        value_type vec;
        while (idx < args.size()) {
            auto elem = ArgTraits<ElemTag>::extract(args, idx);
            if (!elem) return std::nullopt;
            vec.push_back(std::move(*elem));
        }
        return vec;
    }
};

} // namespace daikon::parsing::traits