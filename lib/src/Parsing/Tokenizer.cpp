#include "../../includes/parsing/Parsing.h"

#include <vector>      
#include <string_view>
#include <cstddef>     
#include <cctype>  
#include <utility>  

namespace daikon::parsing {

std::optional<std::vector<Tokenizer::RawCommand>> Tokenizer::GetTokens(std::string_view input) {
    std::vector<Tokenizer::RawCommand> commands;
    Tokenizer::RawCommand current_cmd;
    
    bool in_quotes = false;
    size_t token_start = 0;
    bool inside_token = false;

    for (size_t i = 0; i <= input.size(); ++i) {
        char c = (i < input.size()) ? input[i] : '\0';

        if (!in_quotes && (c == ';' || c == '\0' || std::isspace(static_cast<unsigned char>(c)))) {
            if (inside_token) {
                std::string_view token = input.substr(token_start, i - token_start);
                current_cmd.tokens.push_back(token);
                inside_token = false;
            }

            if (c == ';' || c == '\0') {
                if (!current_cmd.tokens.empty()) {
                    commands.push_back(std::move(current_cmd));
                    current_cmd = {};
                }
            }
            continue;
        }

        if (c == '"') {
            if (in_quotes) {
                std::string_view token = input.substr(token_start, i - token_start);
                current_cmd.tokens.push_back(token);
                in_quotes = false;
                inside_token = false;
            } else {
                if (inside_token) {
                    std::string_view token = input.substr(token_start, i - token_start);
                    if (!token.empty()) {
                        current_cmd.tokens.push_back(token);
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
        return std::nullopt;
    }

    return commands;
}

} // namespace daikon::parsing