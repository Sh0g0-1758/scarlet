#pragma once

#include "regex.cc"
#include <fstream>
#include <iostream>
#include <string>
#include <token/token.hh>
#include <vector>

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
  void read_file(const std::string &file_path);
  bool is_success() { return success; }
};
