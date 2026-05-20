#pragma once

#include <charconv>
#include <concepts>
#include <optional>
#include <span>
#include <string>
#include <unordered_set>

#include "../Commands.h"
#include "CommandNames.h"
#include "Types.h"

namespace daikon::parsing {

template <typename T>
concept IsVarArgsTag = requires {
    typename T::IsVarArgsTag;
};

class CommandParser {
   public:
    std::optional<commands::Command> Dispatch(std::string_view cmd,
                                              std::span<const std::string_view> args);

   private:
    template <typename Command, typename... Tags>
        requires(traits::ParsableArg<traits::ArgTraits<Tags>> && ...)
    std::optional<commands::Command> TryParse(std::span<const std::string_view> args) {
        size_t idx = 0;

        std::tuple<std::optional<typename traits::ArgTraits<Tags>::value_type>...> maybe_vals{
            traits::ArgTraits<Tags>::Extract(args, idx)...};

        bool all_ok = std::apply([](auto&... args) {
            return (args.has_value() && ...);
        },
                                 maybe_vals);

        constexpr bool kHasVarargs = (IsVarArgsTag<Tags> || ...);
        if (!kHasVarargs && idx != args.size()) all_ok = false;

        if (!all_ok) return std::nullopt;

        return std::apply([](auto&&... opts) -> Command {
            return Command{std::move(*opts)...};
        },
                          std::move(maybe_vals));
    }
};

class Tokenizer {
   public:
    struct RawCommand {
        std::vector<std::string_view> tokens;
    };

    static std::optional<std::vector<RawCommand>> GetTokens(std::string_view input);
};

} // namespace daikon::parsing