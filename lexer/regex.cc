
#include "regex.hh"

bool Regex::matchWord(char c) {
    if ((c >= 'a' and c <= 'z') or (c >= 'A' and c <= 'Z') or (c == '_')) return true;
    return false;
}

bool Regex::matchDigit(char c) {
    if (c >= '0' and c <= '9') return true;
    return false;
}
