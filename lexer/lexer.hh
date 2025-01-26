#pragma once

#include <fstream>
#include <iostream>
#include <regex/regex.hh>
#include <string>
#include <token/token.hh>
#include <vector>

namespace Scarlet{
namespace Lexer {
class lexer {
private:
  std::vector<Scarlet::Token::Token> tokens;
  Scarlet::regex::Regex regex;
  bool success = true;
  std::string file_path;

public:
  void tokenize();
  void print_tokens();
  std::vector<Scarlet::Token::Token> get_tokens();
  void set_file_path(std::string &&file_path) {
    this->file_path = std::move(file_path);
  };
  bool is_success() { return success; }
};
}
}
