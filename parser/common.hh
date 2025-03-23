#pragma once

#include "parser.hh"
#include <tools/constant/constant.hh>
#include <tools/macros/macros.hh>

namespace scarlet {
namespace parser {

#define EXPISCONSTANT(exp)                                                     \
  (exp->get_binop_node() == nullptr and exp->get_left() == nullptr and         \
   exp->get_right() == nullptr and exp->get_factor_node() != nullptr and       \
   exp->get_factor_node()->get_const_node() != nullptr and                     \
   exp->get_factor_node()->get_unop_node() == nullptr and                      \
   exp->get_factor_node()->get_identifier_node() == nullptr and                \
   exp->get_factor_node()->get_exp_node() == nullptr)

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

#define EXPECT_IDENTIFIER()                                                    \
  if (!success) {                                                              \
    return;                                                                    \
  }                                                                            \
  if (tokens.empty()) {                                                        \
    eof_error(token::TOKEN::IDENTIFIER);                                       \
    return;                                                                    \
  }                                                                            \
  expect(tokens[0].get_token(), token::TOKEN::IDENTIFIER);                     \
  if (!success) {                                                              \
    return;                                                                    \
  }                                                                            \
  MAKE_SHARED(ast::AST_identifier_Node, identifier);                           \
  identifier->set_identifier(tokens[0].get_value().value());                   \
  tokens.erase(tokens.begin());

} // namespace parser
} // namespace scarlet
