#ifndef LEXER_H
#define LEXER_H

#include <vector>
#include <string>
#include "../tokens/tokens.hh"
using namespace std;

class Lexer {
private:
    const string digits = "0123456789";

public:
    Lexer() = default;

    bool isDigit(char ch) const;

    vector<Token*> scan(const std::string& re);
};

#endif