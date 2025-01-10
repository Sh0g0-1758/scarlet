#include "re.hh"

bool regex_engine::matchCharacter(const std::string& pattern, char c) {
    auto result = re.match(pattern, std::string(1, c));
    bool matched;
    int consumed;
    std::vector<std::deque<Match>> all_matches;
    std::tie(matched, consumed, all_matches) = result;
    return matched;
}

bool regex_engine::matchWord(char c) {
    return matchCharacter("^[a-zA-Z_]$", c);
}

bool regex_engine::matchDigit(char c) {
    return matchCharacter("^[0-9]$", c);
}