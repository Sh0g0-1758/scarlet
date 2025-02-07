#pragma once

#include <fstream>
#include <iomanip>
#include <iostream>
#include <regex/regex.hh>
#include <string>
#include <token/token.hh>
#include <vector>

#define RESET "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define CYAN "\033[36m"
#define BOLD "\033[1m"
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
  void print_symbol_table();
  bool is_success() { return success; }
};
} // namespace lexer
} // namespace scarlet
