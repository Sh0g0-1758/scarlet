#pragma once
#include <binary_operations/binop.hh>
#include <iostream>
#include <optional>
#include <string>
#include <token/token.hh>
#include <unary_operations/unop.hh>
#include <vector>
/*

Grammar:

<program> ::= <function>
<function> ::= "int" <identifier> "(" "void" ")" "{" <statement> "}"
<statement> ::= "return" <exp> ";"
<exp> ::= <factor> | <exp> <binop> <exp>
<factor> ::= <int> | <unop> <factor> | "(" <exp> ")"
<unop> ::= "~" | "-"
<binop> ::= "+" | "-" | "*" | "/" | "%"
<identifier> ::= ? An identifier token ?
<int> ::= ? A constant token ?

*/
namespace scarlet {
namespace ast {

class AST_int_Node {
private:
  std::string value;

public:
  std::string get_AST_name() { return "Int"; }
  std::string get_value() { return value; }
  void set_value(std::string value) { this->value = std::move(value); }
};

class AST_identifier_Node {
private:
  std::string value;

public:
  std::string get_AST_name() { return "Identifier"; }
  std::string get_value() { return value; }
  void set_identifier(std::string value) { this->value = std::move(value); }
};

class AST_unop_Node {
private:
  unop::UNOP op;

public:
  std::string get_AST_name() { return "Unop"; }
  std::string get_op() { return unop::to_string_unop(op); }
  void set_op(unop::UNOP unop) { this->op = unop; }
};

class AST_binop_Node {
private:
  binop::BINOP op;

public:
  std::string get_AST_name() { return "Binop"; }
  binop::BINOP get_op() { return op; }
  void set_op(binop::BINOP binop) { this->op = binop; }
};

class AST_exp_Node;

class AST_factor_Node {
private:
  AST_int_Node int_node;
  std::vector<AST_unop_Node> unop_nodes;
  AST_exp_Node *exp_node = nullptr;

public:
  std::string get_AST_name() { return "Factor"; }
  AST_int_Node &get_int_node() { return int_node; }
  void set_int_node(AST_int_Node &&int_node) {
    this->int_node = std::move(int_node);
  }
  void set_unop_node(AST_unop_Node &&unop_node) {
    this->unop_nodes.emplace_back(std::move(unop_node));
  }
  std::vector<AST_unop_Node> &get_unop_nodes() { return unop_nodes; }
  void set_exp_node(AST_exp_Node *exp_node) { this->exp_node = exp_node; }
  AST_exp_Node *get_exp_node() { return exp_node; }
};

/*
The exp node will be structured as a binary tree which has the following
members:

1. Binary Operator (binop): The operator (e.g., +, -, *, /) that combines two
operands
2. Left Factor and Right Factor: The operands of the binary operator
3. Left Child: A recursive reference to another expression node

exp ==

         binop
        /     \
      exp     factor

exp ==

         binop
        /     \
      factor  factor

We say that this is left assosciative because the expression is always evaluated
from left to right.
*/

class AST_exp_Node {
private:
  AST_binop_Node binop;
  AST_factor_Node factor_left;
  AST_factor_Node factor_right;
  AST_exp_Node *left;

public:
  std::string get_AST_name() { return "Exp"; }
  AST_binop_Node &get_binop_node() { return binop; }
  void set_binop_node(AST_binop_Node &&binop) {
    this->binop = std::move(binop);
  }
  AST_factor_Node &get_left_factor_node() { return factor_left; }
  void set_left_factor_node(AST_factor_Node &&factor) {
    this->factor_left = std::move(factor);
  }
  AST_factor_Node &get_right_factor_node() { return factor_right; }
  void set_right_factor_node(AST_factor_Node &&factor) {
    this->factor_right = std::move(factor);
  }
  void set_left(AST_exp_Node *left) { this->left = left; }
  AST_exp_Node *get_left() { return left; }
};

class AST_Statement_Node {
private:
  std::vector<AST_exp_Node *> exps;
  std::string type;

public:
  AST_Statement_Node(std::string type) {
    exps.reserve(2);
    this->type = type;
  }
  std::string get_AST_name() { return "Statement"; }
  std::vector<AST_exp_Node *> &get_exps() { return exps; }
  void add_exp(AST_exp_Node *exp) { exps.emplace_back(exp); }
  std::string get_type() { return type; }
};

class AST_Function_Node {
private:
  std::vector<AST_Statement_Node> statements;
  AST_identifier_Node identifier;

public:
  AST_Function_Node() { statements.reserve(2); }
  std::string get_AST_name() { return "Function"; }
  std::vector<AST_Statement_Node> &get_statements() { return statements; }
  AST_identifier_Node &get_identifier() { return identifier; }
  void add_statement(AST_Statement_Node statement) {
    statements.emplace_back(statement);
  }
  void set_identifier(AST_identifier_Node identifier) {
    this->identifier = std::move(identifier);
  }
};

class AST_Program_Node {
private:
  std::vector<AST_Function_Node> functions;

public:
  AST_Program_Node() { functions.reserve(2); }
  std::string get_AST_name() { return "Program"; }
  std::vector<AST_Function_Node> &get_functions() { return functions; }
  void add_function(AST_Function_Node function) {
    functions.emplace_back(function);
  }
};
} // namespace ast
} // namespace scarlet
