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
<<<<<<< HEAD
  std::vector<token::Token> tokens;
  regex::Regex regex;
=======
  std::string file_contents;
  std::vector<scarlet::Token> tokens;
  Regex regex;
>>>>>>> 5d3a511 (add regex engine and created namespace for tokens)
  bool success = true;
  std::string file_path;

public:
  void tokenize();
  void print_tokens();
<<<<<<< HEAD
  std::vector<token::Token> get_tokens();
  void set_file_path(std::string &&file_path) {
    this->file_path = std::move(file_path);
  };
=======
  std::vector<scarlet::Token> get_tokens();
  void read_file(const std::string &file_path);
>>>>>>> 5d3a511 (add regex engine and created namespace for tokens)
  bool is_success() { return success; }
};
} // namespace lexer
} // namespace scarlet
