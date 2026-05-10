#pragma once

#include <variant>
#include <tuple>
#include <type_traits>
#include "Types.h" 

namespace daikon::commands::meta {

template <typename T>
struct CommandGroupTraits;

template <typename... Ts>
struct CommandGroupTraits<std::variant<Ts...>> {
    using types = std::tuple<Ts...>;
    static constexpr std::size_t size = sizeof...(Ts);
};

template <std::size_t I, typename Group, typename ArgStruct, typename = void>
struct GroupContainsArgument : std::false_type {};

template <std::size_t I, typename Group, typename ArgStruct>
struct GroupContainsArgument<I, Group, ArgStruct, std::enable_if_t<(I < CommandGroupTraits<Group>::size)>> {
    using current_type = std::tuple_element_t<I, typename CommandGroupTraits<Group>::types>;

    static constexpr bool value = std::is_same_v<current_type, ArgStruct> ||
                                 GroupContainsArgument<I + 1, Group, ArgStruct>::value;
};

template <std::size_t I, typename Group, typename ArgStruct>
inline constexpr bool GroupContainsArgument_v = GroupContainsArgument<I, Group, ArgStruct>::value;

template <std::size_t I, std::size_t Max, typename AllCommands, typename ArgStruct>
struct FindCommandCategory {
    using current_category = std::tuple_element_t<I, typename CommandGroupTraits<AllCommands>::types>;

    using type = std::conditional_t<
        GroupContainsArgument_v<0, current_category, ArgStruct>,
        current_category,
        typename FindCommandCategory<I + 1, Max, AllCommands, ArgStruct>::type
    >;
};

template <std::size_t N, typename AllCommands, typename ArgStruct>
struct FindCommandCategory<N, N, AllCommands, ArgStruct> {
    using type = void;
};

template <typename AllCommands, typename ArgStruct>
struct ResolveCommandGroup {
    using type = typename FindCommandCategory<
        0, 
        CommandGroupTraits<AllCommands>::size, 
        AllCommands, 
        ArgStruct
    >::type;
};

template <typename AllCommands, typename ArgStruct>
using ResolveCommandGroup_t = typename ResolveCommandGroup<AllCommands, ArgStruct>::type;

} // namespace daikon::commands::meta