#pragma once

#include <iostream>
#include <optional>
#include <string>

namespace scarlet {
namespace token {
enum class TOKEN {
  UNKNOWN,
  // IDENTIFIER
  IDENTIFIER,
  // CONSTANTS
  CONSTANT,
  CHARACTER_CONSTANT,
  CHAR_ARR,
  // KEYWORDS
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
  CHAR,
  SIZEOF,
  STRUCT,
  // GRAMMAR TOKENS
  OPEN_PARANTHESES,
  CLOSE_PARANTHESES,
  OPEN_BRACE,
  CLOSE_BRACE,
  SEMICOLON,
  OPEN_BRACKET,
  CLOSE_BRACKET,
  // UNARY OPERATORS
  TILDE,
  HYPHEN,
  DECREMENT_OPERATOR,
  // BINARY OPERATORS
  PLUS,
  ASTERISK,
  FORWARD_SLASH,
  PERCENT_SIGN,
  ASSIGNMENT,
  COLON,
  DOT,
  QUESTION_MARK,
  COMMA,
  ARROW_OPERATOR,
  // BITWISE OPERATORS
  AAND,
  AOR,
  XOR,
  LEFT_SHIFT,
  RIGHT_SHIFT,
  // LOGICAL OPERATORS
  NOT,
  LAND,
  LOR,
  EQUAL,
  NOTEQUAL,
  LESSTHAN,
  GREATERTHAN,
  LESSTHANEQUAL,
  GREATERTHANEQUAL
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
} // namespace token
} // namespace scarlet
