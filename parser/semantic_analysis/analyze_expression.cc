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
    if (!ast::is_lvalue(exp->get_factor_node())) {
      success = false;
      error_messages.emplace_back("Expected a modifiable lvalue on the left "
                                  "side of the assignment operator");
    }
  }

  if (!success)
    return;

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

  // expand the compound expression
  if (exp->get_binop_node() != nullptr and
      binop::is_compound(exp->get_binop_node()->get_op())) {
    auto binop = binop::compound_to_base(exp->get_binop_node()->get_op());
    auto factor = exp->get_factor_node();
    auto right = exp->get_right();

    MAKE_SHARED(ast::AST_exp_Node, base_exp);
    MAKE_SHARED(ast::AST_binop_Node, binop_node);
    binop_node->set_op(binop);
    base_exp->set_binop_node(binop_node);
    base_exp->set_factor_node(factor);
    base_exp->set_right(right);
    base_exp->set_type(exp->get_type());
    base_exp->set_derived_type(exp->get_derived_type());

    if (factor->get_cast_type() != ast::ElemType::NONE) {
      auto baseType = factor->get_child()->get_type();
      auto baseDerivedType = factor->get_child()->get_derived_type();

      add_cast_to_exp(base_exp, baseType, baseDerivedType);

      exp->set_type(baseType);
      exp->set_derived_type(baseDerivedType);

      exp->set_factor_node(factor->get_child());
    }

    exp->get_binop_node()->set_op(binop::BINOP::ASSIGN);
    exp->set_right(base_exp);
  }
}

void parser::analyze_factor(std::shared_ptr<ast::AST_factor_Node> factor,
                            std::map<std::pair<std::string, int>,
                                     symbolTable::symbolInfo> &symbol_table,
                            int indx) {
  if (factor == nullptr)
    return;

  if (factor->get_arrIdx().size() > 0) {
    /*
     * When we have array subscripts, we expand them by adding the subscript
     * to the base type. The only base types allowed are array and pointers.
     * So for instance, p[i][j] will expand to *(*(p + i) + j)
     */
    auto arrIdx = factor->get_arrIdx();
    factor->set_arrIdx({});
    MAKE_SHARED(ast::AST_factor_Node, propogate_factor);

    MAKE_SHARED(ast::AST_unop_Node, unop);
    unop->set_op(unop::UNOP::DEREFERENCE);

    for (int i = 0; i < (int)arrIdx.size(); i++) {
      MAKE_SHARED(ast::AST_factor_Node, deref_factor);
      deref_factor->set_unop_node(unop);
      MAKE_SHARED(ast::AST_factor_Node, child_factor);

      MAKE_SHARED(ast::AST_exp_Node, exp);
      MAKE_SHARED(ast::AST_binop_Node, binop_node);
      binop_node->set_op(binop::BINOP::ADD);
      exp->set_binop_node(binop_node);
      exp->set_right(arrIdx[i]);

      child_factor->set_exp_node(exp);
      deref_factor->set_child(child_factor);

      if (i == 0) {
        exp->set_binop_node(binop_node);
        if (factor->get_exp_node() != nullptr) {
          exp->set_left(factor->get_exp_node());

          factor->set_exp_node(nullptr);
        } else if (factor->get_identifier_node() != nullptr) {
          if (factor->get_factor_type() == ast::FactorType::FUNCTION_CALL) {
            auto func_call_factor =
                std::static_pointer_cast<ast::AST_factor_function_call_Node>(
                    factor);
            MAKE_SHARED(ast::AST_factor_function_call_Node, func_call);
            func_call->set_identifier_node(
                func_call_factor->get_identifier_node());
            func_call->set_arguments(func_call_factor->get_arguments());
            func_call->set_factor_type(ast::FactorType::FUNCTION_CALL);

            func_call_factor->set_identifier_node(nullptr);
            func_call_factor->set_factor_type(ast::FactorType::BASIC);
            func_call_factor->set_arguments({});

            exp->set_factor_node(func_call);
          } else {
            MAKE_SHARED(ast::AST_factor_Node, base_factor);
            base_factor->set_identifier_node(factor->get_identifier_node());

            factor->set_identifier_node(nullptr);

            exp->set_factor_node(base_factor);
          }
        } else if (factor->get_unop_node() != nullptr and
                   unop::is_incr_decr(factor->get_unop_node()->get_op())) {
          MAKE_SHARED(ast::AST_factor_Node, base_factor);
          base_factor->set_unop_node(factor->get_unop_node());
          base_factor->set_child(factor->get_child());
          factor->set_unop_node(nullptr);
          factor->set_child(nullptr);
          exp->set_factor_node(base_factor);
        } else if (ast::factor_is_string(factor)) {
          MAKE_SHARED(ast::AST_factor_Node, base_factor);
          base_factor->set_const_node(factor->get_const_node());

          factor->set_const_node(nullptr);

          exp->set_factor_node(base_factor);
        }
      } else {
        exp->set_factor_node(propogate_factor);
      }

      propogate_factor = deref_factor;
    }

    factor->set_unop_node(unop);
    factor->set_child(propogate_factor->get_child());
  }

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
    // special case for compound expressions as we are copying the factor
    auto dotPos = var_name.find('.');
    if (dotPos != std::string::npos) {
      var_name = var_name.substr(0, dotPos);
    }
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
      decay_arr_to_pointer(nullptr, it);
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
        auto funcArgType = func_call->get_arguments()[i]->get_type();
        auto funcArgDerivedType =
            func_call->get_arguments()[i]->get_derived_type();

        auto [castType, castDerivedType] = ast::getAssignType(
            gstTypeDef[i + 1], gstDerivedTypeDef[i + 1], funcArgType,
            funcArgDerivedType, func_call->get_arguments()[i]);
        if (castType == ast::ElemType::NONE) {
          success = false;
          error_messages.emplace_back(
              "Function " + func_call->get_identifier_node()->get_value() +
              " called with wrong type of arguments");
        } else {
          if (castType != funcArgType or
              castDerivedType != funcArgDerivedType) {
            add_cast_to_exp(func_call->get_arguments()[i], castType,
                            castDerivedType);
          }
        }
      }
    }
  }

  if (!success)
    return;

  if (factor->get_unop_node() != nullptr) {
    if (unop::is_incr_decr(factor->get_unop_node()->get_op())) {
      if (!ast::is_lvalue(factor->get_child())) {
        success = false;
        error_messages.emplace_back(
            "Expected an lvalue for the increment / decrement operator");
      }
      if (ast::is_void_ptr(factor->get_child()->get_type(),
                           factor->get_child()->get_derived_type())) {
        success = false;
        error_messages.emplace_back(
            "Increment / Decrement operator not allowed on void type");
      }
    }

    if (factor->get_unop_node()->get_op() == unop::UNOP::ADDROF) {
      if (!ast::is_lvalue(factor->get_child()) and
          !ast::factor_is_string(factor->get_child())) {
        success = false;
        error_messages.emplace_back(
            "Expected an lvalue for AddressOf (&) operator");
      }
    }
  }

  // since the factor can have its own exp as well, we recursively check that
  analyze_exp(factor->get_exp_node(), symbol_table, indx);

  // assign type to the factor
  assign_type_to_factor(factor);

  // complement operator cannot be used on double precision
  if (factor->get_unop_node() != nullptr) {
    auto unop = factor->get_unop_node()->get_op();
    if (factor->get_type() == ast::ElemType::DOUBLE) {
      if (unop == unop::UNOP::COMPLEMENT) {
        success = false;
        error_messages.emplace_back(
            "Complement operator not allowed on double precision");
      }
    }
    if (factor->get_type() == ast::ElemType::DERIVED) {
      if (unop == unop::UNOP::COMPLEMENT) {
        success = false;
        error_messages.emplace_back(
            "Complement operator not allowed on pointer types");
      }

      if (unop == unop::UNOP::NEGATE) {
        success = false;
        error_messages.emplace_back("negation not allowed on pointer types");
      }
    }

    if (unop != unop::UNOP::SIZEOF and
        factor->get_child()->get_type() == ast::ElemType::VOID) {
      success = false;
      error_messages.emplace_back("Void type not allowed for unary operator");
    }

    if (unop == unop::UNOP::SIZEOF) {
      if (factor->get_child() != nullptr) {
        // exp has been analyzed already
        if (!ast::validate_type_specifier(
                factor->get_child()->get_type(),
                factor->get_child()->get_derived_type())) {
          success = false;
          error_messages.emplace_back(
              "sizeof operator not allowed on incomplete type");
        }
        factor->set_type(ast::ElemType::ULONG);
        factor->set_derived_type({});
      } else {
        std::vector<long> derivedType;
        ast::unroll_derived_type(factor->get_cast_declarator(), derivedType);
        if (!derivedType.empty()) {
          derivedType.push_back((long)factor->get_cast_type());
          if (!ast::validate_type_specifier(ast::ElemType::DERIVED,
                                            derivedType)) {
            success = false;
            error_messages.emplace_back(
                "sizeof operator not allowed on incomplete type");
          }
        } else if (factor->get_cast_type() == ast::ElemType::VOID) {
          success = false;
          error_messages.emplace_back(
              "sizeof operator not allowed on incomplete type");
        }
        factor->set_type(ast::ElemType::ULONG);
        factor->set_derived_type({});
      }
    }

    // convert pre increment / decrement to an expression
    if (unop::is_incr_decr(unop)) {
      auto base = factor->get_child();
      auto baseType = base->get_type();
      auto baseDerivedType = base->get_derived_type();

      MAKE_SHARED(ast::AST_exp_Node, exp);
      exp->set_type(factor->get_type());
      exp->set_derived_type(factor->get_derived_type());
      MAKE_SHARED(ast::AST_binop_Node, binop_node);
      binop_node->set_op(binop::BINOP::ASSIGN);
      exp->set_binop_node(binop_node);
      exp->set_factor_node(base);

      MAKE_SHARED(ast::AST_exp_Node, right);
      right->set_type(factor->get_type());
      right->set_derived_type(factor->get_derived_type());
      /* It is possible that the baseType is {u}char in which case we will have
       * to cast it to int */
      if (baseType == ast::ElemType::CHAR or baseType == ast::ElemType::UCHAR) {
        MAKE_SHARED(ast::AST_factor_Node, rightFactor);
        rightFactor->copy(base);
        add_cast_to_factor(rightFactor, ast::ElemType::INT, {});
        right->set_factor_node(rightFactor);
      } else {
        right->set_factor_node(base);
      }
      MAKE_SHARED(ast::AST_binop_Node, binop_node2);
      if (unop == unop::UNOP::PREDECREMENT or unop == unop::UNOP::POSTDECREMENT)
        binop_node2->set_op(binop::BINOP::SUB);
      else
        binop_node2->set_op(binop::BINOP::ADD);
      right->set_binop_node(binop_node2);

      MAKE_SHARED(ast::AST_exp_Node, constExp);
      constExp->set_type(baseType);
      constExp->set_derived_type(baseDerivedType);
      MAKE_SHARED(ast::AST_factor_Node, constFactor);
      constFactor->set_type(baseType);
      constFactor->set_derived_type(baseDerivedType);
      MAKE_SHARED(ast::AST_const_Node, constVal);
      constant::Constant one;
      if (baseType == ast::ElemType::DOUBLE) {
        one.set_type(constant::Type::DOUBLE);
        one.set_value({.d = 1.0});
      } else {
        one.set_type(constant::Type::LONG);
        one.set_value({.l = 1});
        if (baseType == ast::ElemType::DERIVED) {
          constExp->set_type(ast::ElemType::INT);
          constExp->set_derived_type({});
          constFactor->set_type(ast::ElemType::INT);
          constFactor->set_derived_type({});
        }
      }
      constVal->set_constant(one);
      constFactor->set_const_node(constVal);
      constExp->set_factor_node(constFactor);

      right->set_right(constExp);
      exp->set_right(right);

      if (unop == unop::UNOP::POSTDECREMENT or
          unop == unop::UNOP::POSTINCREMENT) {
        // preserve the unop and child so that during scargen, we can propagate
        // the original lvalue (before updation)
      } else {
        factor->set_unop_node(nullptr);
        factor->set_child(nullptr);
      }
      factor->set_exp_node(exp);
    }
  }
}

void parser::assign_type_to_factor(
    std::shared_ptr<ast::AST_factor_Node> factor) {
  if (!success)
    return;

  if (factor->get_const_node() != nullptr) {
    if (ast::factor_is_string(factor)) {
      std::vector<long> derivedType{};
      derivedType.push_back(
          factor->get_const_node()->get_constant().get_string().size() + 1);
      derivedType.push_back((long)ast::ElemType::CHAR);
      factor->set_type(ast::ElemType::DERIVED);
      factor->set_derived_type(derivedType);
    } else {
      factor->set_type(ast::constTypeToElemType(
          factor->get_const_node()->get_constant().get_type()));
    }
  } else if (factor->get_identifier_node() != nullptr) {
    auto identInfo =
        globalSymbolTable[factor->get_identifier_node()->get_value()];
    factor->set_type(identInfo.typeDef[0]);
    factor->set_derived_type(identInfo.derivedTypeMap[0]);
  } else if (factor->get_exp_node() != nullptr) {
    auto exp = factor->get_exp_node();
    factor->set_type(exp->get_type());
    factor->set_derived_type(exp->get_derived_type());
  } else if (factor->get_unop_node() != nullptr) {
    auto unop = factor->get_unop_node()->get_op();
    if (unop == unop::UNOP::SIZEOF) {
      return;
    }

    if (unop == unop::UNOP::DECAY) {
      // We have already taken care of the type that will be assigned
      return;
    }

    if (unop::is_incr_decr(unop)) {
      if (ast::is_array(factor->get_child())) {
        success = false;
        error_messages.emplace_back(
            "Increment / Decrement operator not allowed on array type");
      }
    }

    if (unop != unop::UNOP::ADDROF) {
      decay_arr_to_pointer(factor->get_child(), nullptr);
    }

    if (unop == unop::UNOP::NOT) {
      factor->set_type(ast::ElemType::INT);
    } else if (unop == unop::UNOP::DEREFERENCE) {
      if (factor->get_child()->get_type() == ast::ElemType::DERIVED and
          factor->get_child()->get_derived_type()[0] ==
              (long)ast::ElemType::POINTER) {
        auto parentType = factor->get_child()->get_derived_type();
        parentType.erase(parentType.begin());
        // parentType is {u}int/{u}long/double
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
    } else if (unop == unop::UNOP::ADDROF) {
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
    } else if (unop::is_incr_decr(unop)) {
      factor->set_type(factor->get_child()->get_type());
      factor->set_derived_type(factor->get_child()->get_derived_type());
    } else {
      // implicit promotion of char to int
      if (factor->get_child()->get_type() == ast::ElemType::CHAR or
          factor->get_child()->get_type() == ast::ElemType::UCHAR) {
        add_cast_to_factor(factor->get_child(), ast::ElemType::INT, {});
      }
      factor->set_type(factor->get_child()->get_type());
      factor->set_derived_type(factor->get_child()->get_derived_type());
    }
  } else if (factor->get_cast_type() != ast::ElemType::NONE) {
    decay_arr_to_pointer(factor->get_child(), nullptr);
    std::vector<long> derivedType;
    ast::unroll_derived_type(factor->get_cast_declarator(), derivedType);

    if (!derivedType.empty()) {
      if (factor->get_child()->get_type() == ast::ElemType::DOUBLE) {
        success = false;
        error_messages.emplace_back(
            "Cannot cast double precision to pointer type");
      }
      if (derivedType[0] > 0) {
        success = false;
        error_messages.emplace_back("cannot cast to an array");
      }
      derivedType.push_back((long)factor->get_cast_type());
      factor->set_derived_type(derivedType);
      factor->set_type(ast::ElemType::DERIVED);
      if (!ast::validate_type_specifier(ast::ElemType::DERIVED, derivedType)) {
        success = false;
        error_messages.emplace_back(
            "Casting operator not allowed on incomplete type");
      }
    } else {
      if (factor->get_cast_type() == ast::ElemType::DOUBLE and
          factor->get_child()->get_type() == ast::ElemType::DERIVED) {
        success = false;
        error_messages.emplace_back(
            "Cannot cast pointer type to double precision");
      }
      factor->set_type(factor->get_cast_type());
    }
    if (factor->get_child()->get_type() == ast::ElemType::VOID and
        factor->get_cast_type() != ast::ElemType::VOID) {
      success = false;
      error_messages.emplace_back(
          "Casting operator not allowed on non-scalar types");
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
    binop::BINOP binop = exp->get_binop_node()->get_op();
    decay_arr_to_pointer(nullptr, exp->get_left());
    decay_arr_to_pointer(nullptr, exp->get_right());
    if (binop != binop::BINOP::ASSIGN and !binop::is_compound(binop)) {
      decay_arr_to_pointer(exp->get_factor_node(), nullptr);
    }

    if (binop::is_compound(binop)) {
      if (ast::is_array(exp->get_factor_node())) {
        success = false;
        error_messages.emplace_back(
            "Assignment operator not allowed on array type");
      }

      binop = binop::compound_to_base(binop);
    }
    // Logical and / or depends on only one operand
    if (binop == binop::BINOP::LAND or binop == binop::BINOP::LOR) {
      exp->set_type(ast::ElemType::INT);
    } else if (binop == binop::BINOP::ASSIGN) {
      auto leftType = exp->get_factor_node()->get_type();
      auto leftDerivedType = exp->get_factor_node()->get_derived_type();
      auto rightType = exp->get_right()->get_type();
      auto rightDerivedType = exp->get_right()->get_derived_type();
      if (leftType == ast::ElemType::DERIVED and leftDerivedType[0] > 0) {
        success = false;
        error_messages.emplace_back(
            "Assignment operator not allowed on array type");
      } else {
        auto [expType, expDerivedType] =
            ast::getAssignType(leftType, leftDerivedType, rightType,
                               rightDerivedType, exp->get_right());
        if (expType == ast::ElemType::NONE or expType == ast::ElemType::VOID) {
          success = false;
          error_messages.emplace_back("Incompatible types in expression");
        } else {
          if (expType != rightType or expDerivedType != rightDerivedType) {
            add_cast_to_exp(exp->get_right(), expType, expDerivedType);
          }
          exp->set_type(expType);
          exp->set_derived_type(expDerivedType);
        }
      }
    } else if (binop == binop::BINOP::TERNARY) {
      auto ternary = std::static_pointer_cast<ast::AST_ternary_exp_Node>(exp);
      decay_arr_to_pointer(nullptr, ternary->get_middle());
      ast::ElemType leftType = ternary->get_middle()->get_type();
      ast::ElemType rightType = exp->get_right()->get_type();
      auto leftDerivedType = ternary->get_middle()->get_derived_type();
      auto rightDerivedType = exp->get_right()->get_derived_type();
      auto [expType, expDerivedType] = ast::getParentType(
          leftType, rightType, leftDerivedType, rightDerivedType, exp);
      if (exp->get_factor_node() != nullptr and
          exp->get_factor_node()->get_type() == ast::ElemType::VOID) {
        success = false;
        error_messages.emplace_back(
            "Ternary operator not allowed on non-scalar types");
      }
      if (exp->get_left() != nullptr and
          exp->get_left()->get_type() == ast::ElemType::VOID) {
        success = false;
        error_messages.emplace_back(
            "Ternary operator not allowed on non-scalar types");
      }

      if (expType == ast::ElemType::NONE) {
        success = false;
        error_messages.emplace_back("Incompatible types in expression");
      }

      if (expType != rightType or expDerivedType != rightDerivedType) {
        add_cast_to_exp(exp->get_right(), expType, expDerivedType);
      }

      if (expType != leftType or expDerivedType != leftDerivedType) {
        add_cast_to_exp(ternary->get_middle(), expType, expDerivedType);
      }

      exp->set_type(expType);
      exp->set_derived_type(expDerivedType);
    } else if (binop == binop::BINOP::RIGHT_SHIFT or
               binop == binop::BINOP::LEFT_SHIFT) {
      ast::ElemType leftType = (exp->get_left() != nullptr)
                                   ? exp->get_left()->get_type()
                                   : exp->get_factor_node()->get_type();
      ast::ElemType rightType = exp->get_right()->get_type();
      // implicit promotion of char to int
      if (leftType == ast::ElemType::CHAR or leftType == ast::ElemType::UCHAR) {
        leftType = ast::ElemType::INT;
        (exp->get_left() != nullptr)
            ? add_cast_to_exp(exp->get_left(), ast::ElemType::INT, {})
            : add_cast_to_factor(exp->get_factor_node(), ast::ElemType::INT,
                                 {});
      }
      if (rightType == ast::ElemType::CHAR or
          rightType == ast::ElemType::UCHAR) {
        rightType = ast::ElemType::INT;
        add_cast_to_exp(exp->get_right(), ast::ElemType::INT, {});
      }
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
      } else if (leftType == ast::ElemType::VOID or
                 rightType == ast::ElemType::VOID) {
        success = false;
        error_messages.emplace_back(
            "Shift operator is not allowed on void types");
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
      if ((binop == binop::BINOP::ADD or binop == binop::BINOP::SUB) and
          (leftType == ast::ElemType::DERIVED or
           rightType == ast::ElemType::DERIVED)) {
        // pointer arithmetic
        if (leftType == ast::ElemType::DERIVED and
            rightType != ast::ElemType::DERIVED and
            rightType != ast::ElemType::DOUBLE) {
          if (rightType == ast::ElemType::VOID) {
            success = false;
            error_messages.emplace_back(
                "void value not ignored as it ought to be");
          } else if (rightType != ast::ElemType::LONG) {
            add_cast_to_exp(exp->get_right(), ast::ElemType::LONG, {});
          }
          exp->set_type(leftType);
          exp->set_derived_type(leftDerivedType);
        } else if (binop == binop::BINOP::ADD and
                   rightType == ast::ElemType::DERIVED and
                   leftType != ast::ElemType::DERIVED and
                   leftType != ast::ElemType::DOUBLE) {
          if (leftType == ast::ElemType::VOID) {
            success = false;
            error_messages.emplace_back(
                "void value not ignored as it ought to be");
          } else if (leftType != ast::ElemType::LONG) {
            exp->get_left() == nullptr
                ? add_cast_to_factor(exp->get_factor_node(),
                                     ast::ElemType::LONG, {})
                : add_cast_to_exp(exp->get_left(), ast::ElemType::LONG, {});
          }
          exp->set_type(rightType);
          exp->set_derived_type(rightDerivedType);
        } else if (binop == binop::BINOP::SUB and
                   rightDerivedType == leftDerivedType) {
          exp->set_type(ast::ElemType::LONG);
          exp->set_derived_type({});
        } else {
          success = false;
          error_messages.emplace_back(
              "Pointer arithmetic not allowed on derived types");
        }
      } else {
        auto [expType, expDerivedType] = ast::getParentType(
            leftType, rightType, leftDerivedType, rightDerivedType, exp);
        if (expType == ast::ElemType::NONE or expType == ast::ElemType::VOID) {
          success = false;
          error_messages.emplace_back("Incompatible types in expression");
        }

        if (expType != rightType or expDerivedType != rightDerivedType) {
          add_cast_to_exp(exp->get_right(), expType, expDerivedType);
        }

        if (expType != leftType or expDerivedType != leftDerivedType) {
          (exp->get_left() != nullptr)
              ? add_cast_to_exp(exp->get_left(), expType, expDerivedType)
              : add_cast_to_factor(exp->get_factor_node(), expType,
                                   expDerivedType);
        }

        if (binop::is_relational(binop)) {
          exp->set_type(ast::ElemType::INT);
        } else {
          exp->set_type(expType);
          exp->set_derived_type(expDerivedType);
        }
      }
    }

    if (exp->get_right()->get_type() == ast::ElemType::DERIVED) {
      if (binop == binop::BINOP::MOD) {
        success = false;
        error_messages.emplace_back(
            "Modulus operator not allowed on derived types");
      } else if (binop == binop::BINOP::DIV) {
        success = false;
        error_messages.emplace_back(
            "Division operator not allowed on derived types");
      } else if (binop == binop::BINOP::MUL) {
        success = false;
        error_messages.emplace_back(
            "Multiplication operator not allowed on derived types");
      } else if (binop == binop::BINOP::AOR) {
        success = false;
        error_messages.emplace_back("OR operator not allowed on derived types");
      } else if (binop == binop::BINOP::AAND) {
        success = false;
        error_messages.emplace_back(
            "AND operator not allowed on derived types");
      } else if (binop == binop::BINOP::XOR) {
        success = false;
        error_messages.emplace_back(
            "XOR operator not allowed on derived types");
      } else if (binop == binop::BINOP::LEFT_SHIFT) {
        success = false;
        error_messages.emplace_back(
            "Left shift operator not allowed on derived types");
      } else if (binop == binop::BINOP::RIGHT_SHIFT) {
        success = false;
        error_messages.emplace_back(
            "Right shift operator not allowed on derived types");
      }
    }
    if (binop != binop::BINOP::TERNARY) {
      ast::ElemType leftType =
          (exp->get_factor_node() != nullptr)
              ? exp->get_factor_node()->get_type()
              : (exp->get_left() != nullptr ? exp->get_left()->get_type()
                                            : ast::ElemType::NONE);
      auto leftDerivedType = (exp->get_factor_node() != nullptr)
                                 ? exp->get_factor_node()->get_derived_type()
                                 : (exp->get_left() != nullptr
                                        ? exp->get_left()->get_derived_type()
                                        : std::vector<long>{});

      if (leftType == ast::ElemType::DERIVED and
          leftDerivedType[0] == (long)ast::ElemType::POINTER and
          !ast::is_pointer_to_complete(leftType, leftDerivedType) and
          (binop == binop::BINOP::ADD or binop == binop::BINOP::SUB)) {
        success = false;
        error_messages.emplace_back(
            "Pointer arithmetic not allowed on incomplete type");
      } else if (leftType == ast::ElemType::VOID) {
        success = false;
        error_messages.emplace_back("void value not ignored as it ought to be");
      }

      auto rightType = (exp->get_right() != nullptr)
                           ? exp->get_right()->get_type()
                           : ast::ElemType::NONE;
      auto rightDerivedType = (exp->get_right() != nullptr)
                                  ? exp->get_right()->get_derived_type()
                                  : std::vector<long>{};

      if (rightType == ast::ElemType::DERIVED and
          rightDerivedType[0] == (long)ast::ElemType::POINTER and
          !ast::is_pointer_to_complete(rightType, rightDerivedType) and
          (binop == binop::BINOP::ADD or binop == binop::BINOP::SUB)) {
        success = false;
        error_messages.emplace_back(
            "Pointer arithmetic not allowed on incomplete type");
      } else if (rightType == ast::ElemType::VOID) {
        success = false;
        error_messages.emplace_back("void value not ignored as it ought to be");
      }
    }
    // left/right shift changed to logical left/right shift if the type is
    // unsigned
    if (binop == binop::BINOP::LEFT_SHIFT or
        binop == binop::BINOP::RIGHT_SHIFT) {
      if (exp->get_type() == ast::ElemType::UINT or
          exp->get_type() == ast::ElemType::ULONG) {
        if (binop == binop::BINOP::LEFT_SHIFT) {
          if (binop::is_compound(exp->get_binop_node()->get_op()))
            exp->get_binop_node()->set_op(
                binop::BINOP::COMPOUND_LOGICAL_LEFTSHIFT);
          else
            exp->get_binop_node()->set_op(binop::BINOP::LOGICAL_LEFT_SHIFT);
        } else {
          if (binop::is_compound(exp->get_binop_node()->get_op()))
            exp->get_binop_node()->set_op(
                binop::BINOP::COMPOUND_LOGICAL_RIGHTSHIFT);
          else
            exp->get_binop_node()->set_op(binop::BINOP::LOGICAL_RIGHT_SHIFT);
        }
      }
    }

    // check that modulus | Xor is not used on double precision
    if (exp->get_type() == ast::ElemType::DOUBLE) {
      if (binop == binop::BINOP::MOD) {
        success = false;
        error_messages.emplace_back(
            "Modulus operator not allowed on double precision");
      }

      if (binop == binop::BINOP::XOR) {
        success = false;
        error_messages.emplace_back(
            "XOR operator not allowed on double precision");
      }

      if (binop == binop::BINOP::AOR) {
        success = false;
        error_messages.emplace_back(
            "OR operator not allowed on double precision");
      }

      if (binop == binop::BINOP::AAND) {
        success = false;
        error_messages.emplace_back(
            "AND operator not allowed on double precision");
      }
    }
  }
}

void parser::add_cast_to_exp(std::shared_ptr<ast::AST_exp_Node> exp,
                             ast::ElemType type,
                             std::vector<long> derivedType) {
  if (exp->get_binop_node() == nullptr) {
    MAKE_SHARED(ast::AST_factor_Node, cast_factor);
    cast_factor->set_cast_type(type);
    cast_factor->set_type(type);
    cast_factor->set_derived_type(derivedType);
    cast_factor->set_child(exp->get_factor_node());

    exp->set_factor_node(std::move(cast_factor));
    exp->set_type(type);
    exp->set_derived_type(derivedType);
  } else {
    if (exp->get_binop_node()->get_op() == binop::BINOP::TERNARY) {
      auto ternary = std::static_pointer_cast<ast::AST_ternary_exp_Node>(exp);
      MAKE_SHARED(ast::AST_ternary_exp_Node, copy_ternary);
      copy_ternary->copy(exp);
      copy_ternary->set_middle(ternary->get_middle());

      MAKE_SHARED(ast::AST_factor_Node, cast_factor);
      cast_factor->set_cast_type(type);
      cast_factor->set_type(type);
      cast_factor->set_derived_type(derivedType);

      MAKE_SHARED(ast::AST_factor_Node, child_factor);
      child_factor->set_exp_node(std::move(copy_ternary));
      child_factor->set_type(exp->get_type());
      child_factor->set_derived_type(exp->get_derived_type());
      cast_factor->set_child(std::move(child_factor));

      ternary->set_middle(nullptr);
      exp->purge();
      exp->set_factor_node(std::move(cast_factor));
      exp->set_type(type);
      exp->set_derived_type(derivedType);
    } else {
      MAKE_SHARED(ast::AST_exp_Node, copy_exp);
      copy_exp->copy(exp);

      MAKE_SHARED(ast::AST_factor_Node, cast_factor);
      cast_factor->set_cast_type(type);
      cast_factor->set_type(type);
      cast_factor->set_derived_type(derivedType);

      MAKE_SHARED(ast::AST_factor_Node, child_factor);
      child_factor->set_exp_node(std::move(copy_exp));
      child_factor->set_type(exp->get_type());
      child_factor->set_derived_type(exp->get_derived_type());
      cast_factor->set_child(std::move(child_factor));

      exp->purge();
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
    copy_factor->copy(factor);
    copy_factor->set_arguments(funcCall->get_arguments());

    funcCall->set_arguments({});
    factor->purge();
    factor->set_cast_type(type);
    factor->set_type(type);
    factor->set_derived_type(derivedType);
    factor->set_child(std::move(copy_factor));
  } else {
    MAKE_SHARED(ast::AST_factor_Node, copy_factor);
    copy_factor->copy(factor);

    factor->purge();
    factor->set_cast_type(type);
    factor->set_type(type);
    factor->set_derived_type(derivedType);
    factor->set_child(std::move(copy_factor));
  }
}

void parser::decay_arr_to_pointer(std::shared_ptr<ast::AST_factor_Node> factor,
                                  std::shared_ptr<ast::AST_exp_Node> exp) {
  MAKE_SHARED(ast::AST_unop_Node, unop);
  unop->set_op(unop::UNOP::DECAY);

  if (factor != nullptr) {
    if (ast::is_array(factor)) {
      auto derivedType = factor->get_derived_type();
      derivedType[0] = (long)ast::ElemType::POINTER;

      add_cast_to_factor(factor, ast::ElemType::DERIVED, derivedType);
      factor->set_cast_type(ast::ElemType::NONE);
      factor->set_unop_node(unop);
    }
    return;
  }

  if (exp != nullptr) {
    if (ast::is_array(exp)) {
      auto derivedType = exp->get_derived_type();
      derivedType[0] = (long)ast::ElemType::POINTER;

      add_cast_to_exp(exp, ast::ElemType::DERIVED, derivedType);
      exp->get_factor_node()->set_cast_type(ast::ElemType::NONE);
      exp->get_factor_node()->set_unop_node(unop);
    }
  }
}

} // namespace parser
} // namespace scarlet
