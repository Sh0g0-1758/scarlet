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

void parser::parse_declaration(
    std::vector<token::Token> &tokens,
    std::shared_ptr<ast::AST_Declaration_Node> &declaration,
    bool atGlobalLevel) {
  bool isFuncDecl = false;
  int iter = 0;
  int num_identifiers = 0;
  while (tokens[iter].get_token() != token::TOKEN::SEMICOLON and
         iter < (int)tokens.size()) {
    // If we find an assignment token, then this is a variable declaration
    if (tokens[iter].get_token() == token::TOKEN::ASSIGNMENT) {
      break;
    }

    // If we find more than one identifier, then this is a function declaration
    if (tokens[iter].get_token() == token::TOKEN::IDENTIFIER or
        tokens[iter].get_token() == token::TOKEN::VOID) {
      num_identifiers++;
      if (num_identifiers > 1) {
        isFuncDecl = true;
        break;
      }
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

void parser::parse_declarator(
    std::vector<token::Token> &tokens,
    std::shared_ptr<ast::AST_declarator_Node> &declarator,
    std::shared_ptr<ast::AST_identifier_Node> &identifier) {
  if (tokens[0].get_token() == token::TOKEN::IDENTIFIER) {
    identifier->set_identifier(tokens[0].get_value().value());
    tokens.erase(tokens.begin());
  } else if (tokens[0].get_token() == token::TOKEN::OPEN_PARANTHESES) {
    tokens.erase(tokens.begin());
    MAKE_SHARED(ast::AST_declarator_Node, child);
    parse_declarator(tokens, child, identifier);
    declarator->set_child(std::move(child));
    EXPECT(token::TOKEN::CLOSE_PARANTHESES);
  } else if (tokens[0].get_token() == token::TOKEN::ASTERISK) {
    tokens.erase(tokens.begin());
    declarator->set_pointer(true);
    MAKE_SHARED(ast::AST_declarator_Node, child);
    parse_declarator(tokens, child, identifier);
    declarator->set_child(std::move(child));
  }
}

void parser::parse_variable_declaration(
    std::vector<token::Token> &tokens,
    std::shared_ptr<ast::AST_variable_declaration_Node> decl) {
  PARSE_TYPE(decl, set_type);
  if (decl->get_specifier() == ast::SpecifierType::NONE)
    PARSE_SPECIFIER(decl);
  MAKE_SHARED(ast::AST_declarator_Node, declarator);
  MAKE_SHARED(ast::AST_identifier_Node, identifier);
  parse_declarator(tokens, declarator, identifier);
  if (identifier->get_value().empty()) {
    success = false;
    error_messages.emplace_back(
        "Identifier is necessary to declare a variable");
  }
  decl->set_identifier(std::move(identifier));
  decl->set_declarator(std::move(declarator));
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
  MAKE_SHARED(ast::AST_declarator_Node, declarator);
  MAKE_SHARED(ast::AST_identifier_Node, identifier);
  parse_declarator(tokens, declarator, identifier);
  if (identifier->get_value().empty()) {
    success = false;
    error_messages.emplace_back(
        "Identifier is necessary to declare a variable");
  }
  decl->set_identifier(std::move(identifier));
  decl->set_declarator(std::move(declarator));
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

  MAKE_SHARED(ast::AST_declarator_Node, declarator);
  MAKE_SHARED(ast::AST_identifier_Node, identifier);
  parse_declarator(tokens, declarator, identifier);
  if (identifier->get_value().empty()) {
    success = false;
    error_messages.emplace_back(
        "Identifier is necessary to declare a variable");
  }
  param->identifier = std::move(identifier);
  param->declarator = std::move(declarator);

  decl->add_param(std::move(param));

  while (tokens[0].get_token() == token::TOKEN::COMMA) {
    tokens.erase(tokens.begin());
    MAKE_SHARED(ast::Param, param);
    PARSE_TYPE(param, set_type);
    MAKE_SHARED(ast::AST_declarator_Node, declarator);
    MAKE_SHARED(ast::AST_identifier_Node, identifier);
    parse_declarator(tokens, declarator, identifier);
    if (identifier->get_value().empty()) {
      success = false;
      error_messages.emplace_back(
          "Identifier is necessary to declare a variable");
    }
    param->identifier = std::move(identifier);
    param->declarator = std::move(declarator);
    decl->add_param(std::move(param));
  }
}

} // namespace parser
} // namespace scarlet
