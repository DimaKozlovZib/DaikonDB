#include "../Commands.h"

namespace daikon::commands::meta {

template <typename T> struct IsWriteOp : std::false_type {};
template <typename T> struct IsReadOp : std::false_type {};

template <> struct IsWriteOp<commands::SetArgs> : std::true_type {};
template <> struct IsWriteOp<commands::DelArgs> : std::true_type {};
template <> struct IsWriteOp<commands::FlushdbArgs> : std::true_type {};
template <> struct IsWriteOp<commands::AppendArgs> : std::true_type {};

template <> struct IsReadOp<commands::GetArgs> : std::true_type {};
template <> struct IsReadOp<commands::LrangeArgs> : std::true_type {};
template <> struct IsReadOp<commands::SmembersArgs> : std::true_type {};

template <typename T>
std::vector<std::string_view> GetAffectedKeys(const T& args) {
    if constexpr (requires { args.key; }) {
        return {args.key};
    } else if constexpr (requires { args.keys; }) {
        return {args.keys.begin(), args.keys.end()};
    }
    return {};
}

} // namespace daikon::commands::meta