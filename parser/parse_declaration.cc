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

void parser::parse_simple_declarator(
    std::vector<token::Token> &tokens,
    std::shared_ptr<ast::AST_simple_declarator_Node> &simple_declarator) {
    if(tokens[0].get_token() == token::TOKEN::IDENTIFIER) {
        EXPECT_IDENTIFIER();
        simple_declarator->set_identifier(std::move(identifier));
    } else {
        EXPECT(token::TOKEN::OPEN_PARANTHESES);
        MAKE_SHARED(ast::AST_declarator_Node, declarator);
        parse_declarator(tokens, declarator);
        simple_declarator->set_declarator(std::move(declarator));
        EXPECT(token::TOKEN::CLOSE_PARANTHESES);
    }
}

void parser::parse_param_list(
    std::vector<token::Token> &tokens,
    std::shared_ptr<ast::AST_param_list_Node> &param_list) {
    if(tokens[0].get_token() == token::TOKEN::VOID) {
        tokens.erase(tokens.begin());
        return;
    }
    MAKE_SHARED(ast::Param, param);
    PARSE_TYPE(param, set_type);
    MAKE_SHARED(ast::AST_declarator_Node, declarator);
    parse_declarator(tokens, declarator);
    param->set_declarator(std::move(declarator));
    param_list->add_param(std::move(param));
    while(tokens[0].get_token() == token::TOKEN::COMMA) {
        tokens.erase(tokens.begin());
        MAKE_SHARED(ast::Param, param);
        PARSE_TYPE(param, set_type);
        MAKE_SHARED(ast::AST_declarator_Node, declarator);
        parse_declarator(tokens, declarator);
        param->set_declarator(std::move(declarator));
        param_list->add_param(std::move(param));
    }
}

void parser::parse_direct_declarator(
    std::vector<token::Token> &tokens,
    std::shared_ptr<ast::AST_direct_declarator_Node> &direct_declarator) {

    MAKE_SHARED(ast::AST_simple_declarator_Node, simple_declarator);
    parse_simple_declarator(tokens, simple_declarator);
    direct_declarator->set_simple_declarator(std::move(simple_declarator));
    if(tokens[0].get_token() == token::TOKEN::OPEN_PARANTHESES) {
        tokens.erase(tokens.begin());
        MAKE_SHARED(ast::AST_param_list_Node, param_list);
        parse_param_list(tokens, param_list);
        direct_declarator->set_param_list(std::move(param_list));
        EXPECT(token::TOKEN::CLOSE_PARANTHESES);
    }
}
void parser::parse_declarator(std::vector<token::Token> &tokens,
                              std::shared_ptr<ast::AST_declarator_Node> &decl) {
  if (tokens[0].get_token() == token::TOKEN::ASTERISK) {
    tokens.erase(tokens.begin());
    MAKE_SHARED(ast::AST_declarator_Node, child);
    parse_declarator(tokens, child);
    decl->set_child(std::move(child));
  }
  else
  {
    MAKE_SHARED(ast::AST_direct_declarator_Node, direct_declarator);
    parse_direct_declarator(tokens, direct_declarator);
    decl->set_direct_declarator(std::move(direct_declarator));
  }
}
bool parser::isFuncDecl(std::shared_ptr<ast::AST_declarator_Node> decl) {
  if (decl->get_child() != nullptr) {
    return isFuncDecl(decl->get_child());  
  } else {
    std::shared_ptr<ast::AST_direct_declarator_Node> direct_declarator = decl->get_direct_declarator();
    if(direct_declarator->get_param_list() != nullptr) {
      return true;
    }
  }
  return false;
}

void parser::parse_declaration(
    std::vector<token::Token> &tokens,
    std::shared_ptr<ast::AST_Declaration_Node> &decl,
    bool atGlobalLevel) {
  PARSE_SPECIFIER(decl);  
  PARSE_TYPE(decl, set_base_type);
  if(decl->get_specifier() == ast::SpecifierType::NONE) {
   PARSE_SPECIFIER(decl);
  }
  MAKE_SHARED(ast::AST_declarator_Node, declarator);
  parse_declarator(tokens, declarator);
  if (isFuncDecl(declarator)) {
    MAKE_SHARED(ast::AST_function_declaration_Node, func_decl);
    func_decl->set_declarator(std::move(declarator));
    //TO-DO: set return type of function after processing declarator
    parse_function_declaration(tokens, func_decl, atGlobalLevel);
    decl = std::static_pointer_cast<ast::AST_Declaration_Node>(func_decl);
    decl->set_type(ast::DeclarationType::FUNCTION);
  } else {
    MAKE_SHARED(ast::AST_variable_declaration_Node, var_decl);
    var_decl->set_declarator(std::move(declarator));
    //TO-DO: set type of variable after processing declarator
    parse_variable_declaration(tokens, var_decl);
    decl = std::static_pointer_cast<ast::AST_Declaration_Node>(var_decl);
    decl->set_type(ast::DeclarationType::VARIABLE);
  }
}

void parser::parse_variable_declaration(
    std::vector<token::Token> &tokens,
    std::shared_ptr<ast::AST_variable_declaration_Node> decl) {
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

  if (tokens[0].get_token() == token::TOKEN::SEMICOLON) {
    tokens.erase(tokens.begin());
    return;
  } else if (atGlobalLevel) {
    MAKE_SHARED(ast::AST_Block_Node, block);
    //currFuncName = decl->get_identifier()->get_value();
    parse_block(tokens, block);
    decl->set_block(std::move(block));
  } else {
    EXPECT(token::TOKEN::SEMICOLON);
  }
}



} // namespace parser
} // namespace scarlet
