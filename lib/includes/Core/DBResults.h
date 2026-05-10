
#pragma once
#include <expected>
#include <optional>
#include <string_view>
#include <vector>

namespace daikon {

enum class LogicError {
    kWrongType,
    kSyntaxError,
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

    ViewResult(std::string_view st_) :data(st_) {}
    ViewResult(const std::string& st_) :data(st_) {}
    ViewResult(const char* st_) : data(st_) {}
};

struct ListViewResult {
    std::vector<std::string> elements;
};

struct IntResult {
    int64_t value = 0;

    IntResult(int num) :value(num) {}
    IntResult(size_t num) :value(num) {}
    IntResult(int64_t num) :value(num) {}
};

struct StatusResult {
    bool success = false;
};

} // namespace daikon::results