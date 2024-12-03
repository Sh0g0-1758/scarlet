#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include "regex.cc"
#include <token/token.hh>

class lexer {
private:
    std::string file_contents;
    std::vector<Token> tokens;
    Regex regex;
    bool success = true;
public:
    void tokenize();
    void print_tokens();
    std::vector<Token> get_tokens();
    void read_file(const std::string& file_path);
    bool is_success() { return success; }
};
