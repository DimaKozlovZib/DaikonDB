#include "../../includes/Parsing/Parsing.h"

#include <charconv>
#include <optional>
#include <span>
#include <string>
#include <type_traits>

namespace daikon::parsing {

    std::vector<Tokenizer::RawCommand> Tokenizer::GetTokens(std::string_view input) {
        std::vector<Tokenizer::RawCommand> commands;
        Tokenizer::RawCommand current_cmd;
        bool in_quotes = false;
        size_t token_start = 0;
        bool inside_token = false;

        for (size_t i = 0; i <= input.size(); ++i) {
            char c = (i < input.size()) ? input[i] : '\0';

            if (c == '"') {
                if (in_quotes) {
                    std::string_view token = input.substr(token_start, i - token_start + 1);
                    token.remove_prefix(1);
                    if (!token.empty()) token.remove_suffix(1);

                    current_cmd.tokens.push_back(token);
                    inside_token = false;
                    in_quotes = false;
                } else {
                    token_start = i;
                    inside_token = true;
                    in_quotes = true;
                }
                continue;
            }

            if (in_quotes) {
                continue;
            }

            if (std::isspace(static_cast<unsigned char>(c)) || c == ';' || c == '\0') {
                if (inside_token) {
                    std::string_view token = input.substr(token_start, i - token_start);
                    if (!token.empty()) {
                        current_cmd.tokens.push_back(token);
                    }
                    inside_token = false;
                }

                if (c == ';' || c == '\0') {
                    if (!current_cmd.tokens.empty()) {
                        commands.push_back(std::move(current_cmd));
                        current_cmd = {};
                    }
                }
            } else {
                if (!inside_token) {
                    token_start = i;
                    inside_token = true;
                }
            }
        }

        return commands;
    }

} // namespace daikon::parsing