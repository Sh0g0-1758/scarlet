#pragma once

#include <iostream>
#include <optional>
#include <string>

namespace scarlet {
namespace token {
enum class TOKEN {
  UNKNOWN,
  // KEYWORDS
  IDENTIFIER,
  CONSTANT,
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
  // GRAMMAR TOKENS
  OPEN_PARANTHESES,
  CLOSE_PARANTHESES,
  OPEN_BRACE,
  CLOSE_BRACE,
  SEMICOLON,
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
  QUESTION_MARK,
  COMMA,
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

enum class TOKEN_TYPE {
  KEYWORDS,
  IDENTIFIERS,
  CONSTANTS,
  OPERATORS,
  SPECIAL_SYMBOLS,
  PRE_DIRECTIVES,
  UNKNOWN
};

class Token {
private:
  TOKEN token;
  std::optional<std::string> value;
  TOKEN_TYPE token_type;

public:
  Token(TOKEN token, std::optional<std::string> value, TOKEN_TYPE token_type)
      : token(token), value(value), token_type(token_type) {}
  Token(TOKEN token, TOKEN_TYPE token_type)
      : token(token), token_type(token_type) {}

  TOKEN get_token() { return token; }
  std::string get_token_type() { return token_type; }
  std::optional<std::string> get_value() { return value; }
};

void print_token(TOKEN token);
std::string to_string(TOKEN token);
bool is_unary_op(TOKEN token);
bool is_binary_op(TOKEN token);
int get_binop_prec(TOKEN token);
} // namespace token
} // namespace scarlet
