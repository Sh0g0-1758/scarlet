#include "common.hh"

namespace scarlet {
namespace parser {

void parser::parse_declaration(
    std::vector<token::Token> &tokens,
    std::shared_ptr<ast::AST_Declaration_Node> &declaration,
    bool atGlobalLevel) {
  if (tokens.size() < 3) {
    success = false;
    error_messages.emplace_back("Invalid Declaration");
    return;
  }
  if (tokens[2].get_token() == token::TOKEN::OPEN_PARANTHESES) {
    MAKE_SHARED(ast::AST_function_declaration_Node, decl);
    parse_function_declaration(tokens, decl, atGlobalLevel);
    declaration = std::static_pointer_cast<ast::AST_Declaration_Node>(decl);
    declaration->set_type(ast::DeclarationType::FUNCTION);
  } else {
    MAKE_SHARED(ast::AST_variable_declaration_Node, decl);
    parse_variable_declaration(tokens, decl);
    declaration = std::static_pointer_cast<ast::AST_Declaration_Node>(decl);
    declaration->set_type(ast::DeclarationType::VARIABLE);
  }
}

void parser::parse_variable_declaration(
    std::vector<token::Token> &tokens,
    std::shared_ptr<ast::AST_variable_declaration_Node> decl) {
  EXPECT(token::TOKEN::INT);
  decl->set_type(ast::ElemType::INT);
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
  EXPECT(token::TOKEN::INT);
  decl->set_return_type(ast::ElemType::INT);
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

  EXPECT(token::TOKEN::INT);

  EXPECT_IDENTIFIER();
  MAKE_SHARED(ast::Param, param);
  param->type = ast::ElemType::INT;
  param->identifier = std::move(identifier);
  decl->add_param(std::move(param));

  while (tokens[0].get_token() == token::TOKEN::COMMA) {
    tokens.erase(tokens.begin());
    EXPECT(token::TOKEN::INT);
    EXPECT_IDENTIFIER();
    MAKE_SHARED(ast::Param, param);
    param->type = ast::ElemType::INT;
    param->identifier = std::move(identifier);
    decl->add_param(std::move(param));
  }
}

} // namespace parser
} // namespace scarlet
