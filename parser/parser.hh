#pragma once

#include <ast/ast.hh>
#include <iostream>
#include <string>
#include <token/token.hh>
#include <unary_operations/unop.hh>
#include <vector>

namespace Scarlet {
namespace Parser {
class parser {
private:
  bool success = true;
  std::vector<std::string> error_messages;
  AST::AST_Program_Node program;
  AST::AST_Function_Node parse_function(std::vector<Token::Token> &tokens);
  void parse_statement(std::vector<Token::Token> &tokens, AST::AST_Function_Node &function);
  void parse_exp(std::vector<Token::Token> &tokens, AST::AST_exp_Node &exp);
  void parse_unary_op(std::vector<Token::Token> &tokens, AST::AST_exp_Node &exp);
  void parse_identifier(std::vector<Token::Token> &tokens,
                        AST::AST_Function_Node &function);
  void parse_int(std::vector<Token::Token> &tokens, AST::AST_exp_Node &exp);
  void expect(Token::TOKEN actual_token, Token::TOKEN expected_token);
  void eof_error(Token::Token token);

public:
  void parse_program(std::vector<Token::Token> tokens);
  bool is_success() { return success; }
  void display_errors();
  AST::AST_Program_Node get_program() { return program; }
  void pretty_print();
};
}
}