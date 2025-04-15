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

/*
- Error: Multiple identical type specifiers in the same declaration are invalid
- Error: Declarations with no type specifiers are considered invalid
- Error: 'unsigned' and 'signed' qualifiers cannot be used together
- Error: 'double' cannot occur with any other type specifier
- Error: 'char' cannot occur with int/long/double
- Note: Type qualifiers (signed/unsigned/int/long/char) may appear in any
sequence
*/
#define PARSE_TYPE(decl, func)                                                 \
  std::set<token::TOKEN> type_specifiers;                                      \
  while (!tokens.empty() and                                                   \
         token::is_type_specifier(tokens[0].get_token())) {                    \
    if (type_specifiers.count(tokens[0].get_token())) {                        \
      success = false;                                                         \
      error_messages.emplace_back("Multiple same type specifiers found");      \
    } else {                                                                   \
      type_specifiers.insert(tokens[0].get_token());                           \
    }                                                                          \
    tokens.erase(tokens.begin());                                              \
  }                                                                            \
  if (type_specifiers.empty()) {                                               \
    success = false;                                                           \
    error_messages.emplace_back("No type specifiers found");                   \
  } else if (type_specifiers.count(token::TOKEN::UNSIGNED) and                 \
             type_specifiers.count(token::TOKEN::SIGNED)) {                    \
    success = false;                                                           \
    error_messages.emplace_back(                                               \
        "Unsigned and signed specifiers found together");                      \
  }                                                                            \
  if (type_specifiers.find(token::TOKEN::VOID) != type_specifiers.end()) {     \
    if (type_specifiers.size() == 1) {                                         \
      decl->func(ast::ElemType::VOID);                                         \
    } else {                                                                   \
      success = false;                                                         \
      error_messages.emplace_back("Void specifier found with other types");    \
    }                                                                          \
  } else if (type_specifiers.count(token::TOKEN::DOUBLE) and                   \
             type_specifiers.size() > 1) {                                     \
    success = false;                                                           \
    error_messages.emplace_back("Double specifier found with other types");    \
  } else if (type_specifiers.count(token::TOKEN::CHAR) and                     \
             (type_specifiers.count(token::TOKEN::INT) or                      \
              type_specifiers.count(token::TOKEN::LONG) or                     \
              type_specifiers.count(token::TOKEN::DOUBLE))) {                  \
    success = false;                                                           \
    error_messages.emplace_back("Char specifier found with other types");      \
  } else {                                                                     \
    if (type_specifiers.count(token::TOKEN::DOUBLE)) {                         \
      decl->func(ast::ElemType::DOUBLE);                                       \
    } else if (type_specifiers.count(token::TOKEN::UNSIGNED)) {                \
      if (type_specifiers.count(token::TOKEN::LONG)) {                         \
        decl->func(ast::ElemType::ULONG);                                      \
      } else if (type_specifiers.count(token::TOKEN::CHAR)) {                  \
        decl->func(ast::ElemType::UCHAR);                                      \
      } else {                                                                 \
        decl->func(ast::ElemType::UINT);                                       \
      }                                                                        \
    } else {                                                                   \
      if (type_specifiers.count(token::TOKEN::LONG)) {                         \
        decl->func(ast::ElemType::LONG);                                       \
      } else if (type_specifiers.count(token::TOKEN::CHAR)) {                  \
        decl->func(ast::ElemType::CHAR);                                       \
      } else {                                                                 \
        decl->func(ast::ElemType::INT);                                        \
      }                                                                        \
    }                                                                          \
  }

} // namespace parser
} // namespace scarlet
