#pragma once
#include <binary_operations/binop.hh>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <token/token.hh>
#include <unary_operations/unop.hh>
#include <vector>

// clang-format off
/*

Grammar:

<program> ::= <function>
<function> ::= "int" <identifier> "(" "void" ")" "{" { <block_item> } "}"
<block_item> ::= <statement> | <declaration>
<declaration> ::= "int" <identifier> [ "=" <exp> ] ";"
<statement> ::= "return" <exp> ";" | <exp> ";" | ";" | "if" "(" <exp> ")" <statement> [ "else" <statement> ]
<exp> ::= <factor> | <exp> <binop> <exp> | <exp> "?" <exp> ":" <exp>
<factor> ::= <int> | <identifier> | <unop> <factor> | "(" <exp> ")"
<unop> ::= "~" | "-" | "!"
<binop> ::= "+" | "-" | "*" | "/" | "%" | "&" | "|" | "^" | "<<" | ">>" | "==" | "!=" | "<" | "<=" | ">" | ">=" | "&&" | "||"  | "="
<identifier> ::= ? An identifier
token ? <int> ::= ? A constant token ?

NOTE: in EBNF notation,
1. {} means 0 or more instances of the enclosed expression
2. [] means 0 or 1 instances of the enclosed expression ie. it is optional

*/
// clang-format on
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
  unop::UNOP get_op() { return op; }
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
  std::shared_ptr<AST_int_Node> int_node;
  std::shared_ptr<AST_identifier_Node> identifier_node;
  std::vector<std::shared_ptr<AST_unop_Node>> unop_nodes;
  // No need to make this a weak pointer because if an object of exp a points to
  // factor b then factor b can never point to exp a. It can only point to
  // another object of exp, say c. exp -> factor -> exp
  //  a  ->    b   ->  c
  std::shared_ptr<AST_exp_Node> exp_node;

public:
  std::string get_AST_name() { return "Factor"; }

  std::shared_ptr<AST_int_Node> get_int_node() { return int_node; }
  void set_int_node(std::shared_ptr<AST_int_Node> int_node) {
    this->int_node = std::move(int_node);
  }
  std::shared_ptr<AST_identifier_Node> get_identifier_node() {
    return identifier_node;
  }
  void
  set_identifier_node(std::shared_ptr<AST_identifier_Node> identifier_node) {
    this->identifier_node = std::move(identifier_node);
  }
  std::vector<std::shared_ptr<AST_unop_Node>> get_unop_nodes() {
    return unop_nodes;
  }
  void set_unop_node(std::shared_ptr<AST_unop_Node> unop_node) {
    unop_nodes.emplace_back(std::move(unop_node));
  }
  void set_exp_node(std::shared_ptr<AST_exp_Node> exp_node) {
    this->exp_node = std::move(exp_node);
  }
  std::shared_ptr<AST_exp_Node> get_exp_node() { return exp_node; }
};

/*
The exp node will be structured as a binary tree which has the following
members:

1. Binary Operator (binop): The operator (e.g., +, -, *, /) that combines two
operands
2. Factor: A factor node that represents a single value
3. Left Child: A recursive reference to another expression node
4. Right Child: A recursive reference to another expression node

exp ==

         binop
        /     \
      exp     exp

exp ==

         binop
        /     \
      factor  exp

We say that this is left assosciative because the expression is always evaluated
from left to right.
*/

class AST_exp_Node {
private:
  std::shared_ptr<AST_binop_Node> binop;
  std::shared_ptr<AST_factor_Node> factor;
  std::shared_ptr<AST_exp_Node> right;
  std::shared_ptr<AST_exp_Node> left;
  // NOTE : This is only used for the ternary operator
  std::shared_ptr<AST_exp_Node> middle;

public:
  std::string get_AST_name() { return "Exp"; }

  std::shared_ptr<AST_binop_Node> get_binop_node() { return binop; }
  void set_binop_node(std::shared_ptr<AST_binop_Node> binop) {
    this->binop = std::move(binop);
  }

  const std::shared_ptr<AST_factor_Node> get_factor_node() { return factor; }
  void set_factor_node(std::shared_ptr<AST_factor_Node> factor) {
    this->factor = std::move(factor);
  }

  std::shared_ptr<AST_exp_Node> get_right() { return right; }
  void set_right(std::shared_ptr<AST_exp_Node> right) {
    this->right = std::move(right);
  }

  std::shared_ptr<AST_exp_Node> get_left() { return left; }
  void set_left(std::shared_ptr<AST_exp_Node> left) {
    this->left = std::move(left);
  }

  std::shared_ptr<AST_exp_Node> get_middle() { return middle; }
  void set_middle(std::shared_ptr<AST_exp_Node> middle) {
    this->middle = std::move(middle);
  }
};

// We don't include the empty statement (just having a semicolon) in the AST
enum class StatementType { UNKNOWN, RETURN, EXP, IF, ELSE };

class AST_Statement_Node {
private:
  std::shared_ptr<AST_exp_Node> exps;
  StatementType type;

public:
  std::string get_AST_name() { return "Statement"; }
  void set_type(StatementType type) { this->type = type; }
  StatementType get_type() { return type; }

  std::shared_ptr<AST_exp_Node> get_exps() { return exps; }
  void set_exp(std::shared_ptr<AST_exp_Node> exp) {
    this->exps = std::move(exp);
  }
};

enum class DeclarationType { INT };

class AST_Declaration_Node {
private:
  std::shared_ptr<AST_identifier_Node> identifier;
  std::optional<std::shared_ptr<AST_exp_Node>> exp;
  DeclarationType type;

public:
  // need to update this when we support more types
  AST_Declaration_Node() { type = DeclarationType::INT; }
  std::string get_AST_name() { return "Declaration"; }
  std::shared_ptr<AST_identifier_Node> get_identifier() { return identifier; }
  void set_identifier(std::shared_ptr<AST_identifier_Node> identifier) {
    this->identifier = std::move(identifier);
  }
  std::shared_ptr<AST_exp_Node> get_exp() {
    if (exp.has_value()) {
      return exp.value();
    }
    return nullptr;
  }
  void set_exp(std::shared_ptr<AST_exp_Node> exp) {
    this->exp = std::move(exp);
  }
};

enum class BlockItemType { UNKNOWN, STATEMENT, DECLARATION };

// Right now, we determine whether a block item is a statement or a declaration
// by simply peeking the next token. If its an int, we say its a declaration,
// else its a statement.
class AST_Block_Item_Node {
private:
  std::shared_ptr<AST_Statement_Node> statement;
  std::shared_ptr<AST_Declaration_Node> declaration;
  BlockItemType type;

public:
  std::string get_AST_name() { return "BlockItem"; }
  std::shared_ptr<AST_Statement_Node> get_statement() { return statement; }
  void set_statement(std::shared_ptr<AST_Statement_Node> statement) {
    this->statement = std::move(statement);
  }
  std::shared_ptr<AST_Declaration_Node> get_declaration() {
    return declaration;
  }
  void set_declaration(std::shared_ptr<AST_Declaration_Node> declaration) {
    this->declaration = std::move(declaration);
  }
  BlockItemType get_type() { return type; }
  void set_type(BlockItemType type) { this->type = type; }
};

class AST_Function_Node {
private:
  std::vector<std::shared_ptr<AST_Block_Item_Node>> blockItems;
  std::shared_ptr<AST_identifier_Node> identifier;

public:
  std::string get_AST_name() { return "Function"; }
  std::shared_ptr<AST_identifier_Node> get_identifier() { return identifier; }
  void set_identifier(std::shared_ptr<AST_identifier_Node> identifier) {
    this->identifier = std::move(identifier);
  }
  std::vector<std::shared_ptr<AST_Block_Item_Node>> get_blockItems() {
    return blockItems;
  }
  void add_blockItem(std::shared_ptr<AST_Block_Item_Node> statement) {
    blockItems.emplace_back(std::move(statement));
  }
};

class AST_Program_Node {
private:
  std::vector<std::shared_ptr<AST_Function_Node>> functions;

public:
  AST_Program_Node() { functions.reserve(2); }
  std::string get_AST_name() { return "Program"; }
  std::vector<std::shared_ptr<AST_Function_Node>> get_functions() {
    return functions;
  }
  void add_function(std::shared_ptr<AST_Function_Node> function) {
    functions.emplace_back(std::move(function));
  }
};
} // namespace ast
} // namespace scarlet
