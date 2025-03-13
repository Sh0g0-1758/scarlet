#include <parser/common.hh>

namespace scarlet {
namespace parser {

void parser::analyze_exp(
    std::shared_ptr<ast::AST_exp_Node> exp,
    std::map<std::pair<std::string, int>, symbolInfo> &symbol_table, int indx) {
  if (exp == nullptr)
    return;
  analyze_exp(exp->get_left(), symbol_table, indx);
  // here we check that the factor of the expresssion is not an undeclared
  // variable
  if (exp->get_factor_node() != nullptr and
      exp->get_factor_node()->get_identifier_node() != nullptr) {
    std::string var_name =
        exp->get_factor_node()->get_identifier_node()->get_value();
    int level = indx;
    bool found = false;
    while (level >= 0) {
      if (symbol_table.find({var_name, level}) != symbol_table.end()) {
        exp->get_factor_node()->get_identifier_node()->set_identifier(
            symbol_table[{var_name, level}].name);
        found = true;
        break;
      }
      level--;
    }
    if (!found) {
      success = false;
      error_messages.emplace_back("Variable " + var_name + " not declared");
    }
  }

  // now we check that if the exp is of type assignment, then factor is an
  // identifier
  if (exp->get_binop_node() != nullptr and
      (exp->get_binop_node()->get_op() == binop::BINOP::ASSIGN or
       binop::is_compound(exp->get_binop_node()->get_op()))) {
    // ERROR CONDITION: (no factor node) or (factor node is a constant, not a
    // variable) or (factor node is a variable but has unary operators) Here we
    // exploit the benefit of short circuiting power of the logical operator
    // this means that as we proceed, we are ensured that the earlier checks
    // must not be satisfied. Note that an identifier with unops makes it an
    // rvalue.
    if (exp->get_factor_node() == nullptr or
        exp->get_factor_node()->get_identifier_node() == nullptr or
        exp->get_factor_node()->get_unop_nodes().size() > 0) {
      success = false;
      error_messages.emplace_back("Expected a modifiable lvalue on the left "
                                  "side of the assignment operator");
    }
  }
  // SEMANTIC ANALYSIS FOR INCREMENT AND DECREMENT OPERATOR
  if (exp->get_factor_node() != nullptr) {
    auto check_factor = exp->get_factor_node();
    bool has_i_d = false;
    bool has_multiple_i_d = false;
    // CHECK WHETHER INCREMENT OR DECREMENT OPERATOR IS PRESENT
    for (auto it : check_factor->get_unop_nodes()) {
      if (it->get_op() == unop::UNOP::PREINCREMENT or
          it->get_op() == unop::UNOP::PREDECREMENT or
          it->get_op() == unop::UNOP::POSTINCREMENT or
          it->get_op() == unop::UNOP::POSTDECREMENT) {
        if (has_i_d) {
          has_multiple_i_d = true;
          break;
        }
        has_i_d = true;
      }
    }
    if (has_multiple_i_d) {
      success = false;
      error_messages.emplace_back(
          "Expected an lvalue for the increment / decrement operator");
    } else if (has_i_d) {
      // THE INCREMENT AND DECREMENT OPERATOR MUST BE APPLIED TO AN LVALUE
      // THAT MEANS IT SHOULD BE THE LAST UNOP IN THE UNOPS VECTOR
      if (check_factor->get_unop_nodes().back()->get_op() !=
              unop::UNOP::PREINCREMENT and
          check_factor->get_unop_nodes().back()->get_op() !=
              unop::UNOP::PREDECREMENT and
          check_factor->get_unop_nodes().back()->get_op() !=
              unop::UNOP::POSTINCREMENT and
          check_factor->get_unop_nodes().back()->get_op() !=
              unop::UNOP::POSTDECREMENT) {
        success = false;
        error_messages.emplace_back(
            "Expected an lvalue for the increment / decrement operator");
      } else {
        // NOW THERE ARE ONLY TWO VALID CASES
        // CASE 1: FACTOR HAS AN IDENTIFIER
        // CASE 2: FACTOR HAS A DEEPLY NESTED EXPRESSION WHICH THEN
        //         CONTAINS THE IDENTIFIER (eg. ++(((((((a))))))) )
        //         or ( (((((((a)))))))++ )
        if (check_factor->get_identifier_node() != nullptr) {
          // EARLIER CHECKS ENSURE THAT THE IDENTIFIER IS ALREADY DECLARED
        } else if (check_factor->get_exp_node() != nullptr) {
          // NOW WE RECURSIVELY CHECK THAT THE EXPRESSION IS A SIMPLE IDENTIFIER
          // AND NOT A COMPLEX EXPRESSION
          auto check_exp = check_factor->get_exp_node();
          while (check_exp != nullptr) {
            // ENSURE THAT BINOP FOR CHECK_EXP IS NULL
            // ENSURE THAT LEFT FOR CHECK_EXP IS NULL
            if (check_exp->get_binop_node() != nullptr or
                check_exp->get_left() != nullptr) {
              success = false;
              error_messages.emplace_back(
                  "Expected an lvalue for the increment / decrement operator");
              break;
            }
            // ENSURE THAT THERE ARE NO UNOPS AS WELL
            // WE ARE BASICALLY GUARANTEED THAT FACTOR IS NOT NULL
            if (check_exp->get_factor_node()->get_unop_nodes().size() > 0) {
              success = false;
              error_messages.emplace_back(
                  "Expected an lvalue for the increment / decrement operator");
              break;
            }
            // NOW WE CHECK THAT THE FACTOR IS AN IDENTIFIER
            if (exp->get_factor_node()->get_exp_node() == nullptr) {
              if (exp->get_factor_node()->get_identifier_node() == nullptr) {
                success = false;
                error_messages.emplace_back("Expected an lvalue for the "
                                            "increment / decrement operator");
                break;
              }
            }
            check_exp = check_exp->get_factor_node()->get_exp_node();
          }
        } else {
          success = false;
          error_messages.emplace_back(
              "Expected an lvalue for the increment / decrement operator");
        }
      }
    }
  }
  // since the factor can have its own exp as well, we recursively check that
  if (exp->get_factor_node() != nullptr)
    analyze_exp(exp->get_factor_node()->get_exp_node(), symbol_table, indx);
  // now we recursively check the right side of the expression
  if (exp->get_right() != nullptr)
    analyze_exp(exp->get_right(), symbol_table, indx);
  // if exp is a binary exp of ternary op, add a recursive check for the middle
  // expression
  if (exp->get_binop_node() != nullptr and
      exp->get_binop_node()->get_op() == binop::BINOP::TERNARY) {
    auto ternary = std::static_pointer_cast<ast::AST_ternary_exp_Node>(exp);
    analyze_exp(ternary->get_middle(), symbol_table, indx);
  }
}

} // namespace parser
} // namespace scarlet
