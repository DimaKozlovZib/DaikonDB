#include <chrono>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "../../includes/core/results.h"
#include "core/DaikonDB.h"

namespace daikon {

results::ViewResult DaikonDatabase::Type(std::string_view key) {
    auto* obj = db_.Get(key);
    if (!obj) return results::ViewResult{"none"};
    return core::types::DataTypeToString(obj->GetType());
}

results::IntResult DaikonDatabase::Del(const std::vector<std::string_view>& keys) {
    if (has_active_del_) db_.ExpireCycle(3);

    int64_t deleted = 0;
    for (const auto& key : keys) {
        if (db_.Delete(key)) ++deleted;
    }
    return results::IntResult{deleted};
}

results::IntResult DaikonDatabase::Exists(const std::vector<std::string_view>& keys) {
    int64_t count = 0;
    for (const auto& key : keys) {
        if (db_.Get(key)) ++count;
    }
    return results::IntResult{count};
}

results::StatusResult DaikonDatabase::ConfigSetMaxmemory(int64_t bytes) {
    if (bytes < 0) return results::StatusResult{false};
    maxmemory_ = static_cast<size_t>(bytes);
    return results::StatusResult{true};
}

results::IntResult DaikonDatabase::ConfigGetMaxmemory() {
    return maxmemory_;
}

results::IntResult DaikonDatabase::Dbsize() {
    return db_.Size();
}

void DaikonDatabase::Flushdb() {
    db_.Clear();
}

bool MatchGlobPattern(std::string_view pattern, std::string_view text) {
    size_t p = 0, t = 0;
    size_t p_star = std::string_view::npos;
    size_t t_star = std::string_view::npos;

    while (t < text.size()) {
        if (p < pattern.size() && pattern[p] == '*') {
            p_star = p++;
            t_star = t;
            continue;
        }
        bool match = false;
        size_t next_p = p;

        if (p < pattern.size()) {
            if (pattern[p] == '\\') {
                if (p + 1 < pattern.size() && pattern[p + 1] == text[t]) {
                    match = true;
                    next_p = p + 2;
                }
            } else if (pattern[p] == '?') {
                match = true;
                next_p = p + 1;
            } else if (pattern[p] == '[') {
                size_t temp_p = p + 1;
                bool negate = (temp_p < pattern.size() && pattern[temp_p] == '^');
                if (negate) temp_p++;

                bool found = false;
                while (temp_p < pattern.size() && pattern[temp_p] != ']') {
                    if (temp_p + 2 < pattern.size() && pattern[temp_p + 1] == '-') {
                        if (text[t] >= pattern[temp_p] && text[t] <= pattern[temp_p + 2]) found = true;
                        temp_p += 3;
                    } else {
                        if (text[t] == pattern[temp_p]) found = true;
                        temp_p++;
                    }
                }

                if (temp_p < pattern.size() && (found != negate)) {
                    match = true;
                    next_p = temp_p + 1;
                }
            } else if (pattern[p] == text[t]) {
                match = true;
                next_p = p + 1;
            }
        }
        if (match) {
            p = next_p;
            t++;
        } else if (p_star != std::string_view::npos) {
            p = p_star + 1;
            t = ++t_star;
        } else {
            return false;
        }
    }

    while (p < pattern.size() && pattern[p] == '*')
        ++p;
    return p == pattern.size();
}

results::ListViewResult DaikonDatabase::Keys(std::string_view pattern) {
    std::vector<std::string> matches;
    db_.ForEachKey([&](std::string_view key) {
        if (MatchGlobPattern(pattern, key)) {
            matches.push_back(std::string(key));
        }
    });

    return results::ListViewResult{std::move(matches)};
}

results::IntResult DaikonDatabase::MemoryUsage(std::string_view key) {
    auto* obj = db_.Get(key);
    if (!obj) return 0;

    size_t key_mem = key.size() + 32;
    uint32_t obj_mem = obj->GetMemoryUsage() + sizeof(*obj);
    return obj_mem + key_mem;
}

results::StatusResult DaikonDatabase::Expire(std::string_view key, int64_t seconds) {
    if (seconds < 0) return false;
    return db_.Expire(key, std::chrono::seconds(seconds));
}

results::IntResult DaikonDatabase::Ttl(std::string_view key) {
    auto* obj = db_.Get(key);
    if (!obj) return -2;
    if (!obj->HasTtl()) return -1;
    auto now = core::types::Clock::now();
    auto remains = std::chrono::duration_cast<std::chrono::seconds>(obj->GetTtl() - now).count();
    return remains < 0 ? -2 : remains;
}

} // namespace daikon