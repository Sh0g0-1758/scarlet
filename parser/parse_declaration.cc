#include "common.hh"

namespace scarlet {
namespace parser {

#define PARSE_SPECIFIER(decl)                                                  \
  if (!tokens.empty() and                                                      \
      token::is_storage_specifier(tokens[0].get_token())) {                    \
    switch (tokens[0].get_token()) {                                           \
    case token::TOKEN::STATIC:                                                 \
      decl->set_specifier(ast::SpecifierType::STATIC);                         \
      break;                                                                   \
    case token::TOKEN::EXTERN:                                                 \
      decl->set_specifier(ast::SpecifierType::EXTERN);                         \
      break;                                                                   \
    default:                                                                   \
      break;                                                                   \
    }                                                                          \
    tokens.erase(tokens.begin());                                              \
  }

/*
-same specifier ->wrong
-empty -> wrong
-unsigned and signed -> wrong
-unsigned and long -> ulong
-IF unsigned -> uint
-IF long -> long
-default int
*/
#define PARSE_TYPE(decl, func)                                                 \
  std::set<token::TOKEN> type_specifiers;                                      \
  std::set<token::TOKEN> storage_specifiers;                                   \
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
    } else {                                                                   \
      decl->func(ast::ElemType::INT);                                          \
    }                                                                          \
  }

void parser::parse_declaration(
    std::vector<token::Token> &tokens,
    std::shared_ptr<ast::AST_Declaration_Node> &declaration,
    bool atGlobalLevel) {
  bool isFuncDecl = false;
  int iter = 0;
  while (tokens[iter].get_token() != token::TOKEN::SEMICOLON and
         iter < (int)tokens.size()) {
    // If we find an assignment token, then this is a variable declaration
    if (tokens[iter].get_token() == token::TOKEN::ASSIGNMENT) {
      break;
    }
    // If we find an open parantheses, then this is a function declaration
    if (tokens[iter].get_token() == token::TOKEN::OPEN_PARANTHESES) {
      isFuncDecl = true;
      break;
    }
    iter++;
  }
  if (isFuncDecl) {
    MAKE_SHARED(ast::AST_function_declaration_Node, decl);
    PARSE_SPECIFIER(decl);
    parse_function_declaration(tokens, decl, atGlobalLevel);
    declaration = std::static_pointer_cast<ast::AST_Declaration_Node>(decl);
    declaration->set_type(ast::DeclarationType::FUNCTION);
  } else {
    MAKE_SHARED(ast::AST_variable_declaration_Node, decl);
    PARSE_SPECIFIER(decl);
    parse_variable_declaration(tokens, decl);
    declaration = std::static_pointer_cast<ast::AST_Declaration_Node>(decl);
    declaration->set_type(ast::DeclarationType::VARIABLE);
  }
}

void parser::parse_variable_declaration(
    std::vector<token::Token> &tokens,
    std::shared_ptr<ast::AST_variable_declaration_Node> decl) {
  PARSE_TYPE(decl, set_type);
  if (decl->get_specifier() == ast::SpecifierType::NONE)
    PARSE_SPECIFIER(decl);
  EXPECT_IDENTIFIER();
  decl->set_identifier(std::move(identifier));
  if (tokens[0].get_token() == token::TOKEN::SEMICOLON) {
    tokens.erase(tokens.begin());
    return;
  }
  EXPECT(token::TOKEN::ASSIGNMENT);
  MAKE_SHARED(ast::AST_exp_Node, exp);
  parse_exp(tokens, exp);
  decl->set_exp(std::move(exp));
  EXPECT(token::TOKEN::SEMICOLON);
}

void parser::parse_function_declaration(
    std::vector<token::Token> &tokens,
    std::shared_ptr<ast::AST_function_declaration_Node> decl,
    bool atGlobalLevel) {
  PARSE_TYPE(decl, set_return_type);
  if (decl->get_specifier() == ast::SpecifierType::NONE)
    PARSE_SPECIFIER(decl);
  EXPECT_IDENTIFIER();
  decl->set_identifier(std::move(identifier));
  EXPECT(token::TOKEN::OPEN_PARANTHESES);
  parse_param_list(tokens, decl);
  EXPECT(token::TOKEN::CLOSE_PARANTHESES);
  if (tokens[0].get_token() == token::TOKEN::SEMICOLON) {
    tokens.erase(tokens.begin());
    return;
  } else if (atGlobalLevel) {
    MAKE_SHARED(ast::AST_Block_Node, block);
    currFuncName = decl->get_identifier()->get_value();
    parse_block(tokens, block);
    decl->set_block(std::move(block));
  } else {
    EXPECT(token::TOKEN::SEMICOLON);
  }
}

void parser::parse_param_list(
    std::vector<token::Token> &tokens,
    std::shared_ptr<ast::AST_function_declaration_Node> decl) {
  if (tokens[0].get_token() == token::TOKEN::VOID) {
    tokens.erase(tokens.begin());
    return;
  }
  MAKE_SHARED(ast::Param, param);

  PARSE_TYPE(param, set_type);

  EXPECT_IDENTIFIER();
  param->identifier = std::move(identifier);
  decl->add_param(std::move(param));

  while (tokens[0].get_token() == token::TOKEN::COMMA) {
    tokens.erase(tokens.begin());
    MAKE_SHARED(ast::Param, param);
    PARSE_TYPE(param, set_type);
    EXPECT_IDENTIFIER();
    param->identifier = std::move(identifier);
    decl->add_param(std::move(param));
  }
}

} // namespace parser
} // namespace scarlet
