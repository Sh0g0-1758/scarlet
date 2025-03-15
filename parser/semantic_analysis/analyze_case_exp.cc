#include <parser/common.hh>

namespace scarlet {
namespace parser {

void parser::analyze_case_exp(
    std::shared_ptr<ast::AST_exp_Node> exp,
    std::map<std::pair<std::string, int>, std::string> &symbol_table,
    int indx) {
  if (exp == nullptr)
    return;
  analyze_case_exp(exp->get_left(), symbol_table, indx);
  // here we check that the factor of the expresssion is not an undeclared
  // variable
  if (exp->get_factor_node() != nullptr and
      exp->get_factor_node()->get_identifier_node() != nullptr) {
    success = false;
    error_messages.emplace_back(
        "case expression is not an integer constant expression");
  }
  // since the factor can have its own exp as well, we recursively check that
  if (exp->get_factor_node() != nullptr)
    analyze_case_exp(exp->get_factor_node()->get_exp_node(), symbol_table,
                     indx);
  // now we recursively check the right side of the expression
  if (exp->get_right() != nullptr)
    analyze_case_exp(exp->get_right(), symbol_table, indx);
  // and a recursive check for the middle expression -> special case(ternary
  // operator)
  if (exp->get_middle() != nullptr)
    analyze_case_exp(exp->get_middle(), symbol_table, indx);
}

} // namespace parser
} // namespace scarlet
