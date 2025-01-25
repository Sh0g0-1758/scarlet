#include "parser.hh"

#define EXPECT(tok)                                                            \
  if (!success) {                                                              \
    return;                                                                    \
  }                                                                            \
  if (tokens.empty()) {                                                        \
    eof_error(tok);                                                            \
    return;                                                                    \
  }                                                                            \
  expect(tokens[0].get_token(), tok);                                          \
  if (!success) {                                                              \
    return;                                                                    \
  }                                                                            \
  tokens.erase(tokens.begin());

#define EXPECT_FUNC(tok)                                                       \
  if (!success) {                                                              \
    return function;                                                           \
  }                                                                            \
  if (tokens.empty()) {                                                        \
    eof_error(tok);                                                            \
    return function;                                                           \
  }                                                                            \
  expect(tokens[0].get_token(), tok);                                          \
  if (!success) {                                                              \
    return function;                                                           \
  }                                                                            \
  tokens.erase(tokens.begin());

#define EXPECT_INT(tok)                                                        \
  if (!success) {                                                              \
    return;                                                                    \
  }                                                                            \
  if (tokens.empty()) {                                                        \
    eof_error(tok);                                                            \
    return;                                                                    \
  }                                                                            \
  expect(tokens[0].get_token(), tok);                                          \
  if (!success) {                                                              \
    return;                                                                    \
  }                                                                            \
  AST_int_Node int_node;                                                       \
  int_node.set_value(tokens[0].get_value().value());                           \
  exp.set_int_node(int_node);                                                  \
  tokens.erase(tokens.begin());

#define EXPECT_IDENTIFIER(tok)                                                 \
  if (!success) {                                                              \
    return;                                                                    \
  }                                                                            \
  if (tokens.empty()) {                                                        \
    eof_error(tok);                                                            \
    return;                                                                    \
  }                                                                            \
  expect(tokens[0].get_token(), tok);                                          \
  if (!success) {                                                              \
    return;                                                                    \
  }                                                                            \
  AST_identifier_Node identifier;                                              \
  identifier.set_identifier(tokens[0].get_value().value());                    \
  function.set_identifier(identifier);                                         \
  tokens.erase(tokens.begin());

void parser::parse_program(std::vector<Token> tokens) {
  AST_Program_Node program;
  while (!tokens.empty() and success) {
    AST_Function_Node func = parse_function(tokens);
    program.add_function(func);
  }
  this->program = program;
}

AST_Function_Node parser::parse_function(std::vector<Token> &tokens) {
  AST_Function_Node function;
  EXPECT_FUNC(TOKEN::INT);
  parse_identifier(tokens, function);
  EXPECT_FUNC(TOKEN::OPEN_PARANTHESES);
  EXPECT_FUNC(TOKEN::VOID);
  EXPECT_FUNC(TOKEN::CLOSE_PARANTHESES);
  EXPECT_FUNC(TOKEN::OPEN_BRACE);
  parse_statement(tokens, function);
  EXPECT_FUNC(TOKEN::CLOSE_BRACE);
  return function;
}

void parser::parse_statement(std::vector<Token> &tokens,
                             AST_Function_Node &function) {
  AST_Statement_Node statement("Return");
  EXPECT(TOKEN::RETURN);
  AST_exp_Node exp;
  parse_exp(tokens, exp);
  statement.add_exp(exp);
  EXPECT(TOKEN::SEMICOLON);
  function.add_statement(statement);
}

void parser::parse_exp(std::vector<Token> &tokens, AST_exp_Node &exp) {
  if (tokens[0].get_token() == TOKEN::CONSTANT) {
    parse_int(tokens, exp);
  } else if (tokens[0].get_token() == TOKEN::TILDE or
             tokens[0].get_token() == TOKEN::HYPHEN) {
    parse_unary_op(tokens, exp);
    parse_exp(tokens, exp);
  } else if (tokens[0].get_token() == TOKEN::OPEN_PARANTHESES) {
    tokens.erase(tokens.begin());
    parse_exp(tokens, exp);
    EXPECT(TOKEN::CLOSE_PARANTHESES);
  } else {
    success = false;
    error_messages.emplace_back(
        "Expected constant, unary operator or open parantheses but got " +
        to_string(tokens[0].get_token()));
  }
}

void parser::parse_unary_op(std::vector<Token> &tokens, AST_exp_Node &exp) {
  if (tokens[0].get_token() == TOKEN::TILDE) {
    AST_unop_Node unop;
    unop.set_op("Complement");
    exp.set_unop_node(unop);
    tokens.erase(tokens.begin());
  } else if (tokens[0].get_token() == TOKEN::HYPHEN) {
    AST_unop_Node unop;
    unop.set_op("Negate");
    exp.set_unop_node(unop);
    tokens.erase(tokens.begin());
  } else {
    success = false;
    error_messages.emplace_back("Expected unary operator but got " +
                                to_string(tokens[0].get_token()));
  }
}

void parser::parse_identifier(std::vector<Token> &tokens,
                              AST_Function_Node &function) {
  EXPECT_IDENTIFIER(TOKEN::IDENTIFIER);
}

void parser::parse_int(std::vector<Token> &tokens, AST_exp_Node &exp) {
  EXPECT_INT(TOKEN::CONSTANT);
}

void parser::expect(TOKEN actual_token, TOKEN expected_token) {
  if (actual_token != expected_token) {
    success = false;
    error_messages.emplace_back("Expected token " + to_string(expected_token) +
                                " but got " + to_string(actual_token));
  }
}

void parser::display_errors() {
  for (auto error : error_messages) {
    std::cerr << error << std::endl;
  }
}

void parser::eof_error(Token token) {
  success = false;
  error_messages.emplace_back("Expected " + to_string(token.get_token()) +
                              " but got end of file");
}

void parser::pretty_print() {
  std::cerr << "Program(" << std::endl;
  for (auto function : program.get_functions()) {
    std::cerr << "\tFunction(" << std::endl;
    std::cerr << "\t\tname=\"" << function.get_identifier().get_value() << "\","
              << std::endl;
    std::cerr << "\t\tbody=[" << std::endl;
    for (auto statement : function.get_statements()) {
      std::cerr << "\t\t\t" << statement.get_type() << "(" << std::endl;
      for (auto exp : statement.get_exps()) {
        for (auto unop : exp.get_unop_nodes()) {
          std::cerr << "\t\t\t\tUnop(" << unop.get_op() << ")" << std::endl;
        }
        std::cerr << "\t\t\t\t" << exp.get_int_node().get_AST_name() << "("
                  << exp.get_int_node().get_value() << "))" << std::endl;
      }
      std::cerr << "\t\t\t)," << std::endl;
    }
    std::cerr << "\t\t]" << std::endl;
    std::cerr << "\t)," << std::endl;
  }
  std::cerr << ")" << std::endl;
}
