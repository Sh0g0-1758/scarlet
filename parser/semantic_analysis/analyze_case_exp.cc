#include <parser/common.hh>

namespace scarlet {
namespace parser {

int parser::analyze_case_exp(
    std::shared_ptr<ast::AST_exp_Node> exp,
    std::map<std::pair<std::string, int>, std::string> &symbol_table,
    int indx) {
  if (exp == nullptr)
    return 0;
  int left = -1;
  int right = -1;
  int middle = -1;
  if (exp->get_left() != nullptr) {
    left = analyze_case_exp(exp->get_left(), symbol_table, indx);
  }
  // here we check that the factor of the expresssion is not a variable
  if (exp->get_factor_node() != nullptr and
      exp->get_factor_node()->get_identifier_node() != nullptr) {
    success = false;
    error_messages.emplace_back(
        "case expression is not an integer constant expression");
  } else if (exp->get_factor_node() != nullptr) {
    // the factor base can either be an int or an expression
    if (exp->get_factor_node()->get_int_node() != nullptr)
      left = std::stoi(exp->get_factor_node()->get_int_node()->get_value());
    else if (exp->get_factor_node()->get_exp_node() != nullptr)
      left = analyze_case_exp(exp->get_factor_node()->get_exp_node(),
                              symbol_table, indx);

    for (auto unop : exp->get_factor_node()->get_unop_nodes()) {
      switch (unop->get_op()) {
      case unop::UNOP::COMPLEMENT:
        left = ~left;
        break;
      case unop::UNOP::NEGATE:
        left = -left;
        break;
      case unop::UNOP::NOT:
        left = !left;
        break;
      case unop::UNOP::POSTINCREMENT:
      case unop::UNOP::POSTDECREMENT:
      case unop::UNOP::PREINCREMENT:
      case unop::UNOP::PREDECREMENT:
        success = false;
        error_messages.emplace_back(
            "constant expression uses post/pre increment/decrement operator");
        break;
      default:
        break;
      }
    }
  }

  // now we recursively check the right side of the expression
  if (exp->get_right() != nullptr)
    right = analyze_case_exp(exp->get_right(), symbol_table, indx);
  // and a recursive check for the middle expression -> special case(ternary
  // operator)
  if (exp->get_middle() != nullptr)
    middle = analyze_case_exp(exp->get_middle(), symbol_table, indx);
  if (exp->get_binop_node() == nullptr) {
    return left;
  }
  switch (exp->get_binop_node()->get_op()) {
  case binop::BINOP::ADD:
    return left + right;
  case binop::BINOP::SUB:
    return left - right;
  case binop::BINOP::MUL:
    return left * right;
  case binop::BINOP::DIV:
    return left / right;
  case binop::BINOP::MOD:
    return left % right;
  case binop::BINOP::XOR:
    return left ^ right;
  case binop::BINOP::AAND:
    return left & right;
  case binop::BINOP::AOR:
    return left | right;
  case binop::BINOP::LEFT_SHIFT:
    return left << right;
  case binop::BINOP::RIGHT_SHIFT:
    return left >> right;
  case binop::BINOP::EQUAL:
    return left == right;
  case binop::BINOP::NOTEQUAL:
    return left != right;
  case binop::BINOP::LESSTHAN:
    return left < right;
  case binop::BINOP::LESSTHANEQUAL:
    return left <= right;
  case binop::BINOP::GREATERTHAN:
    return left > right;
  case binop::BINOP::GREATERTHANEQUAL:
    return left >= right;
  case binop::BINOP::LAND:
    return left && right;
  case binop::BINOP::LOR:
    return left || right;
  case binop::BINOP::TERNARY:
    return left ? middle : right;
  case binop::BINOP::COMPOUND_SUM:
  case binop::BINOP::COMPOUND_DIFFERENCE:
  case binop::BINOP::COMPOUND_DIVISION:
  case binop::BINOP::COMPOUND_PRODUCT:
  case binop::BINOP::COMPOUND_REMAINDER:
  case binop::BINOP::COMPOUND_AND:
  case binop::BINOP::COMPOUND_OR:
  case binop::BINOP::COMPOUND_XOR:
  case binop::BINOP::COMPOUND_LEFTSHIFT:
  case binop::BINOP::COMPOUND_RIGHTSHIFT:
  case binop::BINOP::ASSIGN:
    success = false;
    error_messages.emplace_back(
        "case expression uses compound assignment operator");
    return 0;
  default:
    return 0;
  }
}

} // namespace parser
} // namespace scarlet
