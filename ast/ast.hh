#pragma once
#include <iostream>
#include <optional>
#include <string>
#include <token/token.hh>
#include <vector>

/*

Grammar:

<program> ::= <function>
<function> ::= "int" <identifier> "(" "void" ")" "{" <statement> "}"
<statement> ::= "return" <exp> ";"
<exp> ::= <int> | <unop> <exp> | "(" <exp> ")"
<unop> ::= "~" | "-"
<identifier> ::= ? An identifier token ?
<int> ::= ? A constant token ?

*/

class AST_Node {
public:
  virtual std::string get_AST_name() { return "AST_Node"; }
  virtual ~AST_Node() = default;
};

class AST_int_Node : public AST_Node {
private:
  std::string value;

public:
  std::string get_AST_name() override { return "Int"; }
  std::string get_value() { return value; }
  void set_value(std::string value) { this->value = std::move(value); }
};

class AST_identifier_Node : public AST_Node {
private:
  std::string value;

public:
  std::string get_AST_name() override { return "Identifier"; }
  std::string get_value() { return value; }
  void set_identifier(std::string value) { this->value = std::move(value); }
};

class AST_unop_Node : public AST_Node {
private:
  std::string op;

public:
  std::string get_AST_name() override { return "Unop"; }
  std::string get_op() { return op; }
  void set_op(std::string op) { this->op = std::move(op); }
};

class AST_exp_Node : public AST_Node {
private:
  AST_int_Node int_node;
  std::vector<AST_unop_Node> unop_nodes;

public:
  std::string get_AST_name() override { return "Exp"; }
  AST_int_Node get_int_node() { return int_node; }
  void set_int_node(AST_int_Node int_node) {
    this->int_node = std::move(int_node);
  }
  std::vector<AST_unop_Node> get_unop_node() { return unop_nodes; }
  void set_unop_node(AST_unop_Node unop_node) {
    this->unop_nodes.emplace_back(std::move(unop_node));
  }
  std::vector<AST_unop_Node> get_unop_nodes() { return unop_nodes; }
};

class AST_Statement_Node : public AST_Node {
private:
  std::vector<AST_exp_Node> exps;
  std::string type;

public:
  AST_Statement_Node(std::string type) {
    exps.reserve(2);
    this->type = type;
  }
  std::string get_AST_name() override { return "Statement"; }
  std::vector<AST_exp_Node> get_exps() { return exps; }
  void add_exp(AST_exp_Node exp) { exps.emplace_back(exp); }
  std::string get_type() { return type; }
};

class AST_Function_Node : public AST_Node {
private:
  std::vector<AST_Statement_Node> statements;
  AST_identifier_Node identifier;

public:
  AST_Function_Node() { statements.reserve(2); }
  std::string get_AST_name() override { return "Function"; }
  std::vector<AST_Statement_Node> get_statements() { return statements; }
  AST_identifier_Node get_identifier() { return identifier; }
  void add_statement(AST_Statement_Node statement) {
    statements.emplace_back(statement);
  }
  void set_identifier(AST_identifier_Node identifier) {
    this->identifier = std::move(identifier);
  }
};

class AST_Program_Node : public AST_Node {
private:
  std::vector<AST_Function_Node> functions;

public:
  AST_Program_Node() { functions.reserve(2); }
  std::string get_AST_name() override { return "Program"; }
  std::vector<AST_Function_Node> get_functions() { return functions; }
  void add_function(AST_Function_Node function) {
    functions.emplace_back(function);
  }
};