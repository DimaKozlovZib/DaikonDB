#pragma once

#include <charconv>
#include <optional>
#include <span>
#include <string>
#include <type_traits>
#include <unordered_set>

#include "../Commands.h"
#include "CommandNames.h"
#include "Types.h"

namespace daikon::parsing {

template <typename T>
struct IsVarArgs : std::false_type {};

template <typename ElemTag>
struct IsVarArgs<::daikon::parsing::args::VarArgs<ElemTag>> : std::true_type {};

template <typename T>
inline constexpr bool IsVarArgs_v = IsVarArgs<T>::value;

enum class PipelineCategory {
    kSafe,
    kMixed,
};

class CommandParser {
   public:
    std::optional<commands::Command> Dispatch(std::string_view cmd,
                                              std::span<const std::string_view> args);

   private:
    template <typename Command, typename... Tags>
    std::optional<commands::Command> TryParse(std::span<const std::string_view> args) {
        size_t idx = 0;

        auto maybe_vals = std::make_tuple(traits::ArgTraits<Tags>::Extract(args, idx)...);

        bool all_ok = std::apply([](auto&... args) {
            return (args.has_value() && ...);
        },
                                 maybe_vals);

        constexpr bool kHasVarargs = (IsVarArgs_v<Tags> || ...);
        if (!kHasVarargs && idx != args.size()) all_ok = false;

        if (!all_ok) return std::nullopt;

        return std::apply([](auto&&... opts) -> Command {
            return Command{std::move(*opts)...};
        },
                          std::move(maybe_vals));
    }

    bool IsEquals(std::string_view a, std::string_view b);
};

class Tokenizer {
   public:
    struct RawCommand {
        std::vector<std::string_view> tokens;
    };

    static std::vector<RawCommand> GetTokens(std::string_view input);
};
} // namespace daikon::parsing