#pragma once
#include <ast/ast.hh>
#include <iostream>
#include <string>
#include <token/token.hh>
#include <vector>

class parser {
private:
  bool success = true;
  std::vector<std::string> error_messages;
  AST_Program_Node program;
  AST_Function_Node parse_function(std::vector<scarlet::Token> &tokens);
  void parse_statement(std::vector<scarlet::Token> &tokens, AST_Function_Node &function);
  void parse_exp(std::vector<scarlet::Token> &tokens, AST_exp_Node &exp);
  void parse_unary_op(std::vector<scarlet::Token> &tokens, AST_exp_Node &exp);
  void parse_identifier(std::vector<scarlet::Token> &tokens,
                        AST_Function_Node &function);
  void parse_int(std::vector<scarlet::Token> &tokens, AST_exp_Node &exp);
  void expect(scarlet::TOKEN actual_token, scarlet::TOKEN expected_token);
  void eof_error(scarlet::Token token);

public:
  void parse_program(std::vector<scarlet::Token> tokens);
  bool is_success() { return success; }
  void display_errors();
  AST_Program_Node get_program() { return program; }
  void pretty_print();
};
