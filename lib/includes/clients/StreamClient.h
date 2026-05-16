#pragma once

#include <stdio.h>

#include <functional>
#include <iostream>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "../Commands.h"
#include "../core/DaikonDB.h"
#include "../parsing/CommandNames.h"
#include "../parsing/Parsing.h"
#include "../parsing/Types.h"
#include "details/handlers.h"
#include "details/output.h"

namespace daikon {

class StreamClient : public DaikonDatabase {
   public:
    explicit StreamClient(size_t maxmemory = 0)
        : DaikonDatabase(maxmemory) {}

    void ListenStream(std::istream& in = std::cin,
                      std::ostream& out = std::cout) {
        std::string line;
        while (std::getline(in, line)) {
            if (line.empty()) continue;
            if (IsExitCommand(line)) break;
            ExecuteCommand(line, out);
        }
    }

   private:
    void ExecuteCommand(const std::string& line, std::ostream& out) {
        auto raw_cmds = parsing::Tokenizer::GetTokens(line);
        if (raw_cmds.empty()) return;

        for (size_t i = 0; i < raw_cmds.size(); ++i) {
            const auto& tokens = raw_cmds[i].tokens;
            if (tokens.empty()) continue;

            std::span<const std::string_view> args_view(tokens.begin() + 1, tokens.end());
            auto parsed_cmd = parser_.Dispatch(tokens[0], args_view);

            if (!parsed_cmd) {
                std::cerr << "ERR unknown command or invalid arguments: " << tokens[0] << std::endl;
                continue;
            }

            std::visit([&out, this](const auto& args) {
                handlers::Handle(*this, args, out);
            },
                       *parsed_cmd);

            out << std::endl;
        }
    }

    static void NormalizeString(std::string& s) {
        for (auto& c : s)
            c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }

    static bool IsExitCommand(const std::string& s) {
        std::string upper = s;
        NormalizeString(upper);
        return upper == "EXIT";
    }

    parsing::CommandParser parser_;
};

} // namespace daikon