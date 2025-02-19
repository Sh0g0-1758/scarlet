#include "common.hh"

namespace scarlet {

namespace parser {

void parser::parse_declaration(
    std::vector<token::Token> &tokens,
    std::shared_ptr<ast::AST_Declaration_Node> &declaration) {
  EXPECT(token::TOKEN::INT);
  EXPECT_IDENTIFIER();
  declaration->set_identifier(std::move(identifier));
  if (tokens[0].get_token() == token::TOKEN::SEMICOLON) {
    // the variable just have a declaration
    tokens.erase(tokens.begin());
  } else {
    // the variable has a definition as well
    EXPECT(token::TOKEN::ASSIGNMENT);
    MAKE_SHARED(ast::AST_exp_Node, exp);
    parse_exp(tokens, exp);
    declaration->set_exp(std::move(exp));
    EXPECT(token::TOKEN::SEMICOLON);
  }
}

} // namespace parser

} // namespace scarlet
