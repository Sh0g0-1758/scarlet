#pragma once

#include <fstream>
#include <iomanip>
#include <iostream>
#include <regex/regex.hh>
#include <string>
#include <token/token.hh>
#include <tools/color/color.hh>
#include <vector>

namespace scarlet {
namespace lexer {
class lexer {
private:
  std::vector<token::Token> tokens;
  std::vector<std::pair<std::string, std::string>> error_recovery;
  regex::Regex regex;
  bool success = true;
  std::string file_path;
  int line_number = 1;
  int col_number = 1;

public:
  void tokenize();
  void print_tokens();
  std::vector<token::Token> get_tokens();
  void set_file_path(std::string &&file_path) {
    this->file_path = std::move(file_path);
  };
  void print_symbol_table();
  void print_error_recovery();
  bool is_success() { return success; }
};
} // namespace lexer
} // namespace scarlet
