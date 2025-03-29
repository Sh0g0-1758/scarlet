#pragma once

#include <iostream>
#include <optional>
#include <string>
#include <tools/macros/macros.hh>

namespace scarlet {
namespace token {
enum class TOKEN {
  UNKNOWN,
  // IDENTIFIER
  IDENTIFIER,
  // CONSTANTS
  CONSTANTSTART,
  INT_CONSTANT,
  UINT_CONSTANT,
  LONG_CONSTANT,
  ULONG_CONSTANT,
  DOUBLE_CONSTANT,
  CHARACTER_CONSTANT,
  CHAR_ARR,
  CONSTANTEND,
  // KEYWORDS
  KEYWORDSTART,
  INT,
  VOID,
  RETURN,
  IF,
  ELSE,
  DO,
  WHILE,
  FOR,
  BREAK,
  CONTINUE,
  STATIC,
  EXTERN,
  LONG,
  SIGNED,
  UNSIGNED,
  DOUBLE,
  CHAR,
  SIZEOF,
  STRUCT,
  GOTO,
  SWITCH,
  CASE,
  DEFAULT_CASE,
  KEYWORDEND,
  // GRAMMAR TOKENS
  GRAMMARSTART,
  OPEN_PARANTHESES,
  CLOSE_PARANTHESES,
  OPEN_BRACE,
  CLOSE_BRACE,
  SEMICOLON,
  OPEN_BRACKET,
  CLOSE_BRACKET,
  COLON,
  GRAMMAREND,
  // UNARY OPERATORS
  UNARYSTART,
  TILDE,
  HYPHEN,
  DECREMENT_OPERATOR,
  INCREMENT_OPERATOR,
  UNARYEND,
  // BINARY OPERATORS
  BINARYSTART,
  PLUS,
  ASTERISK,
  FORWARD_SLASH,
  PERCENT_SIGN,
  ASSIGNMENT,
  DOT,
  QUESTION_MARK,
  COMMA,
  COMPOUND_SUM,
  COMPOUND_DIFFERENCE,
  COMPOUND_PRODUCT,
  COMPOUND_DIVISION,
  COMPOUND_REMAINDER,
  ARROW_OPERATOR,
  BINARYEND,
  // BITWISE OPERATORS
  BITWISESTART,
  AAND,
  AOR,
  XOR,
  LEFT_SHIFT,
  RIGHT_SHIFT,
  COMPOUND_LEFTSHIFT,
  COMPOUND_RIGHTSHIFT,
  COMPOUND_AND,
  COMPOUND_XOR,
  COMPOUND_OR,
  BITWISEEND,
  // LOGICAL OPERATORS
  LOGICALSTART,
  NOT,
  LAND,
  LOR,
  EQUAL,
  NOTEQUAL,
  LESSTHAN,
  GREATERTHAN,
  LESSTHANEQUAL,
  GREATERTHANEQUAL,
  LOGICALEND
};

class Token {
private:
  TOKEN token;
  std::optional<std::string> value;

public:
  Token(TOKEN token, std::optional<std::string> value)
      : token(token), value(value) {}
  Token(TOKEN token) : token(token) {}

  TOKEN get_token() { return token; }
  std::optional<std::string> get_value() { return value; }
};

void print_token(TOKEN token);
std::string to_string(TOKEN token);
bool is_unary_op(TOKEN token);
bool is_binary_op(TOKEN token);
std::string char_to_esc(char c);
int get_binop_prec(TOKEN token);
std::string get_token_type(TOKEN token);
bool is_right_associative(TOKEN token);
bool is_type_specifier(TOKEN token);
bool is_storage_specifier(TOKEN token);
bool is_constant_or_identifier(TOKEN token);
bool is_numeric_constant(TOKEN token);
} // namespace token
} // namespace scarlet
