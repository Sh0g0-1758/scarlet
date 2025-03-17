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
<function> ::= "int" <identifier> "(" "void" ")" <block>
<block_item> ::= <statement> | <declaration>
<block> ::= "{" { <block_item> } "}"
<declaration> ::= "int" <identifier> [ "=" <exp> ] ";"
<for-init> ::= <declaration> | [ <exp> ]
<statement> ::= "return" <exp> ";" | <exp> ";" | ";" | "if" "(" <exp> ")" <statement> [ "else" <statement> ] | "goto" <identifier> ";" | <identifier> ":" | <block> | "break" ";" | "continue" ";" | "while" "(" <exp> ")" <statement> | "for" "(" <for-init> ";" [ <exp> ] ";" [ <exp> ] ")" <statement> | "do" <statement> "while" "(" <exp> ")" ";" | switch "("<exp>")" <statement> | case <exp> ":" { <statement> }
<exp> ::= <factor> | <exp> <binop> <exp> | <exp> "?" <exp> ":" <exp>
<factor> ::= <int> | <identifier> | <unop> <factor> | "(" <exp> ")"
<unop> ::= "~" | "-" | "!" | "--" | "++"
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
  void add_unop_node(std::shared_ptr<AST_unop_Node> unop_node) {
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

class AST_Block_Node;

// We don't include the empty statement (just having a semicolon) in the AST
// instead of an else statement, we have an ifelse statement type.
// since each if statement can either exist on its own or have an else statement
enum class statementType {
  UNKNOWN,
  NULLSTMT,
  RETURN,
  EXP,
  IF,
  IFELSE,
  GOTO,
  LABEL,
  BLOCK,
  BREAK,
  CONTINUE,
  WHILE,
  FOR,
  SWITCH,
  CASE,
  DEFAULT_CASE,
  DO_WHILE
};

class AST_Statement_Node {
private:
  std::shared_ptr<AST_exp_Node> exps;
  statementType type;
  // Labels serve different purpose for different statements
  std::pair<std::shared_ptr<AST_identifier_Node>,
            std::shared_ptr<AST_identifier_Node>>
      labels;

public:
  std::string get_AST_name() { return "Statement"; }
  void set_type(statementType type) { this->type = type; }
  statementType get_type() { return type; }

  std::shared_ptr<AST_exp_Node> get_exps() { return exps; }
  void set_exp(std::shared_ptr<AST_exp_Node> exp) {
    this->exps = std::move(exp);
  }

  std::pair<std::shared_ptr<AST_identifier_Node>,
            std::shared_ptr<AST_identifier_Node>>
  get_labels() {
    return labels;
  }
  void set_labels(std::pair<std::shared_ptr<AST_identifier_Node>,
                            std::shared_ptr<AST_identifier_Node>>
                      labels) {
    this->labels = std::move(labels);
  }
};

class AST_block_statement_node : public AST_Statement_Node {
private:
  std::shared_ptr<AST_Block_Node> block;

public:
  std::string get_AST_name() { return "BlockStatement"; }
  std::shared_ptr<AST_Block_Node> get_block() { return block; }
  void set_block(std::shared_ptr<AST_Block_Node> block) {
    this->block = std::move(block);
  }
};

class AST_if_else_statement_Node : public AST_Statement_Node {
private:
  std::shared_ptr<AST_Statement_Node> stmt1;
  std::shared_ptr<AST_Statement_Node> stmt2;

public:
  std::string get_AST_name() {
    if (this->get_type() == statementType::IF) {
      return "IfStatement";
    } else {
      return "IfElseStatement";
    }
  }
  std::shared_ptr<AST_Statement_Node> get_stmt1() { return stmt1; }
  void set_stmt1(std::shared_ptr<AST_Statement_Node> stmt1) {
    this->stmt1 = std::move(stmt1);
  }
  std::shared_ptr<AST_Statement_Node> get_stmt2() { return stmt2; }
  void set_stmt2(std::shared_ptr<AST_Statement_Node> stmt2) {
    this->stmt2 = std::move(stmt2);
  }
};

class AST_while_statement_Node : public AST_Statement_Node {
private:
  std::shared_ptr<AST_Statement_Node> stmt;
  std::string start_label;

public:
  std::string get_AST_name() { return "WhileStatement"; }
  std::shared_ptr<AST_Statement_Node> get_stmt() { return stmt; }
  void set_stmt(std::shared_ptr<AST_Statement_Node> stmt) {
    this->stmt = std::move(stmt);
  }
  std::string get_start_label() { return start_label; }
  void set_start_label(std::string &&start_label) {
    this->start_label = std::move(start_label);
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

class AST_For_Init_Node {
private:
  std::shared_ptr<AST_Declaration_Node> declaration;
  std::shared_ptr<AST_exp_Node> exp;

public:
  std::string get_AST_name() { return "ForInit"; }
  std::shared_ptr<AST_Declaration_Node> get_declaration() {
    return declaration;
  }
  void set_declaration(std::shared_ptr<AST_Declaration_Node> declaration) {
    this->declaration = std::move(declaration);
  }
  std::shared_ptr<AST_exp_Node> get_exp() { return exp; }
  void set_exp(std::shared_ptr<AST_exp_Node> exp) {
    this->exp = std::move(exp);
  }
};

class AST_For_Statement_Node : public AST_Statement_Node {
private:
  std::shared_ptr<AST_For_Init_Node> for_init;
  std::shared_ptr<AST_exp_Node> exp2;
  std::shared_ptr<AST_Statement_Node> stmt;
  std::string start_label;

public:
  std::string get_AST_name() { return "ForStatement"; }
  std::shared_ptr<AST_For_Init_Node> get_for_init() { return for_init; }
  void set_for_init(std::shared_ptr<AST_For_Init_Node> for_init) {
    this->for_init = std::move(for_init);
  }
  std::shared_ptr<AST_exp_Node> get_exp2() { return exp2; }
  void set_exp2(std::shared_ptr<AST_exp_Node> exp2) {
    this->exp2 = std::move(exp2);
  }

  std::shared_ptr<AST_Statement_Node> get_stmt() { return stmt; }
  void set_stmt(std::shared_ptr<AST_Statement_Node> stmt) {
    this->stmt = std::move(stmt);
  }

  std::string get_start_label() { return start_label; }
  void set_start_label(std::string &&start_label) {
    this->start_label = std::move(start_label);
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

class AST_Block_Node {
private:
  std::vector<std::shared_ptr<AST_Block_Item_Node>> blockItems;

public:
  std::string get_AST_name() { return "Block"; }
  std::vector<std::shared_ptr<AST_Block_Item_Node>> &get_blockItems() {
    return blockItems;
  }
  void add_blockItem(std::shared_ptr<AST_Block_Item_Node> statement) {
    blockItems.emplace_back(std::move(statement));
  }
};

class AST_Function_Node {
private:
  std::shared_ptr<AST_Block_Node> block;
  std::shared_ptr<AST_identifier_Node> identifier;

public:
  std::string get_AST_name() { return "Function"; }
  std::shared_ptr<AST_identifier_Node> get_identifier() { return identifier; }
  void set_identifier(std::shared_ptr<AST_identifier_Node> identifier) {
    this->identifier = std::move(identifier);
  }
  std::shared_ptr<AST_Block_Node> get_block() { return block; }
  void set_block(std::shared_ptr<AST_Block_Node> block) {
    this->block = std::move(block);
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
class AST_switch_statement_Node : public AST_Statement_Node {
private:
  std::shared_ptr<AST_Statement_Node> stmt;
  std::vector<std::pair<std::shared_ptr<ast::AST_exp_Node>,
                        std::shared_ptr<ast::AST_identifier_Node>>>
      case_exp_label;
  bool has_default_case = false;

public:
  std::string get_AST_name() { return "Switch Statement"; }
  std::shared_ptr<AST_Statement_Node> get_stmt() { return stmt; }
  void set_stmt(std::shared_ptr<AST_Statement_Node> stmt) {
    this->stmt = std::move(stmt);
  }
  void set_case_exp_label(std::shared_ptr<ast::AST_exp_Node> exp,
                          std::shared_ptr<ast::AST_identifier_Node> label) {
    this->case_exp_label.push_back(
        std::make_pair(std::move(exp), std::move(label)));
  }
  std::vector<std::pair<std::shared_ptr<ast::AST_exp_Node>,
                        std::shared_ptr<ast::AST_identifier_Node>>>
  get_case_exp_label() {
    return case_exp_label;
  }
  void set_has_default_case(bool has_default_case) {
    this->has_default_case = has_default_case;
  }
  bool get_has_default_case() { return has_default_case; }
};

class AST_case_statement_Node : public AST_Statement_Node {
private:
  // std::vector<std::shared_ptr<AST_Block_Item_Node>> stmts;
  std::string case_label;

public:
  std::string get_AST_name() { return "CaseStatement"; }
  // std::vector<std::shared_ptr<AST_Block_Item_Node>> get_stmt() { return
  // stmts; } void set_stmt(std::shared_ptr<AST_Block_Item_Node> stmt) {
  //   this->stmts.push_back(std::move(stmt));
  // }
  std::string get_case_label() { return case_label; }
  void set_case_label(std::string &&case_label) {
    this->case_label = std::move(case_label);
  }
};
} // namespace ast
} // namespace scarlet
