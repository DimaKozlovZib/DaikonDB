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
            return "(error) WRONGTYPE Operation against a key holding the wrong kind of value";
        case LogicError::kNotFound:
            return "(error) ERR no such key";
        case LogicError::kOom:
            return "(error) OOM command not allowed when used memory > 'maxmemory'";
        default:
            return "(error) ERR unknown error";
    }
}

template <typename T>
void WriteResult(const T& value, std::ostream& os, const std::string& success_prefix = "") {
    if constexpr (std::is_same_v<T, results::StatusResult>) {
        os << (value.success ? "OK" : "(error) ERR operation failed");
    } else if constexpr (std::is_same_v<T, results::ViewResult>) {
        os << "\"" << value.data << "\"";
    } else if constexpr (std::is_same_v<T, results::IntResult>) {
        os << "(integer) " << value.value;
    } else if constexpr (std::is_same_v<T, results::ListViewResult>) {
        const auto& vec = value.elements;
        if (vec.empty()) {
            os << "(empty array)";
        } else {
            for (size_t i = 0; i < vec.size(); ++i) {
                if (i > 0) os << "\n";
                os << (i + 1) << ") \"" << vec[i] << "\"";
            }
        }
    } else if constexpr (std::is_same_v<T, std::vector<std::optional<std::pair<double, double>>>>) {
        if (value.empty()) {
            os << "(empty array)";
        } else {
            for (size_t i = 0; i < value.size(); ++i) {
                if (i > 0) os << "\n";
                os << (i + 1) << ") ";
                
                const auto& opt = value[i];

                if (opt)
                    os << "1) " << opt->first << "\n   2) " << opt->second;
                else
                    os << "(nil)";
            }
        }
    } else if constexpr (std::is_floating_point_v<T>) {
        os << "\"" << value << "\""; 
    } else {
        os << success_prefix << value;
    }
}

template <typename T>
void WriteResult(const DbResult<T>& result, std::ostream& os, const std::string& success_prefix = "") {
    if (result) {
        if constexpr (std::is_same_v<T, void>) {
            os << "OK";
        } else {
            WriteResult(*result, os, success_prefix);
        }
    } else {
        os << LogicErrorToString(result.error());
    }
}

} // namespace daikon::detail