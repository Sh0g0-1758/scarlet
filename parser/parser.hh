#pragma once

#include <ast/ast.hh>
#include <binary_operations/binop.hh>
#include <iostream>
#include <map>
#include <stack>
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
  int symbol_counter = 0;
  std::map<std::string, bool> goto_labels;
  ast::AST_Program_Node program;
  int loop_start_counter = 0;
  int loop_end_counter = 0;
  std::stack<std::string> loop_start_labels;
  std::stack<std::string> loop_end_labels;
  std::shared_ptr<ast::AST_Function_Node>
  parse_function(std::vector<token::Token> &tokens);
  void parse_block(std::vector<token::Token> &tokens,
                   std::shared_ptr<ast::AST_Block_Node> block);
  void parse_block_item(std::vector<token::Token> &tokens,
                        std::shared_ptr<ast::AST_Block_Node> function);
  void parse_declaration(std::vector<token::Token> &tokens,
                         std::shared_ptr<ast::AST_Block_Node> function);
  void parse_statement(std::vector<token::Token> &tokens,
                       std::shared_ptr<ast::AST_Block_Node> function);
  void parse_factor(std::vector<token::Token> &tokens,
                    std::shared_ptr<ast::AST_factor_Node> factor);
  void parse_exp(std::vector<token::Token> &tokens,
                 std::shared_ptr<ast::AST_exp_Node> &exp, int prec = 0);
  void parse_unary_op(std::vector<token::Token> &tokens,
                      std::shared_ptr<ast::AST_factor_Node> factor);
  void parse_identifier(std::vector<token::Token> &tokens,
                        std::shared_ptr<ast::AST_Function_Node> function);
  void parse_int(std::vector<token::Token> &tokens,
                 std::shared_ptr<ast::AST_factor_Node> factor);
  void parse_binop(std::vector<token::Token> &tokens,
                   std::shared_ptr<ast::AST_binop_Node> binop);
  void expect(token::TOKEN actual_token, token::TOKEN expected_token);
  std::pair<bool, int>
  is_single_identifier_parentheses(std::vector<token::Token> &tokens);
  void eof_error(token::Token token);
  void pretty_print_exp(std::shared_ptr<ast::AST_exp_Node> exp);
  void pretty_print_factor(std::shared_ptr<ast::AST_factor_Node> factor);
  void pretty_print_block(std::shared_ptr<ast::AST_Block_Node> block);
  void
  analyze_exp(std::shared_ptr<ast::AST_exp_Node> exp,
              std::map<std::pair<std::string, int>, std::string> &symbol_table,
              int indx);
  void analyze_block(
      std::shared_ptr<ast::AST_Block_Node> block,
      std::map<std::pair<std::string, int>, std::string> &symbol_table,
      int indx);

  std::string get_temp_name(std::string &name) {
    std::string tmp = name + "." + std::to_string(symbol_counter);
    symbol_counter++;
    return tmp;
  }

  std::string get_loop_start_label() {
    std::string label = "loopStart." + std::to_string(loop_start_counter);
    loop_start_counter++;
    loop_start_labels.push(label);
    return label;
  }

  std::string get_loop_end_label() {
    std::string label = "loopEnd." + std::to_string(loop_end_counter);
    loop_end_counter++;
    loop_end_labels.push(label);
    return label;
  }

  std::string get_prev_loop_start_label() { return loop_start_labels.top(); }

  std::string get_prev_loop_end_label() { return loop_end_labels.top(); }

  void remove_loop_start_label() { loop_start_labels.pop(); }
  void remove_loop_end_label() { loop_end_labels.pop(); }

public:
  void parse_program(std::vector<token::Token> tokens);
  void semantic_analysis();
  bool is_success() { return success; }
  void display_errors();
  ast::AST_Program_Node get_program() { return program; }
  void pretty_print();
  int get_symbol_counter() { return symbol_counter; }
};
} // namespace parser
} // namespace scarlet
