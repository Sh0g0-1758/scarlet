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
<<<<<<< HEAD
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
=======
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
>>>>>>> 5d3a511 (add regex engine and created namespace for tokens)
  bool is_success() { return success; }
  void display_errors();
  ast::AST_Program_Node get_program() { return program; }
  void pretty_print();
};
} // namespace parser
} // namespace scarlet
