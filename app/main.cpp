#include <cctype>
#include <charconv>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>

#include "../lib/includes/clients/StreamClient.h"

bool EqualIgnoreCase(std::string_view a, std::string_view b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i) {
        if (std::tolower(static_cast<unsigned char>(a[i])) != std::tolower(static_cast<unsigned char>(b[i]))) {
            return false;
        }
    }
    return true;
}

std::optional<size_t> ParseMaxMemory(std::string_view arg) {
    if (arg.empty()) return std::nullopt;

    size_t num_end = 0;
    while (num_end < arg.size() && (std::isdigit(arg[num_end]) || arg[num_end] == '.')) {
        num_end++;
    }

    if (num_end == 0) return std::nullopt;

    double value = 0;
    auto [ptr, ec] = std::from_chars(arg.data(), arg.data() + num_end, value);
    if (ec != std::errc{}) {
        return std::nullopt;
    }

    std::string_view suffix = arg.substr(num_end);

    size_t multiplier = 1;
    if (suffix.empty() || EqualIgnoreCase(suffix, "b")) {
        multiplier = 1;
    } else if (EqualIgnoreCase(suffix, "kb")) {
        multiplier = 1024ULL;
    } else if (EqualIgnoreCase(suffix, "mb")) {
        multiplier = 1024ULL * 1024ULL;
    } else if (EqualIgnoreCase(suffix, "gb")) {
        multiplier = 1024ULL * 1024ULL * 1024ULL;
    } else {
        return std::nullopt;
    }

    return static_cast<size_t>(value * multiplier);
}

int main(int argc, char* argv[]) {
    size_t maxmemory = 0;

    if (argc > 1) {
        if (argc != 3 || std::string_view(argv[1]) != "--maxmemory") {
            std::cerr << "Usage: " << argv[0] << " [--maxmemory <bytes>]\n";
            return 1;
        }
        std::string_view val_arg = argv[2];
        auto parsed_bytes = ParseMaxMemory(val_arg);

        if (!parsed_bytes) {
            std::cerr << "Error: Invalid maxmemory value '" << val_arg << "'\n";
            return 1;
        }
        maxmemory = *parsed_bytes;
    }

    try {
        daikon::StreamClient client(maxmemory);
        client.ListenStream(std::cin, std::cout);
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}