#include "common.hh"

namespace scarlet {

namespace parser {

void parser::parse_identifier(
    std::vector<token::Token> &tokens,
    std::shared_ptr<ast::AST_Function_Node> &function) {
  EXPECT_IDENTIFIER();
  function->set_identifier(std::move(identifier));
}

void parser::parse_int(std::vector<token::Token> &tokens,
                       std::shared_ptr<ast::AST_factor_Node> &factor) {
  EXPECT_INT(token::TOKEN::CONSTANT);
}

} // namespace parser

} // namespace scarlet
