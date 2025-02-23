#include "common.hh"

namespace scarlet {
namespace parser {

void parser::parse_int(std::vector<token::Token> &tokens,
                       std::shared_ptr<ast::AST_factor_Node> &factor) {
  if (!success) {
    return;
  }
  if (tokens.empty()) {
    eof_error(token::TOKEN::CONSTANT);
    return;
  }
  expect(tokens[0].get_token(), token::TOKEN::CONSTANT);
  if (!success) {
    return;
  }
  MAKE_SHARED(ast::AST_int_Node, int_node);
  int_node->set_value(tokens[0].get_value().value());
  factor->set_int_node(std::move(int_node));
  tokens.erase(tokens.begin());
}

} // namespace parser
} // namespace scarlet
