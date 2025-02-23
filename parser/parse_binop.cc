#include "common.hh"

namespace scarlet {
namespace parser {

void parser::parse_binop(std::vector<token::Token> &tokens,
                         std::shared_ptr<ast::AST_binop_Node> &binop) {
  switch (tokens[0].get_token()) {
  case token::TOKEN::PLUS:
    binop->set_op(binop::BINOP::ADD);
    break;
  case token::TOKEN::PERCENT_SIGN:
    binop->set_op(binop::BINOP::MOD);
    break;
  case token::TOKEN::FORWARD_SLASH:
    binop->set_op(binop::BINOP::DIV);
    break;
  case token::TOKEN::ASTERISK:
    binop->set_op(binop::BINOP::MUL);
    break;
  case token::TOKEN::HYPHEN:
    binop->set_op(binop::BINOP::SUB);
    break;
  case token::TOKEN::AAND:
    binop->set_op(binop::BINOP::AAND);
    break;
  case token::TOKEN::AOR:
    binop->set_op(binop::BINOP::AOR);
    break;
  case token::TOKEN::XOR:
    binop->set_op(binop::BINOP::XOR);
    break;
  case token::TOKEN::LEFT_SHIFT:
    binop->set_op(binop::BINOP::LEFT_SHIFT);
    break;
  case token::TOKEN::RIGHT_SHIFT:
    binop->set_op(binop::BINOP::RIGHT_SHIFT);
    break;
  case token::TOKEN::LAND:
    binop->set_op(binop::BINOP::LAND);
    break;
  case token::TOKEN::LOR:
    binop->set_op(binop::BINOP::LOR);
    break;
  case token::TOKEN::EQUAL:
    binop->set_op(binop::BINOP::EQUAL);
    break;
  case token::TOKEN::NOTEQUAL:
    binop->set_op(binop::BINOP::NOTEQUAL);
    break;
  case token::TOKEN::LESSTHAN:
    binop->set_op(binop::BINOP::LESSTHAN);
    break;
  case token::TOKEN::GREATERTHAN:
    binop->set_op(binop::BINOP::GREATERTHAN);
    break;
  case token::TOKEN::LESSTHANEQUAL:
    binop->set_op(binop::BINOP::LESSTHANEQUAL);
    break;
  case token::TOKEN::GREATERTHANEQUAL:
    binop->set_op(binop::BINOP::GREATERTHANEQUAL);
    break;
  case token::TOKEN::ASSIGNMENT:
    binop->set_op(binop::BINOP::ASSIGN);
    break;
  case token::TOKEN::COMPOUND_DIFFERENCE:
    binop->set_op(binop::BINOP::COMPOUND_DIFFERENCE);
    break;
  case token::TOKEN::COMPOUND_DIVISION:
    binop->set_op(binop::BINOP::COMPOUND_DIVISION);
    break;
  case token::TOKEN::COMPOUND_PRODUCT:
    binop->set_op(binop::BINOP::COMPOUND_PRODUCT);
    break;
  case token::TOKEN::COMPOUND_REMAINDER:
    binop->set_op(binop::BINOP::COMPOUND_REMAINDER);
    break;
  case token::TOKEN::COMPOUND_SUM:
    binop->set_op(binop::BINOP::COMPOUND_SUM);
    break;
  case token::TOKEN::COMPOUND_AND:
    binop->set_op(binop::BINOP::COMPOUND_AND);
    break;
  case token::TOKEN::COMPOUND_OR:
    binop->set_op(binop::BINOP::COMPOUND_OR);
    break;
  case token::TOKEN::COMPOUND_XOR:
    binop->set_op(binop::BINOP::COMPOUND_XOR);
    break;
  case token::TOKEN::COMPOUND_LEFTSHIFT:
    binop->set_op(binop::BINOP::COMPOUND_LEFTSHIFT);
    break;
  case token::TOKEN::COMPOUND_RIGHTSHIFT:
    binop->set_op(binop::BINOP::COMPOUND_RIGHTSHIFT);
    break;
  case token::TOKEN::QUESTION_MARK:
    binop->set_op(binop::BINOP::TERNARY);
    break;
  default:
    success = false;
    error_messages.emplace_back("Expected binary operator but got " +
                                token::to_string(tokens[0].get_token()));
    break;
  }
  tokens.erase(tokens.begin());
}

} // namespace parser
} // namespace scarlet
