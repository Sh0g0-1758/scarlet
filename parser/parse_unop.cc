#include "common.hh"

namespace scarlet {

namespace parser {

void parser::parse_unary_op(std::vector<token::Token> &tokens,
                            std::shared_ptr<ast::AST_factor_Node> &factor) {
  MAKE_SHARED(ast::AST_unop_Node, unop);
  switch (tokens[0].get_token()) {
  case token::TOKEN::TILDE:
    unop->set_op(unop::UNOP::COMPLEMENT);
    break;
  case token::TOKEN::HYPHEN:
    unop->set_op(unop::UNOP::NEGATE);
    break;
  case token::TOKEN::NOT:
    unop->set_op(unop::UNOP::NOT);
    break;
  case token::TOKEN::INCREMENT_OPERATOR:
    unop->set_op(unop::UNOP::PREINCREMENT);
    break;
  case token::TOKEN::DECREMENT_OPERATOR:
    unop->set_op(unop::UNOP::PREDECREMENT);
    break;
  default:
    UNREACHABLE()
  }
  factor->add_unop_node(std::move(unop));
  tokens.erase(tokens.begin());
}

} // namespace parser

} // namespace scarlet
