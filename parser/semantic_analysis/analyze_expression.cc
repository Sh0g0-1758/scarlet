#include <parser/common.hh>

namespace scarlet {
namespace parser {

void parser::analyze_exp(std::shared_ptr<ast::AST_exp_Node> exp,
                         std::map<std::pair<std::string, int>,
                                  symbolTable::symbolInfo> &symbol_table,
                         int indx) {
  if (exp == nullptr)
    return;
  analyze_exp(exp->get_left(), symbol_table, indx);

  // Check that if the exp is of type assignment, then factor is an identifier
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
        exp->get_factor_node()->get_unop_node() != nullptr) {
      success = false;
      error_messages.emplace_back("Expected a modifiable lvalue on the left "
                                  "side of the assignment operator");
    }
    if (exp->get_factor_node() != nullptr and
        exp->get_factor_node()->get_factor_type() ==
            ast::FactorType::FUNCTION_CALL) {
      success = false;
      error_messages.emplace_back("Invalid assignment to an rvalue");
    }
  }

  // check the factor
  analyze_factor(exp->get_factor_node(), symbol_table, indx);

  // Recursively check the right side of the expression
  analyze_exp(exp->get_right(), symbol_table, indx);

  // if exp is a binary exp of ternary op, add a recursive check for the middle
  // expression
  if (exp->get_binop_node() != nullptr and
      exp->get_binop_node()->get_op() == binop::BINOP::TERNARY) {
    auto ternary = std::static_pointer_cast<ast::AST_ternary_exp_Node>(exp);
    analyze_exp(ternary->get_middle(), symbol_table, indx);
  }
}

void parser::analyze_factor(std::shared_ptr<ast::AST_factor_Node> factor,
                            std::map<std::pair<std::string, int>,
                                     symbolTable::symbolInfo> &symbol_table,
                            int indx) {
  if (factor == nullptr)
    return;
  // here we check that the factor is not an undeclared
  // identifier and also do some type checking
  // In case of function call, we additionally check that the identifier being
  // called is a function declaration only.
  // We also check that when the factor is not a function call, it's not
  // using a function name as an identifier.
  if (factor->get_identifier_node() != nullptr) {
    bool isFuncCall =
        factor->get_factor_type() == ast::FactorType::FUNCTION_CALL;
    std::string var_name = factor->get_identifier_node()->get_value();
    int level = indx;
    bool found = false;
    std::string updatedIdentifierName;
    bool isfuncType = false;
    while (level >= 0) {
      if (symbol_table.find({var_name, level}) != symbol_table.end()) {
        updatedIdentifierName = symbol_table[{var_name, level}].name;
        isfuncType = symbol_table[{var_name, level}].type ==
                     symbolTable::symbolType::FUNCTION;
        found = true;
        break;
      }
      level--;
    }
    if (!found) {
      success = false;
      if (isFuncCall) {
        error_messages.emplace_back("Function " + var_name + " not declared");
      } else {
        error_messages.emplace_back("Variable " + var_name + " not declared");
      }
    } else {
      if (isFuncCall) {
        if (!isfuncType) {
          success = false;
          error_messages.emplace_back("Object " + var_name +
                                      " is not a function");
        }
      } else {
        if (isfuncType) {
          success = false;
          error_messages.emplace_back("Illegal use of function " + var_name);
        } else {
          factor->get_identifier_node()->set_identifier(updatedIdentifierName);
        }
      }
    }
  }

  // The factor can be a function call
  if (factor->get_factor_type() == ast::FactorType::FUNCTION_CALL) {
    auto func_call =
        std::static_pointer_cast<ast::AST_factor_function_call_Node>(factor);

    for (auto it : func_call->get_arguments()) {
      analyze_exp(it, symbol_table, indx);
    }

    // Check that the function is being called with the correct number of
    // arguments
    auto gstTypeDef =
        globalSymbolTable[func_call->get_identifier_node()->get_value()]
            .typeDef;
    if ((int)func_call->get_arguments().size() != (int)gstTypeDef.size() - 1) {
      success = false;
      error_messages.emplace_back(
          "Function " + func_call->get_identifier_node()->get_value() +
          " called with wrong number of arguments");
    } else {
      // check that the function is being called with the correct type of
      // arguments for(int i = 0; i < (int)func_call->get_arguments().size();
      // i++) {
      //   if(func_call->get_arguments()[i]->get_type() != gstTypeDef[i + 1]) {
      //     success = false;
      //     error_messages.emplace_back("Function " +
      //     func_call->get_identifier_node()->get_value() + " called with wrong
      //     type of arguments"); break;
      //   }
      // }
    }
  }

  // If factor has an increment/decrement operator
  // make sure it operates on an lvalue
  if (factor->get_unop_node() != nullptr and
      unop::is_incr_decr(factor->get_unop_node()->get_op())) {
    if (factor->get_child() == nullptr or
        factor->get_child()->get_identifier_node() == nullptr or
        factor->get_child()->get_factor_type() != ast::FactorType::BASIC) {
      success = false;
      error_messages.emplace_back(
          "Expected an lvalue for the increment / decrement operator");
    }
  }

  // Recursively check the factor child
  analyze_factor(factor->get_child(), symbol_table, indx);

  // since the factor can have its own exp as well, we recursively check that
  analyze_exp(factor->get_exp_node(), symbol_table, indx);
}

// void parser::assign_type_to_factor(std::shared_ptr<ast::AST_factor_Node>
// factor, std::map<std::pair<std::string, int>, symbolTable::symbolInfo>
// &symbol_table, int indx) {
// if (factor == nullptr)
//   return;
// if (factor->get_factor_type() == ast::FactorType::BASIC) {
//   if (factor->get_const_node() != nullptr) {
//     switch (factor->get_const_node()->get_constant().get_type()) {
//     case constant::Type::INT:
//       factor->set_type(ast::ElemType::INT);
//       break;
//     case constant::Type::LONG:
//       factor->set_type(ast::ElemType::LONG);
//       break;
//     }
//   } else if (factor->get_identifier_node() != nullptr) {
//     auto var_name = factor->get_identifier_node()->get_value();
//     int level = indx;
//     while (level >= 0) {
//       if (symbol_table.find({var_name, level}) != symbol_table.end()) {
//         factor->set_type(symbol_table[{var_name, level}].typeDef[0]);
//         break;
//       }
//       level--;
//     }
//   } else if(factor->get_cast_type() != ast::ElemType::NONE) {
//     assign_type_to_factor(factor->get_child(), symbol_table, indx);
//     factor->set_type(factor->get_cast_type());
//   } else if(factor->get_exp_node() != nullptr) {
//     assign_type_to_exp(factor->get_exp_node(), symbol_table, indx);
//     factor->set_type(factor->get_exp_node()->get_type());
//   }
// } else if (factor->get_factor_type() == ast::FactorType::FUNCTION_CALL) {
//   auto func_call =
//   std::static_pointer_cast<ast::AST_factor_function_call_Node>(factor);
//   factor->set_type(globalSymbolTable[func_call->get_identifier_node()->get_value()].typeDef[0]);
// }
// }

// void parser::assign_type_to_exp(std::shared_ptr<ast::AST_exp_Node> exp,
//   std::map<std::pair<std::string, int>,
//            symbolTable::symbolInfo> &symbol_table,
//   int indx) {

//   }

} // namespace parser
} // namespace scarlet
