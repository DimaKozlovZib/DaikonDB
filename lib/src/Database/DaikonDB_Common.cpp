#include "../../includes/core/DaikonDB.h"

namespace daikon {

results::ViewResult DaikonDatabase::Type(std::string_view key) {
    auto* obj = db_.Get(key);
    if (!obj) return results::ViewResult{"none"};
    switch (obj->GetType()) {
        case core::types::DataType::kString:
            return results::ViewResult{"string"};
        case core::types::DataType::kList:
            return results::ViewResult{"list"};
        case core::types::DataType::kSet:
            return results::ViewResult{"set"};
        case core::types::DataType::kGeo:
            return results::ViewResult{"geo"};
        default:
            return results::ViewResult{"none"};
    }
}

results::IntResult DaikonDatabase::Del(const std::vector<std::string_view>& keys) {
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
    maxmemory_ = static_cast<std::size_t>(bytes);
    return results::StatusResult{true};
}

results::IntResult DaikonDatabase::ConfigGetMaxmemory() {
    return results::IntResult(maxmemory_);
}

results::IntResult DaikonDatabase::Dbsize() {
    return results::IntResult(db_.Size());
}

void DaikonDatabase::Flushdb() {
    db_.Clear();
}

static bool MatchGlobPattern(std::string_view pattern, std::string_view text) {
    size_t p = 0, t = 0;
    size_t star = std::string_view::npos;
    size_t match = 0;

    while (t < text.size()) {
        if (p < pattern.size() && pattern[p] == '\\') {
            ++p;
            if (p < pattern.size() && pattern[p] == text[t]) {
                ++p;
                ++t;
            } else if (p < pattern.size() && pattern[p] != text[t]) {
                return false;
            }
        } else if (p < pattern.size() && pattern[p] == '*') {
            star = p;
            match = t;
            ++p;
        } else if (p < pattern.size() && pattern[p] == '?') {
            ++p;
            ++t;
        } else if (p < pattern.size() && pattern[p] == '[') {
            ++p;
            bool negate = false;
            if (p < pattern.size() && pattern[p] == '^') {
                negate = true;
                ++p;
            }
            bool matched = false;
            while (p < pattern.size() && pattern[p] != ']') {
                if (p + 2 < pattern.size() && pattern[p + 1] == '-') {
                    char start = pattern[p];
                    char end = pattern[p + 2];
                    if (text[t] >= start && text[t] <= end) matched = true;
                    p += 3;
                } else {
                    if (text[t] == pattern[p]) matched = true;
                    ++p;
                }
            }
            if (p < pattern.size() && pattern[p] == ']') ++p;
            if ((negate && matched) || (!negate && !matched)) return false;
            ++t;
        } else if (p < pattern.size() && pattern[p] != text[t]) {
            if (star != std::string_view::npos) {
                p = star + 1;
                t = ++match;
            } else {
                return false;
            }
        } else {
            ++p;
            ++t;
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
    uint32_t obj_mem = obj->GetMemoryUsage();
    return obj_mem;
}

results::StatusResult DaikonDatabase::Expire(std::string_view key, int64_t seconds) {
    if (seconds < 0) return results::StatusResult{false};
    return results::StatusResult{db_.Expire(key, std::chrono::seconds(seconds))};
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