#include "common.hh"

namespace scarlet {
namespace parser {

void parser::parse_const(std::vector<token::Token> &tokens,
                         std::shared_ptr<ast::AST_factor_Node> &factor) {
  if (!success) {
    return;
  }
  if (tokens.empty()) {
    eof_error(token::TOKEN::INT_CONSTANT);
    return;
  }
  MAKE_SHARED(ast::AST_const_Node, const_node);
  switch (tokens[0].get_token()) {
  case token::TOKEN::INT_CONSTANT: {
    expect(tokens[0].get_token(), token::TOKEN::INT_CONSTANT);
    if (!success)
      return;
    constant::Constant constant;
    constant.set_type(constant::Type::INT);
    constant::Value v;
    v.i = std::stoi(tokens[0].get_value().value());
    constant.set_value(v);
    const_node->set_constant(constant);
    factor->set_const_node(std::move(const_node));
    tokens.erase(tokens.begin());
  } break;
  case token::TOKEN::LONG_CONSTANT: {
    expect(tokens[0].get_token(), token::TOKEN::LONG_CONSTANT);
    if (!success)
      return;
    constant::Constant constant;
    constant.set_type(constant::Type::LONG);
    constant::Value v;
    v.l = std::stol(tokens[0].get_value().value());
    constant.set_value(v);
    const_node->set_constant(constant);
    factor->set_const_node(std::move(const_node));
    tokens.erase(tokens.begin());
  } break;
  default:
    break;
  }
}

} // namespace parser
} // namespace scarlet
