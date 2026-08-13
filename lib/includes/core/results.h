#pragma once

#include <concepts>
#include <expected>
#include <optional>
#include <string_view>
#include <vector>

namespace daikon {

enum class LogicError {
    kWrongType,
    kNotFound,
    kNone,
    kOom
};

template <typename T>
using DbResult = std::expected<T, LogicError>;

} // namespace daikon

namespace daikon::results {

struct ViewResult {
    std::string data;

    ViewResult(std::string_view st_) : data(st_) {}
    ViewResult(const std::string& st_) : data(st_) {}
    ViewResult(std::string&& st_) noexcept : data(std::move(st_)) {}
    ViewResult(const char* st_) : data(st_) {}
    ViewResult(std::nullptr_t) = delete;
};

struct ListViewResult {
    std::vector<std::string> elements;

    ListViewResult() = default;
    ListViewResult(const std::vector<std::string>& vec) : elements(vec) {}
    ListViewResult(std::vector<std::string>&& vec) noexcept : elements(std::move(vec)) {}
    ListViewResult(std::initializer_list<std::string> list) : elements(list) {}
};

struct IntResult {
    int64_t value = 0;

    template <std::integral N>
    IntResult(N num) : value(static_cast<int64_t>(num)) {}
};

struct StatusResult {
    bool success = false;

    StatusResult(bool v) : success(v) {}
};

} // namespace daikon::results