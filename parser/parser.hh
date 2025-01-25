#pragma once
#include <ast/ast.hh>
#include <iostream>
#include <string>
#include <token/token.hh>
#include <vector>
#include <unary_operations/unop.hh>

class parser {
private:
  bool success = true;
  std::vector<std::string> error_messages;
  AST_Program_Node program;
  AST_Function_Node parse_function(std::vector<Token> &tokens);
  void parse_statement(std::vector<Token> &tokens, AST_Function_Node &function);
  void parse_exp(std::vector<Token> &tokens, AST_exp_Node &exp);
  void parse_unary_op(std::vector<Token> &tokens, AST_exp_Node &exp);
  void parse_identifier(std::vector<Token> &tokens,
                        AST_Function_Node &function);
  void parse_int(std::vector<Token> &tokens, AST_exp_Node &exp);
  void expect(TOKEN actual_token, TOKEN expected_token);
  void eof_error(Token token);

public:
  void parse_program(std::vector<Token> tokens);
  bool is_success() { return success; }
  void display_errors();
  AST_Program_Node get_program() { return program; }
  void pretty_print();
};
