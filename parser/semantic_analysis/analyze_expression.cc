#include <parser/common.hh>

namespace scarlet {
namespace parser {

void parser::analyze_exp(std::shared_ptr<ast::AST_exp_Node> exp,
                         std::map<std::pair<std::string, int>,
                                  symbolTable::symbolInfo> &symbol_table,
                         int indx) {
  if (exp == nullptr)
    return;

  // Recursively check the left expression
  analyze_exp(exp->get_left(), symbol_table, indx);

  // Recursively check the factor
  analyze_factor(exp->get_factor_node(), symbol_table, indx);

  // Check that if the exp is of type assignment or exp is a compound expression
  // then factor is an identifier
  if (exp->get_binop_node() != nullptr and
      (exp->get_binop_node()->get_op() == binop::BINOP::ASSIGN or
       binop::is_compound(exp->get_binop_node()->get_op()))) {
    // ERROR CONDITION: (no factor node) or (factor node is a constant, not a
    // variable) or (factor node is a variable but has unary operators) Here we
    // exploit the benefit of short circuiting power of the logical operator
    // this means that as we proceed, we are ensured that the earlier checks
    // must not be satisfied. Note that an identifier with unops makes it an
    // rvalue.
    if (!ast::is_lvalue(exp->get_factor_node())) {
      success = false;
      error_messages.emplace_back("Expected a modifiable lvalue on the left "
                                  "side of the assignment operator");
    }
  }

  if (!success)
    return;

  // expand the compound expression
  if (exp->get_binop_node() != nullptr and
      binop::is_compound(exp->get_binop_node()->get_op())) {
    // a += 5
    //    |
    //    v
    // a = a + 5
    MAKE_SHARED(ast::AST_exp_Node, rightChild);

    MAKE_SHARED(ast::AST_binop_Node, rightBinop);
    rightBinop->set_op(compound_to_base[exp->get_binop_node()->get_op()]);
    rightChild->set_binop_node(rightBinop);

    MAKE_SHARED(ast::AST_factor_Node, rightFactor);
    rightFactor->set_factor_type(ast::FactorType::BASIC);
    MAKE_SHARED(ast::AST_identifier_Node, rightIdentifier);

    // remove after . from the identifier name
    std::string identifierName =
        exp->get_factor_node()->get_identifier_node()->get_value();
    auto dotPos = identifierName.find('.');
    if (dotPos != std::string::npos) {
      identifierName = identifierName.substr(0, dotPos);
    }

    rightIdentifier->set_identifier(identifierName);
    rightFactor->set_identifier_node(std::move(rightIdentifier));

    rightChild->set_factor_node(std::move(rightFactor));
    rightChild->set_right(exp->get_right());

    MAKE_SHARED(ast::AST_binop_Node, binop);
    binop->set_op(binop::BINOP::ASSIGN);
    exp->set_binop_node(binop);

    exp->set_right(rightChild);
  }

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

  if (exp->get_binop_node() != nullptr) {
    if (exp->get_right()->get_type() == ast::ElemType::DERIVED) {
      if (exp->get_binop_node()->get_op() == binop::BINOP::MOD) {
        success = false;
        error_messages.emplace_back(
            "Modulus, division and multiplication operator not allowed on "
            "derived types");
      } else if (exp->get_binop_node()->get_op() == binop::BINOP::DIV) {
        success = false;
        error_messages.emplace_back(
            "Modulus, division and multiplication operator not allowed on "
            "derived types");
      } else if (exp->get_binop_node()->get_op() == binop::BINOP::MUL) {
        success = false;
        error_messages.emplace_back(
            "Modulus, division and multiplication operator not allowed on "
            "derived types");
      }
    }
  }

  // left/right shift changed to logical left/right shift if the type is
  // unsigned
  if (exp->get_binop_node() != nullptr and
      (exp->get_binop_node()->get_op() == binop::BINOP::LEFT_SHIFT or
       exp->get_binop_node()->get_op() == binop::BINOP::RIGHT_SHIFT)) {
    if (exp->get_type() == ast::ElemType::UINT or
        exp->get_type() == ast::ElemType::ULONG) {
      if (exp->get_binop_node()->get_op() == binop::BINOP::LEFT_SHIFT) {
        exp->get_binop_node()->set_op(binop::BINOP::LOGICAL_LEFT_SHIFT);
      } else {
        exp->get_binop_node()->set_op(binop::BINOP::LOGICAL_RIGHT_SHIFT);
      }
    }
  }

  // check that modulus | Xor is not used on double precision
  if (exp->get_binop_node() != nullptr and
      exp->get_type() == ast::ElemType::DOUBLE) {
    if (exp->get_binop_node()->get_op() == binop::BINOP::MOD) {
      success = false;
      error_messages.emplace_back(
          "Modulus operator not allowed on double precision");
    }

    if (exp->get_binop_node()->get_op() == binop::BINOP::XOR) {
      success = false;
      error_messages.emplace_back(
          "XOR operator not allowed on double precision");
    }
  }
}

void parser::analyze_factor(std::shared_ptr<ast::AST_factor_Node> factor,
                            std::map<std::pair<std::string, int>,
                                     symbolTable::symbolInfo> &symbol_table,
                            int indx) {
  if (factor == nullptr)
    return;

  // Recursively check the factor child
  analyze_factor(factor->get_child(), symbol_table, indx);

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

  if (!success)
    return;

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
    auto gstDerivedTypeDef =
        globalSymbolTable[func_call->get_identifier_node()->get_value()]
            .derivedTypeMap;
    if ((int)func_call->get_arguments().size() != (int)gstTypeDef.size() - 1) {
      success = false;
      error_messages.emplace_back(
          "Function " + func_call->get_identifier_node()->get_value() +
          " called with wrong number of arguments");
    } else {
      // check that the function is being called with the correct type of
      //  arguments. If not we cast the arguments to the correct type
      for (int i = 0; i < (int)func_call->get_arguments().size(); i++) {
        if (func_call->get_arguments()[i]->get_type() != gstTypeDef[i + 1] or
            func_call->get_arguments()[i]->get_derived_type() !=
                gstDerivedTypeDef[i + 1]) {
          add_cast_to_exp(func_call->get_arguments()[i], gstTypeDef[i + 1],
                          gstDerivedTypeDef[i + 1]);
        }
      }
    }
  }

  if (!success)
    return;

  // If factor has an increment/decrement operator
  // make sure it operates on an lvalue
  if (factor->get_unop_node() != nullptr and
      unop::is_incr_decr(factor->get_unop_node()->get_op())) {
    if (!ast::is_lvalue(factor->get_child())) {
      success = false;
      error_messages.emplace_back(
          "Expected an lvalue for the increment / decrement operator");
    }
  }

  // If factor has an addressof(&) operator
  // make sure it operates on an lvalue
  if (factor->get_unop_node() != nullptr and
      factor->get_unop_node()->get_op() == unop::UNOP::ADDROF) {
    if (!ast::is_lvalue(factor->get_child())) {
      success = false;
      error_messages.emplace_back(
          "Expected an lvalue for AddressOf (&) operator");
    }
  }

  // since the factor can have its own exp as well, we recursively check that
  analyze_exp(factor->get_exp_node(), symbol_table, indx);

  // assign type to the factor
  assign_type_to_factor(factor);

  // complement operator cannot be used on double precision
  if (factor->get_unop_node() != nullptr) {
    if (factor->get_type() == ast::ElemType::DOUBLE) {
      if (factor->get_unop_node()->get_op() == unop::UNOP::COMPLEMENT) {
        success = false;
        error_messages.emplace_back(
            "Complement operator not allowed on double precision");
      }
    }
    if (factor->get_type() == ast::ElemType::DERIVED) {
      if (factor->get_unop_node()->get_op() == unop::UNOP::COMPLEMENT) {
        // FIXME : include case for array
        success = false;
        error_messages.emplace_back(
            "Complement operator not allowed on pointer types");
      }

      if (factor->get_unop_node()->get_op() == unop::UNOP::NEGATE) {

        // FIXME : include case for array
        success = false;
        error_messages.emplace_back("negation not allowed on pointer types");
      }
    }
  }
}

void parser::assign_type_to_factor(
    std::shared_ptr<ast::AST_factor_Node> factor) {
  if (!success)
    return;

  if (factor->get_const_node() != nullptr) {
    factor->set_type(ast::constTypeToElemType(
        factor->get_const_node()->get_constant().get_type()));
  } else if (factor->get_identifier_node() != nullptr) {
    auto identInfo =
        globalSymbolTable[factor->get_identifier_node()->get_value()];
    factor->set_type(identInfo.typeDef[0]);
    if (factor->get_type() == ast::ElemType::DERIVED) {
      factor->set_derived_type(identInfo.derivedTypeMap[0]);
    }
  } else if (factor->get_exp_node() != nullptr) {
    factor->set_type(factor->get_exp_node()->get_type());
    factor->set_derived_type(factor->get_exp_node()->get_derived_type());
  } else if (factor->get_unop_node() != nullptr) {
    if (factor->get_unop_node()->get_op() == unop::UNOP::NOT) {
      factor->set_type(ast::ElemType::INT);
    } else if (factor->get_unop_node()->get_op() == unop::UNOP::DEREFERENCE) {
      if (factor->get_child()->get_type() == ast::ElemType::DERIVED and
          factor->get_child()->get_derived_type()[0] ==
              (long)ast::ElemType::POINTER) {
        auto parentType = factor->get_child()->get_derived_type();
        parentType.erase(parentType.begin());
        if (parentType[0] <= -3) {
          factor->set_type((ast::ElemType)parentType[0]);
        } else {
          factor->set_type(ast::ElemType::DERIVED);
          factor->set_derived_type(parentType);
        }
      } else {
        success = false;
        error_messages.emplace_back(
            "Dereference operator not allowed on non-pointer types");
      }
    } else if (factor->get_unop_node()->get_op() == unop::UNOP::ADDROF) {
      factor->set_type(ast::ElemType::DERIVED);
      std::vector<long> derivedType;
      if (factor->get_child()->get_type() == ast::ElemType::DERIVED) {
        derivedType = factor->get_child()->get_derived_type();
        derivedType.insert(derivedType.begin(), (long)ast::ElemType::POINTER);
      } else {
        derivedType.push_back((long)ast::ElemType::POINTER);
        derivedType.push_back((long)factor->get_child()->get_type());
      }
      factor->set_derived_type(derivedType);
    } else {
      factor->set_type(factor->get_child()->get_type());
      factor->set_derived_type(factor->get_child()->get_derived_type());
    }
  } else if (factor->get_cast_type() != ast::ElemType::NONE) {
    std::vector<long> derivedType;
    unroll_derived_type(factor->get_cast_declarator(), derivedType);
    if (!derivedType.empty()) {
      if (factor->get_type() == ast::ElemType::DOUBLE and
          derivedType.size() > 0) {
        // FIXME : include case for array
        success = false;
        error_messages.emplace_back(
            "Cannot cast double precision to pointer type");
      }
      derivedType.push_back((long)factor->get_cast_type());
      factor->set_derived_type(derivedType);
      factor->set_type(ast::ElemType::DERIVED);
    } else {
      if (factor->get_cast_type() == ast::ElemType::DOUBLE and
          factor->get_type() == ast::ElemType::DERIVED) {
        // FIXME : include case for array
        success = false;
        error_messages.emplace_back(
            "Cannot cast pointer type to double precision");
      }
      factor->set_type(factor->get_cast_type());
    }
  }
}

void parser::assign_type_to_exp(std::shared_ptr<ast::AST_exp_Node> exp) {
  if (!success)
    return;

  if (exp->get_binop_node() == nullptr) {
    exp->set_type(exp->get_factor_node()->get_type());
    exp->set_derived_type(exp->get_factor_node()->get_derived_type());
  } else {
    // Logical and / or depends on only one operand
    if (exp->get_binop_node()->get_op() == binop::BINOP::LAND or
        exp->get_binop_node()->get_op() == binop::BINOP::LOR) {
      exp->set_type(ast::ElemType::INT);
    } else if (exp->get_binop_node()->get_op() == binop::BINOP::ASSIGN or
               binop::is_compound(exp->get_binop_node()->get_op())) {
      auto [expType, expDerivedType] = ast::getAssignType(
          exp->get_factor_node()->get_type(),
          exp->get_factor_node()->get_derived_type(),
          exp->get_right()->get_type(), exp->get_right()->get_derived_type(),
          exp->get_right());
      if (expType == ast::ElemType::NONE) {
        success = false;
        error_messages.emplace_back("Incompatible types in expression");
      }
      if (expType != exp->get_right()->get_type() or
          expDerivedType != exp->get_right()->get_derived_type()) {
        add_cast_to_exp(exp->get_right(), expType, expDerivedType);
      }
      exp->set_type(expType);
      exp->set_derived_type(expDerivedType);
      if (exp->get_factor_node()->get_type() != exp->get_right()->get_type()) {
        add_cast_to_exp(exp->get_right(), exp->get_type(),
                        exp->get_derived_type());
      }
    } else if (exp->get_binop_node()->get_op() == binop::BINOP::TERNARY) {
      auto ternary = std::static_pointer_cast<ast::AST_ternary_exp_Node>(exp);
      ast::ElemType leftType = (ternary->get_middle() != nullptr)
                                   ? ternary->get_middle()->get_type()
                                   : exp->get_factor_node()->get_type();
      ast::ElemType rightType = exp->get_right()->get_type();
      auto leftDerivedType = (ternary->get_middle() != nullptr)
                                 ? ternary->get_middle()->get_derived_type()
                                 : exp->get_factor_node()->get_derived_type();
      auto rightDerivedType = exp->get_right()->get_derived_type();
      auto [expType, expDerivedType] = ast::getParentType(
          leftType, rightType, leftDerivedType, rightDerivedType, exp);
      if (expType == ast::ElemType::NONE) {
        success = false;
        error_messages.emplace_back("Incompatible types in expression");
      }

      if (expType == leftType and expDerivedType == leftDerivedType) {
        add_cast_to_exp(exp->get_right(), expType, expDerivedType);
      } else if (expType == rightType and expDerivedType == rightDerivedType) {
        (ternary->get_middle() != nullptr)
            ? add_cast_to_exp(ternary->get_middle(), expType, expDerivedType)
            : add_cast_to_factor(exp->get_factor_node(), expType,
                                 expDerivedType);
      }

      exp->set_type(expType);
      exp->set_derived_type(expDerivedType);
    } else if (exp->get_binop_node()->get_op() == binop::BINOP::RIGHT_SHIFT or
               exp->get_binop_node()->get_op() == binop::BINOP::LEFT_SHIFT) {
      ast::ElemType leftType = (exp->get_left() != nullptr)
                                   ? exp->get_left()->get_type()
                                   : exp->get_factor_node()->get_type();
      ast::ElemType rightType = exp->get_right()->get_type();
      if (leftType == ast::ElemType::DOUBLE or
          rightType == ast::ElemType::DOUBLE) {
        success = false;
        error_messages.emplace_back(
            "Shift operator is not allowed on double precision");
      } else if (leftType == ast::ElemType::DERIVED or
                 rightType == ast::ElemType::DERIVED) {
        success = false;
        error_messages.emplace_back(
            "Shift operator is not allowed on derived types");
      } else {
        ast::ElemType expType = leftType;
        if (expType != rightType) {
          add_cast_to_exp(exp->get_right(), expType, {});
        }
        exp->set_type(expType);
      }
    } else {
      ast::ElemType leftType = (exp->get_left() != nullptr)
                                   ? exp->get_left()->get_type()
                                   : exp->get_factor_node()->get_type();
      ast::ElemType rightType = exp->get_right()->get_type();
      auto leftDerivedType = (exp->get_left() != nullptr)
                                 ? exp->get_left()->get_derived_type()
                                 : exp->get_factor_node()->get_derived_type();
      auto rightDerivedType = exp->get_right()->get_derived_type();
      auto [expType, expDerivedType] = ast::getParentType(
          leftType, rightType, leftDerivedType, rightDerivedType, exp);
      if (expType == ast::ElemType::NONE) {
        success = false;
        error_messages.emplace_back("Incompatible types in expression");
      }

      if (expType == leftType and expDerivedType == leftDerivedType) {
        add_cast_to_exp(exp->get_right(), expType, expDerivedType);
      } else if (expType == rightType and expDerivedType == rightDerivedType) {
        (exp->get_left() != nullptr)
            ? add_cast_to_exp(exp->get_left(), expType, expDerivedType)
            : add_cast_to_factor(exp->get_factor_node(), expType,
                                 expDerivedType);
      }

      if (binop::is_relational(exp->get_binop_node()->get_op())) {
        exp->set_type(ast::ElemType::INT);
      } else {
        exp->set_type(expType);
        exp->set_derived_type(expDerivedType);
      }
    }
  }
}

void parser::add_cast_to_exp(std::shared_ptr<ast::AST_exp_Node> exp,
                             ast::ElemType type,
                             std::vector<long> derivedType) {
  if (exp->get_binop_node() == nullptr) {
    MAKE_SHARED(ast::AST_exp_Node, copy_exp);
    copy_exp->set_factor_node(exp->get_factor_node());
    copy_exp->set_type(exp->get_type());
    copy_exp->set_derived_type(exp->get_derived_type());

    MAKE_SHARED(ast::AST_factor_Node, cast_factor);
    cast_factor->set_cast_type(type);
    cast_factor->set_type(type);
    cast_factor->set_derived_type(derivedType);

    MAKE_SHARED(ast::AST_factor_Node, child_factor);
    child_factor->set_exp_node(std::move(copy_exp));
    child_factor->set_type(exp->get_type());
    child_factor->set_derived_type(exp->get_derived_type());
    cast_factor->set_child(std::move(child_factor));

    exp->set_factor_node(std::move(cast_factor));
    exp->set_type(type);
    exp->set_derived_type(derivedType);
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
      copy_ternary->set_derived_type(exp->get_derived_type());

      exp->set_binop_node(nullptr);
      exp->set_left(nullptr);
      ternary->set_middle(nullptr);
      exp->set_right(nullptr);

      MAKE_SHARED(ast::AST_factor_Node, cast_factor);
      cast_factor->set_cast_type(type);
      cast_factor->set_type(type);
      cast_factor->set_derived_type(derivedType);

      MAKE_SHARED(ast::AST_factor_Node, child_factor);
      auto copy_exp = std::static_pointer_cast<ast::AST_exp_Node>(copy_ternary);
      child_factor->set_exp_node(std::move(copy_exp));
      child_factor->set_type(exp->get_type());
      child_factor->set_derived_type(exp->get_derived_type());
      cast_factor->set_child(std::move(child_factor));

      exp->set_factor_node(std::move(cast_factor));
      exp->set_type(type);
      exp->set_derived_type(derivedType);
    } else {
      MAKE_SHARED(ast::AST_exp_Node, copy_exp);
      copy_exp->set_binop_node(exp->get_binop_node());
      copy_exp->set_factor_node(exp->get_factor_node());
      copy_exp->set_left(exp->get_left());
      copy_exp->set_right(exp->get_right());
      copy_exp->set_type(exp->get_type());
      copy_exp->set_derived_type(exp->get_derived_type());

      exp->set_binop_node(nullptr);
      exp->set_left(nullptr);
      exp->set_right(nullptr);

      MAKE_SHARED(ast::AST_factor_Node, cast_factor);
      cast_factor->set_cast_type(type);
      cast_factor->set_type(type);
      cast_factor->set_derived_type(derivedType);

      MAKE_SHARED(ast::AST_factor_Node, child_factor);
      child_factor->set_exp_node(std::move(copy_exp));
      child_factor->set_type(exp->get_type());
      child_factor->set_derived_type(exp->get_derived_type());
      cast_factor->set_child(std::move(child_factor));

      exp->set_factor_node(std::move(cast_factor));
      exp->set_type(type);
      exp->set_derived_type(derivedType);
    }
  }
}

void parser::add_cast_to_factor(std::shared_ptr<ast::AST_factor_Node> factor,
                                ast::ElemType type,
                                std::vector<long> derivedType) {
  if (factor->get_factor_type() == ast::FactorType::FUNCTION_CALL) {
    auto funcCall =
        std::static_pointer_cast<ast::AST_factor_function_call_Node>(factor);
    MAKE_SHARED(ast::AST_factor_function_call_Node, copy_factor);
    copy_factor->set_const_node(factor->get_const_node());
    copy_factor->set_identifier_node(factor->get_identifier_node());
    copy_factor->set_unop_node(factor->get_unop_node());
    copy_factor->set_exp_node(factor->get_exp_node());
    copy_factor->set_factor_type(factor->get_factor_type());
    copy_factor->set_cast_type(factor->get_cast_type());
    copy_factor->set_child(factor->get_child());
    copy_factor->set_type(factor->get_type());
    copy_factor->set_arguments(funcCall->get_arguments());
    copy_factor->set_derived_type(factor->get_derived_type());

    factor->set_const_node(nullptr);
    factor->set_identifier_node(nullptr);
    factor->set_unop_node(nullptr);
    factor->set_exp_node(nullptr);
    factor->set_factor_type(ast::FactorType::BASIC);

    factor->set_cast_type(type);
    factor->set_type(type);
    factor->set_derived_type(derivedType);
    factor->set_child(std::move(copy_factor));
  } else {
    MAKE_SHARED(ast::AST_factor_Node, copy_factor);
    copy_factor->set_const_node(factor->get_const_node());
    copy_factor->set_identifier_node(factor->get_identifier_node());
    copy_factor->set_unop_node(factor->get_unop_node());
    copy_factor->set_exp_node(factor->get_exp_node());
    copy_factor->set_factor_type(factor->get_factor_type());
    copy_factor->set_cast_type(factor->get_cast_type());
    copy_factor->set_child(factor->get_child());
    copy_factor->set_type(factor->get_type());
    copy_factor->set_derived_type(factor->get_derived_type());

    factor->set_const_node(nullptr);
    factor->set_identifier_node(nullptr);
    factor->set_unop_node(nullptr);
    factor->set_exp_node(nullptr);
    factor->set_factor_type(ast::FactorType::BASIC);

    factor->set_cast_type(type);
    factor->set_type(type);
    factor->set_derived_type(derivedType);
    factor->set_child(std::move(copy_factor));
  }
}

} // namespace parser
} // namespace scarlet
