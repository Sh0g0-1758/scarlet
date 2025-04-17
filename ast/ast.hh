#pragma once
#include <binary_operations/binop.hh>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <token/token.hh>
#include <tools/constant/constant.hh>
#include <tools/macros/macros.hh>
#include <unary_operations/unop.hh>
#include <vector>

// clang-format off
/*

Grammar:

<program> ::= { <declaration> }

<declaration> ::= <variable-declaration> | <function-declaration> | <struct-declaration>

<variable-declaration> ::= { <specifier> }+ <declarator> [ "=" <exp> ] ";"

<function-declaration> ::= { <specifier> }+ <declarator> ( <block> | ";" )

<struct-declaration> ::= "struct" <identifier> [ "{" { <member-declaration> }+ "}" ] ";"

<member-declaration> ::= { <type-specifier> }+ <declarator> ";"

<declarator> ::= "*" <declarator>
               | <identifier> [ <declarator-suffix> ]
               | "(" <declarator> ")" [ <declarator-suffix> ]

<declarator-suffix> ::= { "[" <const> "]" }+
                      | "(" <param-list> ")"

<param-list> ::= "void" | <param> { "," <param> }

<param> ::= { <type-specifier> }+ <declarator>

<type-specifier> ::= "int" | "long" |"unsigned" | "signed" | "char" | "double" | "void" | "struct" <identifier>

<specifier> ::= <type-specifier> | "static" | "extern"

<block_item> ::= <statement> | <declaration>

<block> ::= "{" { <block_item> } "}"

<for-init> ::= <variable-declaration> | [ <exp> ]

<statement> ::= 
              | "return" <exp> ";" 
              | <exp> ";" | ";" 
              | "if" "(" <exp> ")" <statement> [ "else" <statement> ] 
              | "goto" <identifier> ";" 
              | <identifier> ":" 
              | <block> 
              | "break" ";" 
              | "continue" ";" 
              | "while" "(" <exp> ")" <statement> 
              | "for" "(" <for-init> ";" [ <exp> ] ";" [ <exp> ] ")" <statement> 
              | "do" <statement> "while" "(" <exp> ")" ";" 
              | switch "("<exp>")" <statement> 
              | case <exp> ":" { <statement> }

<exp> ::= <factor> | <exp> <binop> <exp> | <exp> "?" <exp> ":" <exp>

<factor> ::= <const> { <postfix-op> }
           | <identifier> { <postfix-op> }
           | "(" { <type-specifier> }+ [ <abstract-declarator> ] ")" <factor> 
           | <unop> <factor> 
           | "(" <exp> ")" { <postfix-op> }
           | <identifier> "(" [ <argument-list> ] ")" { <postfix-op> }
           | <postfix-exp>
           | { <string> }+ { <postfix-op> }

<postfix-op> ::= "[" <exp> "]"
                | "." <identifier>
                | "->" <identifier>

<postfix-exp> ::= <identifier> { "[" <const> "]" }+
                | "(" <exp> ")" { "[" <const> "]" }+

<abstract-declarator> ::= "*"
                        | "*" <abstract-declarator> 
                        | "(" <abstract-declarator> ")"

<argument-list> ::= <exp> { "," <exp> }

<unop> ::= "~" | "-" | "!" | "--" | "++" | "*" | "&"

<binop> ::= "+" | "-" | "*" | "/" | "%" | "&" | "|" | "^" | "<<" | ">>" | "==" | "!=" | "<" | "<=" | ">" | ">=" | "&&" | "||"  | "="

<const> ::= <int> | <long> | <uint> | <ulong> 

<identifier> ::= ? An identifier token ?

<string> ::= ? A string token ?

<char> ::= ? A char token ?

<int> ::= ? An int token ?

<uint> ::= ? An unsigned int token ?

<long> ::= ? An int or long token ?

<ulong> ::= ? An unsigned int or unsigned long token ?

<double> ::= ? A floating-point constant token ?

NOTE: in EBNF notation,
1. {} means 0 or more instances of the enclosed expression
2. {}+ means 1 or more instances of the enclosed expression
3. [] means 0 or 1 instances of the enclosed expression ie. it is optional

*/
// clang-format on
namespace scarlet {
namespace ast {

enum class SpecifierType { NONE, STATIC, EXTERN };
// Assignment to these values help us store the derived type as a flat vector in
// the global symbol table
enum class ElemType {
  NONE = 0,
  DERIVED = -1,
  POINTER = -2,
  INT = -3,
  LONG = -4,
  ULONG = -5,
  UINT = -6,
  DOUBLE = -7,
  CHAR = -8,
  UCHAR = -9,
  VOID = -10,
  STRUCT = -11,
};

class AST_exp_Node;
class AST_identifier_Node;

class AST_postfix_op_Node {
private:
  std::shared_ptr<AST_exp_Node> postfix_exp;
  std::shared_ptr<AST_identifier_Node> arrow_identifier;
  std::shared_ptr<AST_identifier_Node> dot_identifier;
public:
  std::string get_AST_name() { return "PostfixOp"; }
  std::shared_ptr<AST_exp_Node> get_postfix_exp() { return postfix_exp; }
  void set_postfix_exp(std::shared_ptr<AST_exp_Node> postfix_exp) {
    this->postfix_exp = std::move(postfix_exp);
  }
  std::shared_ptr<AST_identifier_Node> get_arrow_identifier() {
    return arrow_identifier;
  }
  void set_arrow_identifier(std::shared_ptr<AST_identifier_Node> identifier) {
    this->arrow_identifier = std::move(identifier);
  }
  std::shared_ptr<AST_identifier_Node> get_dot_identifier() {
    return dot_identifier;
  }
  void set_dot_identifier(std::shared_ptr<AST_identifier_Node> identifier) {
    this->dot_identifier = std::move(identifier);
  }
};

class AST_const_Node {
private:
  constant::Constant constant;

public:
  std::string get_AST_name() { return "Constant"; }
  constant::Constant &get_constant() { return constant; }
  void set_constant(constant::Constant constant) { this->constant = constant; }
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

enum class FactorType { BASIC, FUNCTION_CALL };
class AST_declarator_Node;

class AST_factor_Node {
private:
  std::shared_ptr<AST_const_Node> const_node;
  std::shared_ptr<AST_identifier_Node> identifier_node;
  std::shared_ptr<AST_unop_Node> unop_node;
  std::shared_ptr<AST_postfix_op_Node> postfix_op_node;
  // No need to make this a weak pointer because if an object of exp a points to
  // factor b then factor b can never point to exp a. It can only point to
  // another object of exp, say c. exp -> factor -> exp
  //  a  ->    b   ->  c
  std::shared_ptr<AST_exp_Node> exp_node;
  FactorType factorType = FactorType::BASIC;
  ElemType castType;
  std::shared_ptr<AST_declarator_Node> castDeclarator;
  std::shared_ptr<AST_factor_Node> child;
  ElemType type = ElemType::NONE;
  std::vector<std::shared_ptr<ast::AST_exp_Node>> arrIdx;
  std::vector<long> derivedType{};

public:
  void copy(std::shared_ptr<AST_factor_Node> other) {
    this->const_node = other->const_node;
    this->identifier_node = other->identifier_node;
    this->unop_node = other->unop_node;
    this->exp_node = other->exp_node;
    this->factorType = other->factorType;
    this->castType = other->castType;
    this->castDeclarator = other->castDeclarator;
    this->child = other->child;
    this->type = other->type;
    this->arrIdx = other->arrIdx;
    this->derivedType = other->derivedType;
    this->postfix_op_node = other->postfix_op_node;
  }

  void purge() {
    this->const_node = nullptr;
    this->identifier_node = nullptr;
    this->unop_node = nullptr;
    this->exp_node = nullptr;
    this->factorType = FactorType::BASIC;
    this->castType = ElemType::NONE;
    this->castDeclarator = nullptr;
    this->child = nullptr;
    this->type = ElemType::NONE;
    this->arrIdx.clear();
    this->derivedType.clear();
    this->postfix_op_node = nullptr;
  }
  std::string get_AST_name() { return "Factor"; }
  std::shared_ptr<AST_const_Node> get_const_node() { return const_node; }
  void set_const_node(std::shared_ptr<AST_const_Node> const_node) {
    this->const_node = std::move(const_node);
  }
  std::shared_ptr<AST_identifier_Node> get_identifier_node() {
    return identifier_node;
  }
  void
  set_identifier_node(std::shared_ptr<AST_identifier_Node> identifier_node) {
    this->identifier_node = std::move(identifier_node);
  }
  std::shared_ptr<AST_unop_Node> get_unop_node() { return unop_node; }
  void set_unop_node(std::shared_ptr<AST_unop_Node> unop_node) {
    this->unop_node = std::move(unop_node);
  }
  void set_exp_node(std::shared_ptr<AST_exp_Node> exp_node) {
    this->exp_node = std::move(exp_node);
  }
  std::shared_ptr<AST_exp_Node> get_exp_node() { return exp_node; }

  FactorType get_factor_type() { return factorType; }
  void set_factor_type(FactorType type) { this->factorType = type; }

  ElemType get_cast_type() { return castType; }
  void set_cast_type(ElemType castType) { this->castType = castType; }

  std::shared_ptr<AST_declarator_Node> get_cast_declarator() {
    return castDeclarator;
  }
  void
  set_cast_declarator(std::shared_ptr<AST_declarator_Node> castDeclarator) {
    this->castDeclarator = std::move(castDeclarator);
  }

  std::shared_ptr<AST_factor_Node> get_child() { return child; }
  void set_child(std::shared_ptr<AST_factor_Node> child) {
    this->child = std::move(child);
  }

  ElemType get_type() { return type; }
  void set_type(ElemType type) { this->type = type; }

  std::vector<std::shared_ptr<ast::AST_exp_Node>> get_arrIdx() {
    return arrIdx;
  }
  void add_arrIdx(std::shared_ptr<ast::AST_exp_Node> arrIdx) {
    this->arrIdx.emplace_back(arrIdx);
  }
  void set_arrIdx(std::vector<std::shared_ptr<ast::AST_exp_Node>> arrIdx) {
    this->arrIdx = arrIdx;
  }
  std::vector<long> get_derived_type() { return derivedType; }
  void set_derived_type(std::vector<long> derivedType) {
    this->derivedType = std::move(derivedType);
  }
  std::shared_ptr<AST_postfix_op_Node> get_postfix_op_node() {
    return postfix_op_node;
  }
  void set_postfix_op_node(std::shared_ptr<AST_postfix_op_Node> postfix_op_node) {
    this->postfix_op_node = std::move(postfix_op_node);
  }
};

class AST_factor_function_call_Node : public AST_factor_Node {
private:
  std::vector<std::shared_ptr<AST_exp_Node>> arguments;

public:
  std::string get_AST_name() { return "FunctionCall"; }
  std::vector<std::shared_ptr<AST_exp_Node>> get_arguments() {
    return arguments;
  }
  void set_arguments(std::vector<std::shared_ptr<AST_exp_Node>> arguments) {
    this->arguments = arguments;
  }
  void add_argument(std::shared_ptr<AST_exp_Node> argument) {
    arguments.emplace_back(argument);
  }
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
  ElemType type = ElemType::NONE;
  std::vector<long> derivedType{};

public:
  void copy(std::shared_ptr<AST_exp_Node> other) {
    this->binop = other->binop;
    this->factor = other->factor;
    this->right = other->right;
    this->left = other->left;
    this->type = other->type;
    this->derivedType = other->derivedType;
  }
  void purge() {
    this->binop = nullptr;
    this->factor = nullptr;
    this->right = nullptr;
    this->left = nullptr;
    this->type = ElemType::NONE;
    this->derivedType.clear();
  }
  std::string get_AST_name() { return "Exp"; }

  std::shared_ptr<AST_binop_Node> get_binop_node() { return binop; }
  void set_binop_node(std::shared_ptr<AST_binop_Node> binop) {
    this->binop = std::move(binop);
  }

  std::shared_ptr<AST_factor_Node> get_factor_node() { return factor; }
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

  ElemType get_type() { return type; }
  void set_type(ElemType type) { this->type = type; }

  std::vector<long> get_derived_type() { return derivedType; }
  void set_derived_type(std::vector<long> derivedType) {
    this->derivedType = std::move(derivedType);
  }
};

class AST_ternary_exp_Node : public AST_exp_Node {
private:
  std::shared_ptr<AST_exp_Node> middle;

public:
  AST_ternary_exp_Node() = default;
  AST_ternary_exp_Node(std::shared_ptr<AST_exp_Node> parent) {
    this->set_binop_node(parent->get_binop_node());
    this->set_factor_node(parent->get_factor_node());
    this->set_left(parent->get_left());
    this->set_right(parent->get_right());
  }
  std::string get_AST_name() { return "TernaryExp"; }
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

enum class DeclarationType { VARIABLE, FUNCTION, STRUCT};

class AST_declarator_Node {
private:
  bool pointer = false;
  std::vector<long> arrDim;
  std::shared_ptr<AST_declarator_Node> child;

public:
  std::string get_AST_name() { return "Declarator"; }
  bool is_pointer() { return pointer; }
  void set_pointer(bool pointer) { this->pointer = pointer; }
  std::shared_ptr<AST_declarator_Node> get_child() { return child; }
  void set_child(std::shared_ptr<AST_declarator_Node> child) {
    this->child = std::move(child);
  }
  std::vector<long> get_arrDim() { return arrDim; }
  void add_dim(long dim) { arrDim.emplace_back(dim); }
};

class AST_member_declaration_Node {
private:
  std::shared_ptr<AST_declarator_Node> declarator;
  ElemType base_type;
public:
  std::string get_AST_name() { return "MemberDeclaration"; }
  std::shared_ptr<AST_declarator_Node> get_declarator() { return declarator; }
  void set_declarator(std::shared_ptr<AST_declarator_Node> declarator) {
    this->declarator = std::move(declarator);
  }
  void set_base_type(ElemType type) { this->base_type = type; }
  ElemType get_base_type() { return base_type; }
};

class AST_Declaration_Node {
private:
  std::shared_ptr<AST_declarator_Node> declarator;
  std::shared_ptr<AST_identifier_Node> identifier;
  DeclarationType type;
  SpecifierType specifier;

public:
  std::string get_AST_name() { return "Declaration"; }
  std::shared_ptr<AST_declarator_Node> get_declarator() { return declarator; }
  void set_declarator(std::shared_ptr<AST_declarator_Node> declarator) {
    this->declarator = std::move(declarator);
  }

  std::shared_ptr<AST_identifier_Node> get_identifier() { return identifier; }
  void set_identifier(std::shared_ptr<AST_identifier_Node> identifier) {
    this->identifier = std::move(identifier);
  }

  DeclarationType get_type() { return type; }
  void set_type(DeclarationType type) { this->type = type; }

  SpecifierType get_specifier() { return specifier; }
  void set_specifier(SpecifierType specifier) { this->specifier = specifier; }
};

class AST_struct_declaration_Node : public AST_Declaration_Node {
private:
  std::vector<std::shared_ptr<AST_member_declaration_Node>> members;
public:
  std::string get_AST_name() { return "StructDeclaration"; }
  std::vector<std::shared_ptr<AST_member_declaration_Node>> get_members() {
    return members;
  }
  void add_member(std::shared_ptr<AST_member_declaration_Node> member) {
    members.emplace_back(std::move(member));
  }
  void set_members(std::vector<std::shared_ptr<AST_member_declaration_Node>> members) {
    this->members = std::move(members);
  }
};

struct initializer {
  std::vector<std::shared_ptr<initializer>> initializer_list;
  std::vector<std::shared_ptr<AST_exp_Node>> exp_list;
  std::vector<int> exp_indx;
};

class AST_variable_declaration_Node : public AST_Declaration_Node {
private:
  std::shared_ptr<AST_exp_Node> exp;
  std::shared_ptr<initializer> init;
  ElemType base_type;

public:
  std::string get_AST_name() { return "VariableDeclaration"; }
  void set_exp(std::shared_ptr<AST_exp_Node> exp) {
    this->exp = std::move(exp);
  }
  std::shared_ptr<AST_exp_Node> get_exp() { return exp; }
  void set_base_type(ElemType type) { this->base_type = type; }
  ElemType get_base_type() { return base_type; }
  void set_initializer(std::shared_ptr<initializer> init) {
    this->init = std::move(init);
  }
  std::shared_ptr<initializer> get_initializer() { return init; }
};

struct Param {
  ElemType base_type;
  std::shared_ptr<AST_declarator_Node> declarator;
  std::shared_ptr<AST_identifier_Node> identifier;

  // This lets us use a macro which simplifies function params parsing
  void set_type(ElemType type) { this->base_type = type; }
  ElemType get_type() { return base_type; }
};

class AST_Block_Node;

class AST_function_declaration_Node : public AST_Declaration_Node {
private:
  std::vector<std::shared_ptr<Param>> params;
  ElemType return_type;
  std::shared_ptr<AST_Block_Node> block;

public:
  std::string get_AST_name() { return "FunctionDeclaration"; }
  std::vector<std::shared_ptr<Param>> get_params() { return params; }
  void add_param(std::shared_ptr<Param> param) {
    params.emplace_back(std::move(param));
  }
  std::shared_ptr<AST_Block_Node> get_block() { return block; }
  void set_block(std::shared_ptr<AST_Block_Node> block) {
    this->block = std::move(block);
  }
  void set_return_type(ElemType return_type) {
    this->return_type = return_type;
  }
  ElemType get_return_type() { return return_type; }
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

class AST_Program_Node {
private:
  std::vector<std::shared_ptr<AST_Declaration_Node>> declarations;

public:
  AST_Program_Node() { declarations.reserve(2); }
  std::string get_AST_name() { return "Program"; }
  std::vector<std::shared_ptr<AST_Declaration_Node>> get_declarations() {
    return declarations;
  }
  void add_declaration(std::shared_ptr<AST_Declaration_Node> declaration) {
    declarations.emplace_back(std::move(declaration));
  }
};
class AST_switch_statement_Node : public AST_Statement_Node {
private:
  std::shared_ptr<AST_Statement_Node> stmt;
  std::vector<std::pair<std::shared_ptr<AST_exp_Node>,
                        std::shared_ptr<AST_identifier_Node>>>
      case_exp_label;
  bool has_default_case = false;

public:
  std::string get_AST_name() { return "Switch Statement"; }
  std::shared_ptr<AST_Statement_Node> get_stmt() { return stmt; }
  void set_stmt(std::shared_ptr<AST_Statement_Node> stmt) {
    this->stmt = std::move(stmt);
  }
  void set_case_exp_label(std::shared_ptr<AST_exp_Node> exp,
                          std::shared_ptr<AST_identifier_Node> label) {
    this->case_exp_label.push_back(
        std::make_pair(std::move(exp), std::move(label)));
  }
  std::vector<std::pair<std::shared_ptr<AST_exp_Node>,
                        std::shared_ptr<AST_identifier_Node>>>
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
  std::string case_label;

public:
  std::string get_AST_name() { return "CaseStatement"; }
  std::string get_case_label() { return case_label; }
  void set_case_label(std::string &&case_label) {
    this->case_label = std::move(case_label);
  }
};

ElemType constTypeToElemType(constant::Type t);
constant::Type elemTypeToConstType(ElemType t);
std::string to_string(ElemType type);
std::string to_string(SpecifierType type);
std::pair<ElemType, std::vector<long>>
getParentType(ElemType left, ElemType right, std::vector<long> &leftDerivedType,
              std::vector<long> &rightDerivedType,
              std::shared_ptr<AST_exp_Node> exp);
std::pair<ElemType, std::vector<long>>
getAssignType(ElemType target, std::vector<long> targetDerived, ElemType src,
              std::vector<long> srcDerived,
              std::shared_ptr<AST_exp_Node> srcExp);
constant::Constant castConstToElemType(constant::Constant c, ElemType type);
bool isComplexType(ElemType type);
bool is_lvalue(std::shared_ptr<AST_factor_Node> factor);
bool is_const_zero(std::shared_ptr<AST_factor_Node> factor);
bool is_lvalue(std::shared_ptr<AST_factor_Node> factor);
int getSizeOfTypeOnArch(ElemType type);
unsigned long getSizeOfReferencedTypeOnArch(std::vector<long> derivedType);
unsigned long getSizeOfArrayTypeOnArch(std::vector<long> derivedType);
std::string get_lvalue_identifier(std::shared_ptr<AST_factor_Node> factor);
bool exp_is_factor(std::shared_ptr<AST_exp_Node> exp);
bool is_array(std::shared_ptr<AST_factor_Node> factor);
bool is_array(std::shared_ptr<AST_exp_Node> exp);
bool is_void_ptr(ast::ElemType type, std::vector<long> derivedType);
bool is_ptr_type(ast::ElemType type, std::vector<long> derivedType);
bool is_pointer_to_complete_type(ast::ElemType type,
                                 std::vector<long> derivedType);
bool validate_type_specifier(ast::ElemType type, std::vector<long> derivedType);
bool is_valid_declarator(ast::ElemType type, std::vector<long> derivedType);
void unroll_derived_type(std::shared_ptr<ast::AST_declarator_Node> declarator,
                         std::vector<long> &derivedType);
bool exp_is_string(std::shared_ptr<AST_exp_Node> exp);
bool factor_is_string(std::shared_ptr<AST_factor_Node> factor);
} // namespace ast
} // namespace scarlet
