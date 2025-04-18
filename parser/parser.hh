#pragma once

#include <ast/ast.hh>
#include <binary_operations/binop.hh>
#include <iostream>
#include <map>
#include <set>
#include <stack>
#include <string>
#include <token/token.hh>
#include <tools/macros/macros.hh>
#include <tools/symbolTable/symbolTable.hh>
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
  int loop_continue_counter = 0;
  int loop_end_counter = 0;
  int ifelse_counter = 0;
  int switch_counter = 0;
  int case_counter = 0;
  int goto_label_counter = 0;
  // Used to annotate goto labels during parsing and explicitly cast
  // return expressions in a function to the correct type during semantic
  // analysis
  std::string currFuncName{};
  std::stack<std::string> loop_start_labels;
  // break can appear in both loop and switch statements
  std::stack<std::string> loop_switch_end_labels;
  std::stack<std::shared_ptr<ast::AST_switch_statement_Node>> switch_stack;
  std::map<std::string, symbolTable::symbolInfo> globalSymbolTable;
  void
  parse_param_list(std::vector<token::Token> &tokens,
                   std::shared_ptr<ast::AST_function_declaration_Node> decl);
  void parse_function_declaration(
      std::vector<token::Token> &tokens,
      std::shared_ptr<ast::AST_function_declaration_Node> decl,
      bool atGlobalLevel);
  void parse_variable_declaration(
      std::vector<token::Token> &tokens,
      std::shared_ptr<ast::AST_variable_declaration_Node> decl);
  void parse_initializer(std::vector<token::Token> &tokens,
                         std::shared_ptr<ast::initializer> &init);
  void parse_declarator(std::vector<token::Token> &tokens,
                        std::shared_ptr<ast::AST_declarator_Node> &decl,
                        std::shared_ptr<ast::AST_identifier_Node> &identifier);
  void parse_variable_declarator_suffix(
      std::vector<token::Token> &tokens,
      std::shared_ptr<ast::AST_declarator_Node> &declarator);
  void parse_function_declarator(
      std::vector<token::Token> &tokens,
      std::shared_ptr<ast::AST_declarator_Node> &decl,
      std::shared_ptr<ast::AST_identifier_Node> &identifier,
      std::shared_ptr<ast::AST_function_declaration_Node> &funcDecl,
      bool &haveParams);
  void parse_function_declarator_suffix(
      std::vector<token::Token> &tokens,
      std::shared_ptr<ast::AST_function_declaration_Node> &funcDecl,
      bool &haveParams);
  void parse_block(std::vector<token::Token> &tokens,
                   std::shared_ptr<ast::AST_Block_Node> block);
  void parse_block_item(std::vector<token::Token> &tokens,
                        std::shared_ptr<ast::AST_Block_Node> &block);
  void
  parse_declaration(std::vector<token::Token> &tokens,
                    std::shared_ptr<ast::AST_Declaration_Node> &declaration,
                    bool atGlobalLevel = false);
  void parse_statement(std::vector<token::Token> &tokens,
                       std::shared_ptr<ast::AST_Statement_Node> &stmt);
  void parse_for_init(std::vector<token::Token> &tokens,
                      std::shared_ptr<ast::AST_For_Statement_Node> &forstmt);
  void parse_factor(std::vector<token::Token> &tokens,
                    std::shared_ptr<ast::AST_factor_Node> &factor);
  void
  parse_factor_post_incr_decr(std::vector<token::Token> &tokens,
                              std::shared_ptr<ast::AST_factor_Node> &factor);
  void parse_abstract_declarator(
      std::vector<token::Token> &tokens,
      std::shared_ptr<ast::AST_declarator_Node> &declarator);
  void parse_exp(std::vector<token::Token> &tokens,
                 std::shared_ptr<ast::AST_exp_Node> &exp, int prec = 0);
  void parse_unary_op(std::vector<token::Token> &tokens,
                      std::shared_ptr<ast::AST_factor_Node> &factor);
  void parse_const(std::vector<token::Token> &tokens,
                   std::shared_ptr<ast::AST_factor_Node> &factor);
  void parse_binop(std::vector<token::Token> &tokens,
                   std::shared_ptr<ast::AST_binop_Node> &binop);
  void expect(token::TOKEN actual_token, token::TOKEN expected_token);
  void eof_error(token::Token token);
  void pretty_print_exp(std::shared_ptr<ast::AST_exp_Node> exp);
  void pretty_print_factor(std::shared_ptr<ast::AST_factor_Node> factor);
  void pretty_print_block(std::shared_ptr<ast::AST_Block_Node> block);
  void pretty_print_declaration(
      std::shared_ptr<ast::AST_Declaration_Node> declaration);
  void
  pretty_print_declarator(std::shared_ptr<ast::AST_declarator_Node> declarator);
  void
  pretty_print_statement(std::shared_ptr<ast::AST_Statement_Node> statement);
  void pretty_print_variable_declaration(
      std::shared_ptr<ast::AST_variable_declaration_Node> declaration);
  void pretty_print_function_declaration(
      std::shared_ptr<ast::AST_function_declaration_Node> declaration);
  void pretty_print_initializer(std::shared_ptr<ast::initializer> init);
  void analyze_exp(std::shared_ptr<ast::AST_exp_Node> exp,
                   std::map<std::pair<std::string, int>,
                            symbolTable::symbolInfo> &symbol_table,
                   int indx);
  void analyze_factor(std::shared_ptr<ast::AST_factor_Node> factor,
                      std::map<std::pair<std::string, int>,
                               symbolTable::symbolInfo> &symbol_table,
                      int indx);
  void analyze_block(std::shared_ptr<ast::AST_Block_Node> block,
                     std::map<std::pair<std::string, int>,
                              symbolTable::symbolInfo> &symbol_table,
                     int indx);
  void analyze_goto_labels();
  void analyze_declaration(
      std::shared_ptr<ast::AST_Declaration_Node> declaration,
      std::map<std::pair<std::string, int>, symbolTable::symbolInfo>
          &symbol_table,
      int indx);
  void analyze_array_initializer(
      std::shared_ptr<ast::initializer> init,
      std::map<std::pair<std::string, int>, symbolTable::symbolInfo>
          &symbol_table,
      int indx, std::vector<long> arrDim, ast::ElemType baseElemType,
      std::vector<long> derivedElemType);
  void init_static_array_initializer(std::shared_ptr<ast::initializer> init,
                                     std::vector<long> arrDim,
                                     ast::ElemType baseElemType,
                                     std::vector<long> derivedElemType,
                                     symbolTable::symbolInfo &varInfo);
  void get_arrInfo(std::vector<long> &arrDim, ast::ElemType &baseElemType,
                   std::vector<long> &derivedElemType,
                   symbolTable::symbolInfo &varInfo);
  void initialize_global_variable(
      symbolTable::symbolInfo &varInfo,
      std::shared_ptr<ast::AST_variable_declaration_Node> varDecl,
      std::string &var_name);
  bool previous_declaration_has_same_type(
      ast::ElemType prev_base_type, std::vector<long> prev_derived_type,
      std::shared_ptr<ast::AST_declarator_Node> curr_declarator,
      ast::ElemType curr_base_type);
  void analyze_global_variable_declaration(
      std::shared_ptr<ast::AST_variable_declaration_Node> varDecl,
      std::map<std::pair<std::string, int>, symbolTable::symbolInfo>
          &symbol_table);
  void analyze_global_function_declaration(
      std::shared_ptr<ast::AST_function_declaration_Node> funcDecl,
      std::map<std::pair<std::string, int>, symbolTable::symbolInfo>
          &symbol_table);
  void analyze_function_declaration(
      std::shared_ptr<ast::AST_function_declaration_Node> funcDecl,
      std::map<std::pair<std::string, int>, symbolTable::symbolInfo>
          &symbol_table,
      std::string &var_name, int indx);
  void analyze_local_variable_declaration(
      std::shared_ptr<ast::AST_variable_declaration_Node> varDecl,
      std::map<std::pair<std::string, int>, symbolTable::symbolInfo>
          &symbol_table,
      std::string &var_name, int indx);
  template <typename T>
  void analyze_switch_case(
      std::shared_ptr<ast::AST_switch_statement_Node> switch_statement,
      constant::Type switchType);
  void assign_type_to_factor(std::shared_ptr<ast::AST_factor_Node> factor);
  void assign_type_to_exp(std::shared_ptr<ast::AST_exp_Node> exp);
  void add_cast_to_exp(std::shared_ptr<ast::AST_exp_Node> exp,
                       ast::ElemType type, std::vector<long> derivedType);
  void add_cast_to_factor(std::shared_ptr<ast::AST_factor_Node> factor,
                          ast::ElemType type, std::vector<long> derivedType);
  void analyze_statement(std::shared_ptr<ast::AST_Statement_Node> statement,
                         std::map<std::pair<std::string, int>,
                                  symbolTable::symbolInfo> &symbol_table,
                         int indx);
  void decay_arr_to_pointer(std::shared_ptr<ast::AST_factor_Node> factor,
                            std::shared_ptr<ast::AST_exp_Node> exp);
  void analyze_for_statement(
      std::shared_ptr<ast::AST_For_Statement_Node> for_statement,
      std::map<std::pair<std::string, int>, symbolTable::symbolInfo>
          &symbol_table,
      int indx);
  std::string get_temp_name(std::string &name) {
    std::string tmp = name + "." + std::to_string(symbol_counter);
    symbol_counter++;
    return tmp;
  }

  std::string get_temp_name(std::string &&name) {
    std::string tmp = name + "." + std::to_string(symbol_counter);
    symbol_counter++;
    return tmp;
  }

  std::pair<std::shared_ptr<ast::AST_identifier_Node>,
            std::shared_ptr<ast::AST_identifier_Node>>
  get_ifelse_labels() {
    std::string if_label = "if_end." + std::to_string(ifelse_counter);
    std::string else_label = "else_end." + std::to_string(ifelse_counter);
    ifelse_counter++;
    MAKE_SHARED(ast::AST_identifier_Node, if_label_node);
    MAKE_SHARED(ast::AST_identifier_Node, else_label_node);
    if_label_node->set_identifier(if_label);
    else_label_node->set_identifier(else_label);
    return {if_label_node, else_label_node};
  }

  std::pair<std::shared_ptr<ast::AST_identifier_Node>,
            std::shared_ptr<ast::AST_identifier_Node>>
  get_loop_labels() {
    std::string start_label =
        "loop_continue." + std::to_string(loop_continue_counter);
    std::string end_label = "loop_end." + std::to_string(loop_end_counter);
    loop_continue_counter++;
    loop_end_counter++;
    loop_start_labels.push(start_label);
    loop_switch_end_labels.push(end_label);
    MAKE_SHARED(ast::AST_identifier_Node, start_label_node);
    MAKE_SHARED(ast::AST_identifier_Node, end_label_node);
    start_label_node->set_identifier(start_label);
    end_label_node->set_identifier(end_label);
    return {start_label_node, end_label_node};
  }
  std::pair<std::shared_ptr<ast::AST_identifier_Node>,
            std::shared_ptr<ast::AST_identifier_Node>>
  get_switch_loop_labels() {
    std::string label = "switch." + std::to_string(switch_counter);
    switch_counter++;
    loop_switch_end_labels.push(label);
    MAKE_SHARED(ast::AST_identifier_Node, label_node);
    label_node->set_identifier(label);
    return {nullptr, label_node};
  }

  std::shared_ptr<ast::AST_identifier_Node> get_case_label() {
    std::string case_label = "case." + std::to_string(case_counter);
    case_counter++;
    MAKE_SHARED(ast::AST_identifier_Node, label);
    label->set_identifier(case_label);
    return label;
  }

  std::string get_loop_start_label() {
    std::string label = "loop_start." + std::to_string(loop_start_counter);
    loop_start_counter++;
    return label;
  }

  std::shared_ptr<ast::AST_identifier_Node> get_prev_loop_start_label() {
    if (loop_start_labels.empty()) {
      return nullptr;
    }
    MAKE_SHARED(ast::AST_identifier_Node, label);
    label->set_identifier(loop_start_labels.top());
    return label;
  }

  std::shared_ptr<ast::AST_identifier_Node> get_prev_loop_end_label() {
    if (loop_switch_end_labels.empty()) {
      return nullptr;
    }
    MAKE_SHARED(ast::AST_identifier_Node, label);
    label->set_identifier(loop_switch_end_labels.top());
    return label;
  }

  void remove_loop_labels() {
    loop_start_labels.pop();
    loop_switch_end_labels.pop();
  }

  void remove_switch_loop_label() { loop_switch_end_labels.pop(); }

  std::string to_string(ast::statementType type) {
    switch (type) {
    case ast::statementType::NULLSTMT:
      return "NullStmt";
    case ast::statementType::RETURN:
      return "Return";
    case ast::statementType::EXP:
      return "Exp";
    case ast::statementType::IF:
      return "If";
    case ast::statementType::IFELSE:
      return "IfElse";
    case ast::statementType::GOTO:
      return "Goto";
    case ast::statementType::LABEL:
      return "Label";
    case ast::statementType::BLOCK:
      return "Block";
    case ast::statementType::CONTINUE:
      return "Continue";
    case ast::statementType::BREAK:
      return "Break";
    case ast::statementType::WHILE:
      return "While";
    case ast::statementType::DO_WHILE:
      return "DoWhile";
    case ast::statementType::FOR:
      return "For";
    case ast::statementType::CASE:
      return "Case";
    case ast::statementType::DEFAULT_CASE:
      return "DefaultCase";
    case ast::statementType::SWITCH:
      return "Switch";
    case ast::statementType::UNKNOWN:
      UNREACHABLE()
    }
    return "";
  }

public:
  void parse_program(std::vector<token::Token> tokens);
  void semantic_analysis();
  bool is_success() { return success; }
  void display_errors();
  ast::AST_Program_Node get_program() { return program; }
  void pretty_print();
  int get_symbol_counter() { return symbol_counter; }
  std::map<std::string, symbolTable::symbolInfo> getGlobalSymbolTable() {
    return globalSymbolTable;
  }
};
} // namespace parser
} // namespace scarlet
