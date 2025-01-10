#ifndef RE_HH
#define RE_HH

#include <string>
#include <vector>
#include <deque>
#include <tuple>
#include "../src/regex/regex.hh"

class regex_engine {
private:
    RegexEngine re;

    bool matchCharacter(const std::string& pattern, char c);

public:
    bool matchWord(char c);
    bool matchDigit(char c);
};

#endif 