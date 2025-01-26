#pragma once

#include <fstream>
#include <iostream>
#include <regex/regex.hh>
#include <string>
#include <token/token.hh>
#include <vector>

namespace scarlet {
namespace lexer {
class lexer {
private:
  std::vector<token::Token> tokens;
  regex::Regex regex;
  bool success = true;
  std::string file_path;

public:
  void tokenize();
  void print_tokens();
  std::vector<token::Token> get_tokens();
  void set_file_path(std::string &&file_path) {
    this->file_path = std::move(file_path);
  };
  bool is_success() { return success; }
};
} // namespace lexer
} // namespace scarlet
