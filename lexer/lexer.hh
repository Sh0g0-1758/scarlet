#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include "regex.cc"

enum class TOKEN {
    IDENTIFIER,
    CONSTANT,
    INT,
    VOID,
    RETURN,
    OPEN_PARANTHESES,
    CLOSE_PARANTHESES,
    OPEN_BRACE,
    CLOSE_BRACE,
    SEMICOLON,
    UNKNOWN
};

class lexer {
private:
    std::string file_contents;
    std::vector<TOKEN> tokens;
    Regex regex;
public:
    void tokenize();
    void print_tokens();
    void read_file(const std::string& file_path);
};
