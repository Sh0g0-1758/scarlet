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
- Error: 'char' cannot occur with any other type specifier
- Type resolution: When 'unsigned' and 'long' appear together -> ulong
- Type resolution: When only 'unsigned' appears -> uint
- Type resolution: When only 'long' appears -> long, otherwise defaults to int
- Note: Type qualifiers (signed/unsigned/int/long) may appear in any sequence
*/
#define PARSE_TYPE(decl, func)                                                 \
  std::set<token::TOKEN> type_specifiers;                                      \
  while (!tokens.empty() and                                                   \
         token::is_type_specifier(tokens[0].get_token())) {                    \
    if (type_specifiers.find(tokens[0].get_token()) !=                         \
        type_specifiers.end()) {                                               \
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
  } else if (type_specifiers.find(token::TOKEN::UNSIGNED) !=                   \
                 type_specifiers.end() and                                     \
             type_specifiers.find(token::TOKEN::SIGNED) !=                     \
                 type_specifiers.end()) {                                      \
    success = false;                                                           \
    error_messages.emplace_back(                                               \
        "Unsigned and signed specifiers found together");                      \
  } else if(type_specifiers.find(token::TOKEN::CHAR) != type_specifiers.end()) { \
      if(type_specifiers.find(token::TOKEN::LONG) != type_specifiers.end() or  \
        type_specifiers.find(token::TOKEN::INT) != type_specifiers.end() or    \
        type_specifiers.find(token::TOKEN::DOUBLE) != type_specifiers.end()) { \
         success = false;                                                      \
         error_messages.emplace_back("Char specifier found with other types"); \
        }                                                                      \
      else {                                                                   \
        if(type_specifiers.find(token::TOKEN::UNSIGNED) !=                     \
            type_specifiers.end()) {                                           \
          decl->func(ast::ElemType::UCHAR);                                    \
        } else if (type_specifiers.find(token::TOKEN::SIGNED) != type_specifiers.end()) {\
          decl->func(ast::ElemType::SCHAR);                                    \
        } else {                                                               \
          decl->func(ast::ElemType::CHAR);                                     \
        }                                                                      \
  }                                                                            \
}                                                                              \
  else if (type_specifiers.find(token::TOKEN::DOUBLE) !=                       \
                 type_specifiers.end() and                                     \
             type_specifiers.size() > 1) {                                     \
    success = false;                                                           \
    error_messages.emplace_back("Double specifier found with other types");    \
  } else {                                                                     \
    if (type_specifiers.find(token::TOKEN::UNSIGNED) !=                        \
            type_specifiers.end() and                                          \
        type_specifiers.find(token::TOKEN::LONG) != type_specifiers.end()) {   \
      decl->func(ast::ElemType::ULONG);                                        \
    } else if (type_specifiers.find(token::TOKEN::UNSIGNED) !=                 \
               type_specifiers.end()) {                                        \
      decl->func(ast::ElemType::UINT);                                         \
    } else if (type_specifiers.find(token::TOKEN::LONG) !=                     \
               type_specifiers.end()) {                                        \
      decl->func(ast::ElemType::LONG);                                         \
    } else if (type_specifiers.find(token::TOKEN::DOUBLE) !=                   \
               type_specifiers.end()) {                                        \
      decl->func(ast::ElemType::DOUBLE);                                       \
    } else {                                                                   \
      decl->func(ast::ElemType::INT);                                          \
    }                                                                          \
  }
} // namespace parser
} // namespace scarlet
