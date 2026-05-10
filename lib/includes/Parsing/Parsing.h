#pragma once

#include <charconv>
#include <optional>
#include <span>
#include <string>
#include <type_traits>
#include <unordered_set>

#include "../Commands.h"
#include "CommandNames.h"
#include "MetaParse.h"
#include "Types.h"

namespace daikon::parsing {

template <typename T>
struct is_varargs : std::false_type {};

template <typename ElemTag>
struct is_varargs<::daikon::parsing::args::VarArgs<ElemTag>> : std::true_type {};

template <typename T>
inline constexpr bool is_varargs_v = is_varargs<T>::value;

enum class PipelineCategory {
    kSafe,
    kMixed,
};

class CommandParser {
   private:
    template <typename Command, typename... Tags>
    std::optional<commands::Command> TryParse(std::span<const std::string_view> args) {
        size_t idx = 0;

        auto maybe_vals = std::make_tuple(traits::ArgTraits<Tags>::extract(args, idx)...);

        bool all_ok = std::apply([](auto&... args) {
            return (args.has_value() && ...);
        },
                                 maybe_vals);

        constexpr bool has_varargs = (is_varargs_v<Tags> || ...);
        if (!has_varargs && idx != args.size()) all_ok = false;

        if (!all_ok) return std::nullopt;

        return std::apply([](auto&&... opts) -> Command {
            return Command{std::move(*opts)...};
        },
                          std::move(maybe_vals));
    }

    // template <typename CommandType, typename... Tags>
    // std::optional<commands::Command> TryParseComSeg(std::span<const std::string_view> args) {
    //     auto result = TryParse<CommandType, Tags...>(args);
    //     if (result) {
    //         return commands::Command{::daikon::commands::meta::ResolveCommandGroup_t<commands::Command, CommandType>{std::move(*result)}};
    //     }
    //     return std::nullopt;
    // }

    bool IsEquals(std::string_view a, std::string_view b);

    std::optional<commands::Command> Dispatch(std::string_view cmd,
                                              std::span<const std::string_view> args);

    // PipelineCategory AnalyzePipeline(const std::vector<commands::Command>& pipeline) {
    //     std::unordered_set<std::string_view> read_keys;
    //     bool global_read_active = false;

    //     for (const auto& cmd : pipeline) {
    //         auto category = std::visit([&](auto&& args) -> PipelineCategory {
    //             using T = std::decay_t<decltype(args)>;

    //             bool is_write = commands::meta::IsWriteOp<T>::value;
    //             bool is_read = commands::meta::IsReadOp<T>::value;
    //             auto current_keys = commands::meta::GetAffectedKeys(args);

    //             if (is_write) {
    //                 if constexpr (std::is_same_v<T, commands::FlushdbArgs>) {
    //                     if (!read_keys.empty() || global_read_active) return PipelineCategory::kMixed;
    //                 }

    //                 for (auto k : current_keys) {
    //                     if (read_keys.contains(k)) return PipelineCategory::kMixed;
    //                 }
    //             }

    //             if (is_read) {
    //                 for (auto k : current_keys) {
    //                     read_keys.insert(k);
    //                 }
    //             }
    //             return PipelineCategory::kSafe;
    //         },
    //                                    cmd);

    //         if (category == PipelineCategory::kMixed) return PipelineCategory::kMixed;
    //     }

    //     return PipelineCategory::kSafe;
    // }
};

class Tokenizer {
   public:
    struct RawCommand {
        std::vector<std::string_view> tokens;
    };

    static std::vector<RawCommand> GetTokens(std::string_view input);
};
} // namespace daikon::parsing