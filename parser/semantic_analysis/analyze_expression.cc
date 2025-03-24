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

  // assign type to the expression
  assign_type_to_exp(exp);
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
      //  arguments. If not we cast the arguments to the correct type
      for (int i = 0; i < (int)func_call->get_arguments().size(); i++) {
        if (func_call->get_arguments()[i]->get_type() != gstTypeDef[i + 1]) {
          add_cast_to_exp(func_call->get_arguments()[i], gstTypeDef[i + 1]);
        }
      }
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

  // assign type to the factor
  assign_type_to_factor(factor);
}

void parser::assign_type_to_factor(
    std::shared_ptr<ast::AST_factor_Node> factor) {
  if (!success)
    return;

  if (factor->get_const_node() != nullptr) {
    factor->set_type(constTypeToElemType(
        factor->get_const_node()->get_constant().get_type()));
  } else if (factor->get_identifier_node() != nullptr) {
    factor->set_type(
        globalSymbolTable[factor->get_identifier_node()->get_value()]
            .typeDef[0]);
  } else if (factor->get_exp_node() != nullptr) {
    factor->set_type(factor->get_exp_node()->get_type());
  } else if (factor->get_unop_node() != nullptr) {
    if (factor->get_unop_node()->get_op() == unop::UNOP::NOT) {
      factor->set_type(ast::ElemType::INT);
    } else {
      factor->set_type(factor->get_child()->get_type());
    }
  } else if (factor->get_cast_type() != ast::ElemType::NONE) {
    factor->set_type(factor->get_cast_type());
  }
}

void parser::assign_type_to_exp(std::shared_ptr<ast::AST_exp_Node> exp) {
  if (!success)
    return;

  if (exp->get_binop_node() == nullptr) {
    exp->set_type(exp->get_factor_node()->get_type());
  } else {
    // Logical and / or depends on only one operand
    if (exp->get_binop_node()->get_op() == binop::BINOP::LAND or
        exp->get_binop_node()->get_op() == binop::BINOP::LOR) {
      exp->set_type(ast::ElemType::INT);
    } else if (exp->get_binop_node()->get_op() == binop::BINOP::ASSIGN or
               binop::is_compound(exp->get_binop_node()->get_op())) {
      exp->set_type(exp->get_factor_node()->get_type());
      if (exp->get_factor_node()->get_type() != exp->get_right()->get_type()) {
        add_cast_to_exp(exp->get_right(), exp->get_factor_node()->get_type());
      }
    } else if (exp->get_binop_node()->get_op() == binop::BINOP::TERNARY) {
      auto ternary = std::static_pointer_cast<ast::AST_ternary_exp_Node>(exp);
      ast::ElemType leftType = (ternary->get_middle() != nullptr)
                                   ? ternary->get_middle()->get_type()
                                   : exp->get_factor_node()->get_type();
      ast::ElemType rightType = exp->get_right()->get_type();

      ast::ElemType expType = getParentType(leftType, rightType);
      exp->set_type(expType);

      // Explicitly add cast operation in case of type mismatch
      if (expType != rightType) {
        add_cast_to_exp(exp->get_right(), expType);
      }
      if (expType != leftType) {
        (ternary->get_middle() != nullptr)
            ? add_cast_to_exp(ternary->get_middle(), expType)
            : add_cast_to_factor(exp->get_factor_node(), expType);
      }
    } else {
      ast::ElemType leftType = (exp->get_left() != nullptr)
                                   ? exp->get_left()->get_type()
                                   : exp->get_factor_node()->get_type();
      ast::ElemType rightType = exp->get_right()->get_type();

      ast::ElemType expType = getParentType(leftType, rightType);
      exp->set_type(expType);

      // Explicitly add cast operation in case of type mismatch
      if (expType != rightType) {
        add_cast_to_exp(exp->get_right(), expType);
      }
      if (expType != leftType) {
        (exp->get_left() != nullptr)
            ? add_cast_to_exp(exp->get_left(), expType)
            : add_cast_to_factor(exp->get_factor_node(), expType);
      }
    }
  }
}

void parser::add_cast_to_exp(std::shared_ptr<ast::AST_exp_Node> exp,
                             ast::ElemType type) {
  if (exp->get_binop_node() == nullptr) {
    MAKE_SHARED(ast::AST_exp_Node, copy_exp);
    copy_exp->set_factor_node(exp->get_factor_node());
    copy_exp->set_type(exp->get_type());

    MAKE_SHARED(ast::AST_factor_Node, cast_factor);
    cast_factor->set_cast_type(type);

    MAKE_SHARED(ast::AST_factor_Node, child_factor);
    child_factor->set_exp_node(std::move(copy_exp));
    cast_factor->set_child(std::move(child_factor));

    exp->set_factor_node(std::move(cast_factor));
    exp->set_type(type);
  } else {
    if (exp->get_binop_node()->get_op() == binop::BINOP::TERNARY) {
      auto ternary = std::static_pointer_cast<ast::AST_ternary_exp_Node>(exp);
      MAKE_SHARED(ast::AST_ternary_exp_Node, copy_ternary);
      copy_ternary->set_binop_node(exp->get_binop_node());
      copy_ternary->set_factor_node(exp->get_factor_node());
      copy_ternary->set_left(exp->get_left());
      copy_ternary->set_middle(ternary->get_middle());
      copy_ternary->set_right(exp->get_right());
      copy_ternary->set_type(exp->get_type());

      exp->set_binop_node(nullptr);
      exp->set_left(nullptr);
      ternary->set_middle(nullptr);
      exp->set_right(nullptr);

      MAKE_SHARED(ast::AST_factor_Node, cast_factor);
      cast_factor->set_cast_type(type);

      MAKE_SHARED(ast::AST_factor_Node, child_factor);
      auto copy_exp = std::static_pointer_cast<ast::AST_exp_Node>(copy_ternary);
      child_factor->set_exp_node(std::move(copy_exp));
      cast_factor->set_child(std::move(child_factor));

      exp->set_factor_node(std::move(cast_factor));
      exp->set_type(type);
    } else {
      MAKE_SHARED(ast::AST_exp_Node, copy_exp);
      copy_exp->set_binop_node(exp->get_binop_node());
      copy_exp->set_factor_node(exp->get_factor_node());
      copy_exp->set_left(exp->get_left());
      copy_exp->set_right(exp->get_right());
      copy_exp->set_type(exp->get_type());

      exp->set_binop_node(nullptr);
      exp->set_left(nullptr);
      exp->set_right(nullptr);

      MAKE_SHARED(ast::AST_factor_Node, cast_factor);
      cast_factor->set_cast_type(type);

      MAKE_SHARED(ast::AST_factor_Node, child_factor);
      child_factor->set_exp_node(std::move(copy_exp));
      cast_factor->set_child(std::move(child_factor));

      exp->set_factor_node(std::move(cast_factor));
      exp->set_type(type);
    }
  }
}

void parser::add_cast_to_factor(std::shared_ptr<ast::AST_factor_Node> factor,
                                ast::ElemType type) {
  MAKE_SHARED(ast::AST_factor_Node, copy_factor);
  copy_factor->set_const_node(factor->get_const_node());
  copy_factor->set_identifier_node(factor->get_identifier_node());
  copy_factor->set_unop_node(factor->get_unop_node());
  copy_factor->set_exp_node(factor->get_exp_node());
  copy_factor->set_factor_type(factor->get_factor_type());
  copy_factor->set_cast_type(factor->get_cast_type());
  copy_factor->set_child(factor->get_child());
  copy_factor->set_type(factor->get_type());

  factor->set_const_node(nullptr);
  factor->set_identifier_node(nullptr);
  factor->set_unop_node(nullptr);
  factor->set_exp_node(nullptr);
  factor->set_factor_type(ast::FactorType::BASIC);

  factor->set_cast_type(type);
  factor->set_type(type);
  factor->set_child(std::move(copy_factor));
}

} // namespace parser
} // namespace scarlet
