#include "common.hh"
#include <climits>

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
  constant::Constant constant;
  constant::Value v;
  switch (tokens[0].get_token()) {
  case token::TOKEN::INT_CONSTANT: {
    long val{};
    try {
      val = std::stol(tokens[0].get_value().value());
    } catch (std::out_of_range &e) {
      success = false;
      error_messages.emplace_back(
          std::to_string(val) + " is too large to represent as an int or long");
      return;
    }
    if (val > INT_MAX) {
      // implicit promotion to long
      constant.set_type(constant::Type::LONG);
      v.l = val;
    } else {
      constant.set_type(constant::Type::INT);
      v.i = val;
    }
  } break;
  case token::TOKEN::UINT_CONSTANT:
    constant.set_type(constant::Type::UINT);
    try {
      unsigned long ul = std::stoul(tokens[0].get_value().value());
      if (ul > UINT_MAX) {
        // implict promotion to unsigned long
        constant.set_type(constant::Type::ULONG);
        v.ul = ul;
      } else {
        v.ui = ul;
      }
    } catch (std::out_of_range &e) {
      success = false;
      error_messages.emplace_back("Unsigned int constant out of range");
      return;
    }
    break;
  case token::TOKEN::LONG_CONSTANT: {
    constant.set_type(constant::Type::LONG);
    try {
      v.l = std::stol(tokens[0].get_value().value());
    } catch (std::out_of_range &e) {
      success = false;
      error_messages.emplace_back("Long constant out of range");
      return;
    }
  } break;
  case token::TOKEN::ULONG_CONSTANT:
    constant.set_type(constant::Type::ULONG);
    try {
      v.ul = std::stoul(tokens[0].get_value().value());
    } catch (std::out_of_range &e) {
      success = false;
      error_messages.emplace_back("Unsigned long constant out of range");
      return;
    }
    break;
  default:
    break;
  }
  constant.set_value(v);
  const_node->set_constant(constant);
  factor->set_const_node(std::move(const_node));
  tokens.erase(tokens.begin());
}

} // namespace parser
} // namespace scarlet
