#pragma once

#include "regex.cc"
#include <fstream>
#include <iostream>
#include <string>
#include <token/token.hh>
#include <vector>

class lexer {
private:
  std::vector<Token> tokens;
  Regex regex;
  bool success = true;
  std::string file_path;

public:
  void tokenize();
  void print_tokens();
  std::vector<Token> get_tokens();
  void set_file_path(std::string &&file_path) {
    this->file_path = std::move(file_path);
  };
  bool is_success() { return success; }
};
