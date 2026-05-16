#pragma once

#include <functional>
#include <iostream>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "../../Commands.h"
#include "../../core/DaikonDB.h"
#include "../../parsing/CommandNames.h"
#include "../../parsing/Parsing.h"
#include "../../parsing/Types.h"

namespace daikon::detail {

inline std::string_view LogicErrorToString(LogicError error) {
    switch (error) {
        case LogicError::kWrongType:
            return "WRONGTYPE Operation against a key holding the wrong kind of value";
        case LogicError::kNotFound:
            return "ERR no such key";
        case LogicError::kOom:
            return "(error) OOM command not allowed when used memory > 'maxmemory'";
        case LogicError::kSyntaxError:
            return "ERR syntax error";
        default:
            return "ERR unknown error";
    }
}

template <typename T>
void WriteResult(const T& result, std::ostream& os, const std::string& success_prefix = "") {
    if constexpr (std::is_same_v<T, results::StatusResult>) {
        os << (result->success ? "OK" : "ERR operation failed");
    } else if constexpr (std::is_same_v<T, results::ViewResult>) {
        os << result->data;
    } else if constexpr (std::is_same_v<T, results::IntResult>) {
        os << result->value;
    } else if constexpr (std::is_same_v<T, results::ListViewResult>) {
        const auto& vec = result->elements;
        if (vec.empty()) {
            os << "(empty array)";
        } else {
            for (size_t i = 0; i < vec.size(); ++i) {
                if (i) os << " ";
                os << vec[i];
            }
        }
    } else if constexpr (std::is_same_v<T, std::vector<std::optional<std::pair<double, double>>>>) {
        for (size_t i = 0; i < result.size(); ++i) {
            if (i) os << ' ';
            const auto& opt = result[i];
            if (opt)
                os << "(" << opt->first << " " << opt->second << ")";
            else
                os << "(nil)";
        }
    } else if constexpr (std::is_floating_point_v<T>) {
        os << result;
    } else if constexpr (std::is_same_v<T, void>) {
        os << "OK";
    } else {
        os << success_prefix << result;
    }
}

template <typename T>
void WriteResult(const DbResult<T>& result, std::ostream& os, const std::string& success_prefix = "") {
    if (result)
        WriteResult(result.value(), os, success_prefix);
    else
        os << LogicErrorToString(result.error());
}

} // namespace daikon::detail