#pragma once

#include <ast/ast.hh>
#include <iostream>
#include <string>
#include <token/token.hh>
#include <unary_operations/unop.hh>
#include <vector>
namespace scarlet {
namespace parser {
class parser {
private:
  bool success = true;
  std::vector<std::string> error_messages;
  ast::AST_Program_Node program;
  ast::AST_Function_Node parse_function(std::vector<token::Token> &tokens);
  void parse_statement(std::vector<token::Token> &tokens,
                       ast::AST_Function_Node &function);
  void parse_exp(std::vector<token::Token> &tokens, ast::AST_exp_Node &exp);
  void parse_unary_op(std::vector<token::Token> &tokens,
                      ast::AST_exp_Node &exp);
  void parse_identifier(std::vector<token::Token> &tokens,
                        ast::AST_Function_Node &function);
  void parse_int(std::vector<token::Token> &tokens, ast::AST_exp_Node &exp);
  void expect(token::TOKEN actual_token, token::TOKEN expected_token);
  void eof_error(token::Token token);

public:
  void parse_program(std::vector<token::Token> tokens);
  bool is_success() { return success; }
  void display_errors();
  ast::AST_Program_Node get_program() { return program; }
  void pretty_print();
};
} // namespace parser
} // namespace scarlet
