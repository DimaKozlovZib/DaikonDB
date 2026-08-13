#include "../../includes/parsing/Parsing.h"

#include <vector>
#include <string_view>
#include <cstddef>
#include <cctype>
#include <utility>
#include <span>

namespace daikon::parsing {

void Tokenizer::Clear() {
    tokens_storage_.clear();
    commands_storage_.clear();
    command_boundaries_.clear();
}

std::optional<std::span<const Tokenizer::RawCommand>> Tokenizer::GetTokens(std::string_view input) {
    Clear();
    
    bool in_quotes = false;
    size_t token_start = 0;
    bool inside_token = false;

    size_t current_cmd_start = 0;

    for (size_t i = 0; i <= input.size(); ++i) {
        char c = (i < input.size()) ? input[i] : '\0';

        if (!in_quotes && (c == ';' || c == '\0' || std::isspace(static_cast<unsigned char>(c)))) {
            if (inside_token) {
                std::string_view token = input.substr(token_start, i - token_start);
                tokens_storage_.push_back(token);
                inside_token = false;
            }

            if (c == ';' || c == '\0') {
                if (current_cmd_start < tokens_storage_.size()) {
                    command_boundaries_.push_back({current_cmd_start, tokens_storage_.size() - current_cmd_start});
                    current_cmd_start = tokens_storage_.size();
                }
            }
            continue;
        }

        if (c == '"') {
            if (in_quotes) {
                std::string_view token = input.substr(token_start, i - token_start);
                tokens_storage_.push_back(token);
                in_quotes = false;
                inside_token = false;
            } else {
                if (inside_token) {
                    std::string_view token = input.substr(token_start, i - token_start);
                    if (!token.empty()) {
                        tokens_storage_.push_back(token);
                    }
                }
                token_start = i + 1;
                in_quotes = true;
                inside_token = true;
            }
            continue;
        }

        if (!inside_token) {
            token_start = i;
            inside_token = true;
        }
    }

    if (in_quotes) {
        Clear();
        return std::nullopt;
    }

    commands_storage_.reserve(command_boundaries_.size());
    for (const auto& [start_idx, count] : command_boundaries_) {
        commands_storage_.push_back(RawCommand{
            std::span<const std::string_view>(tokens_storage_.data() + start_idx, count)
        });
    }

    if (commands_storage_.empty()) {
        return std::span<const RawCommand>();
    }

    return std::span<const RawCommand>(commands_storage_);
}

} // namespace daikon::parsing