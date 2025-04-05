#include <parser/common.hh>

namespace scarlet {
namespace parser {

#define INITZERO(type)                                                         \
  constant::Constant constZero;                                                \
  switch (type) {                                                              \
  case ast::ElemType::INT:                                                     \
    constZero.set_type(constant::Type::INT);                                   \
    constZero.set_value({.i = 0});                                             \
    break;                                                                     \
  case ast::ElemType::LONG:                                                    \
    constZero.set_type(constant::Type::LONG);                                  \
    constZero.set_value({.l = 0});                                             \
    break;                                                                     \
  case ast::ElemType::UINT:                                                    \
    constZero.set_type(constant::Type::UINT);                                  \
    constZero.set_value({.ui = 0});                                            \
    break;                                                                     \
  case ast::ElemType::ULONG:                                                   \
    constZero.set_type(constant::Type::ULONG);                                 \
    constZero.set_value({.ul = 0});                                            \
    break;                                                                     \
  case ast::ElemType::DOUBLE:                                                  \
    constZero.set_type(constant::Type::DOUBLE);                                \
    constZero.set_value({.d = 0});                                             \
    break;                                                                     \
  case ast::ElemType::DERIVED:                                                 \
    constZero.set_type(constant::Type::ULONG);                                 \
    constZero.set_value({.ul = 0});                                            \
    break;                                                                     \
  case ast::ElemType::POINTER:                                                 \
  case ast::ElemType::NONE:                                                    \
    UNREACHABLE();                                                             \
  }

void parser::analyze_declaration(
    std::shared_ptr<ast::AST_Declaration_Node> declaration,
    std::map<std::pair<std::string, int>, symbolTable::symbolInfo>
        &symbol_table,
    int indx) {
  std::string var_name = declaration->get_identifier()->get_value();
  if (declaration->get_type() == ast::DeclarationType::VARIABLE) {
    if (symbol_table.find({var_name, indx}) != symbol_table.end()) {
      // the symbol has been declared twice which is illegal
      // but if both the declarations are extern then it is legal
      if (symbol_table[{var_name, indx}].link ==
              symbolTable::linkage::EXTERNAL and
          declaration->get_specifier() == ast::SpecifierType::EXTERN) {
        auto varDecl =
            std::static_pointer_cast<ast::AST_variable_declaration_Node>(
                declaration);
        analyze_local_variable_declaration(varDecl, symbol_table, var_name,
                                           indx);
      } else {
        success = false;
        error_messages.emplace_back("Variable " + var_name +
                                    " already declared");
      }
    } else {
      auto varDecl =
          std::static_pointer_cast<ast::AST_variable_declaration_Node>(
              declaration);
      analyze_local_variable_declaration(varDecl, symbol_table, var_name, indx);
    }
  } else {
    // Make sure that function declarations at block scope don't have
    // static storage specifier.
    if (indx != 0 and
        declaration->get_specifier() == ast::SpecifierType::STATIC) {
      success = false;
      error_messages.emplace_back(
          "Function " + var_name +
          " declared with static storage specifier inside a block");
    }
    auto funcDecl =
        std::static_pointer_cast<ast::AST_function_declaration_Node>(
            declaration);
    analyze_function_declaration(funcDecl, symbol_table, var_name, indx);
  }
}

void parser::unroll_derived_type(
    std::shared_ptr<ast::AST_declarator_Node> declarator,
    std::vector<long> &derivedType) {
  if (declarator == nullptr)
    return;

  unroll_derived_type(declarator->get_child(), derivedType);

  if (declarator->is_pointer()) {
    derivedType.push_back((long)ast::ElemType::POINTER);
  }

  if (!declarator->get_arrDim().empty()) {
    for (auto dim : declarator->get_arrDim()) {
      derivedType.push_back(dim);
    }
  }
}

bool parser::previous_declaration_has_same_type(
    ast::ElemType prev_base_type, std::vector<long> prev_derived_type,
    std::shared_ptr<ast::AST_declarator_Node> curr_declarator,
    ast::ElemType curr_base_type) {
  std::vector<long> derivedType;
  unroll_derived_type(curr_declarator, derivedType);
  if (!derivedType.empty()) {
    derivedType.push_back((long)curr_base_type);
    curr_base_type = ast::ElemType::DERIVED;
  }

  if ((prev_base_type != curr_base_type) or (prev_derived_type != derivedType))
    return false;
  return true;
}

void parser::analyze_global_variable_declaration(
    std::shared_ptr<ast::AST_variable_declaration_Node> varDecl,
    std::map<std::pair<std::string, int>, symbolTable::symbolInfo>
        &symbol_table) {
  std::string var_name = varDecl->get_identifier()->get_value();

  // Check if the symbol has been declared before
  if (globalSymbolTable.find(var_name) != globalSymbolTable.end()) {
    // Ensure that the previous declaration has the same type
    if (globalSymbolTable[var_name].type != symbolTable::symbolType::VARIABLE) {
      success = false;
      error_messages.emplace_back(var_name +
                                  " redeclared as a different kind of symbol");
    } else {
      bool same_type = previous_declaration_has_same_type(
          globalSymbolTable[var_name].typeDef[0],
          globalSymbolTable[var_name].derivedTypeMap[0],
          varDecl->get_declarator(), varDecl->get_base_type());
      if (!same_type) {
        success = false;
        error_messages.emplace_back(
            var_name + " redeclared as a different kind of symbol");
      }
    }

    // If the symbol has been redefined, throw an error
    if (globalSymbolTable[var_name].def == symbolTable::defType::TRUE and
        varDecl->get_exp() != nullptr) {
      success = false;
      error_messages.emplace_back("Variable " + var_name +
                                  " has already been defined");
    }

    if (varDecl->get_specifier() == ast::SpecifierType::EXTERN) {
      // The linkage will be the same as the linkage of the previous
      // declaration.
    } else if (varDecl->get_specifier() == ast::SpecifierType::STATIC) {
      // If the previous declaration was external, throw an error
      if (globalSymbolTable[var_name].link == symbolTable::linkage::EXTERNAL) {
        success = false;
        error_messages.emplace_back(
            "Variable " + var_name +
            " declared with static storage specifier after being declared "
            "with external linkage");
      }
    } else {
      // If the previous declaration was internal, throw an error
      if (globalSymbolTable[var_name].link == symbolTable::linkage::INTERNAL) {
        success = false;
        error_messages.emplace_back("Variable " + var_name +
                                    " declared with external linkage after "
                                    "being declared with internal linkage");
      }
    }

    // If the previous declaration was because of a block scope extern
    // then it is possible that the symbol definition is not present in the
    // symbol table at index 0
    if (symbol_table.find({var_name, 0}) == symbol_table.end()) {
      symbol_table[{var_name, 0}] = globalSymbolTable[var_name];
    }

    // If the variable has been defined, check that it is initialized with a
    // constant integer
    if (varDecl->get_exp() != nullptr) {
      if (!EXPISCONSTANT(varDecl->get_exp())) {
        success = false;
        error_messages.emplace_back(
            "Variable " + var_name +
            " is not initialized with a constant integer");
      } else {
        globalSymbolTable[var_name].def = symbolTable::defType::TRUE;
        globalSymbolTable[var_name].value =
            ast::castConstToElemType(varDecl->get_exp()
                                         ->get_factor_node()
                                         ->get_const_node()
                                         ->get_constant(),
                                     globalSymbolTable[var_name].typeDef[0]);
        if (globalSymbolTable[var_name].typeDef[0] == ast::ElemType::DERIVED and
            globalSymbolTable[var_name].value.get_value().i != 0) {
          success = false;
          error_messages.emplace_back(
              "Invalid initialization of derived type " + var_name);
        }
        symbol_table[{var_name, 0}].def = symbolTable::defType::TRUE;
        symbol_table[{var_name, 0}].value = globalSymbolTable[var_name].value;
      }
    } else if (varDecl->get_specifier() != ast::SpecifierType::EXTERN) {
      // If the variable has not been defined and is not extern,
      // mark it as a tentative definition
      if (globalSymbolTable[var_name].def == symbolTable::defType::FALSE) {
        INITZERO(globalSymbolTable[var_name].typeDef[0]);
        symbol_table[{var_name, 0}].def = symbolTable::defType::TENTATIVE;
        globalSymbolTable[var_name].def = symbolTable::defType::TENTATIVE;
        symbol_table[{var_name, 0}].value = constZero;
        globalSymbolTable[var_name].value = constZero;
      }
    }
  } else { // symbol has not been declared before
    symbolTable::symbolInfo varInfo;
    varInfo.name = var_name;
    varInfo.link = symbolTable::linkage::EXTERNAL;
    varInfo.type = symbolTable::symbolType::VARIABLE;
    varInfo.def = symbolTable::defType::TENTATIVE;
    std::vector<long> derivedType;
    unroll_derived_type(varDecl->get_declarator(), derivedType);
    if (!derivedType.empty()) {
      derivedType.push_back((long)varDecl->get_base_type());
      varInfo.typeDef.push_back(ast::ElemType::DERIVED);
      varInfo.derivedTypeMap[0] = derivedType;
    } else {
      varInfo.typeDef.push_back(varDecl->get_base_type());
    }
    if (varDecl->get_specifier() == ast::SpecifierType::STATIC) {
      varInfo.link = symbolTable::linkage::INTERNAL;
    } else if (varDecl->get_specifier() == ast::SpecifierType::EXTERN) {
      varInfo.def = symbolTable::defType::FALSE;
    }

    // If storage specifier is not extern, set the tentative value to zero
    if (varDecl->get_specifier() != ast::SpecifierType::EXTERN) {
      INITZERO(varInfo.typeDef[0]);
      varInfo.value = constZero;
    }

    // Make sure that global variables are initialized only with
    // constant integers
    if (varDecl->get_exp() != nullptr) {
      varInfo.def = symbolTable::defType::TRUE;
      if (!EXPISCONSTANT(varDecl->get_exp())) {
        success = false;
        error_messages.emplace_back(
            "Global variable " + var_name +
            " is not initialized with a constant integer");
      } else {
        varInfo.value = ast::castConstToElemType(varDecl->get_exp()
                                                     ->get_factor_node()
                                                     ->get_const_node()
                                                     ->get_constant(),
                                                 varInfo.typeDef[0]);
        if (varInfo.typeDef[0] == ast::ElemType::DERIVED and
            varInfo.value.get_value().i != 0) {
          success = false;
          error_messages.emplace_back(
              "Invalid initialization of derived type " + var_name);
        }
      }
    }
    symbol_table[{var_name, 0}] = varInfo;
    globalSymbolTable[var_name] = varInfo;
  }
}

void parser::analyze_global_function_declaration(
    std::shared_ptr<ast::AST_function_declaration_Node> funcDecl,
    std::map<std::pair<std::string, int>, symbolTable::symbolInfo>
        &symbol_table) {
  if (funcDecl->get_block() == nullptr) {
    analyze_function_declaration(funcDecl, symbol_table, currFuncName, 0);
  } else {
    if (globalSymbolTable.find(currFuncName) != globalSymbolTable.end() and
        globalSymbolTable[currFuncName].def == symbolTable::defType::TRUE) {
      success = false;
      error_messages.emplace_back("Function " + currFuncName +
                                  " has already been defined");
    }
    analyze_function_declaration(funcDecl, symbol_table, currFuncName, 0);
    symbol_table[{currFuncName, 0}].def = symbolTable::defType::TRUE;
    globalSymbolTable[currFuncName].def = symbolTable::defType::TRUE;
    std::map<std::pair<std::string, int>, symbolTable::symbolInfo>
        proxy_symbol_table(symbol_table);
    for (int i = 0; i < (int)funcDecl->get_params().size(); i++) {
      auto param = funcDecl->get_params()[i];
      auto paramType = globalSymbolTable[currFuncName].typeDef[i + 1];
      auto paramDerivedType =
          globalSymbolTable[currFuncName].derivedTypeMap[i + 1];
      std::string paramName = param->identifier->get_value();
      std::string temp_name = get_temp_name(paramName);

      symbolTable::symbolInfo paramInfo;
      paramInfo.name = temp_name;
      paramInfo.link = symbolTable::linkage::NONE;
      paramInfo.type = symbolTable::symbolType::VARIABLE;
      paramInfo.typeDef.push_back(paramType);
      paramInfo.derivedTypeMap[0] = paramDerivedType;
      proxy_symbol_table[{paramName, 1}] = paramInfo;
      globalSymbolTable[temp_name] = paramInfo;
      param->identifier->set_identifier(temp_name);
    }
    analyze_block(funcDecl->get_block(), proxy_symbol_table, 1);
  }
}

void parser::analyze_function_declaration(
    std::shared_ptr<ast::AST_function_declaration_Node> funcDecl,
    std::map<std::pair<std::string, int>, symbolTable::symbolInfo>
        &symbol_table,
    std::string &var_name, int indx) {
  // Check that the function parameters always have different names.
  std::set<std::string> param_names;
  for (auto param : funcDecl->get_params()) {
    std::string paramName = param->identifier->get_value();
    if (param_names.find(paramName) != param_names.end()) {
      success = false;
      error_messages.emplace_back("Variable " + paramName +
                                  " already declared");
    } else {
      param_names.insert(paramName);
    }
  }

  // If the identifier has already been declared somewhere in the same
  // scope then make sure that it is also function declaration.
  if (symbol_table.find({var_name, indx}) != symbol_table.end()) {
    if (symbol_table[{var_name, indx}].type !=
        symbolTable::symbolType::FUNCTION) {
      success = false;
      error_messages.emplace_back(var_name +
                                  " redeclared as a different kind of symbol");
    }
  }

  // create the function type (return_type ++ arg_types)
  std::vector<ast::ElemType> funcType;
  std::map<int, std::vector<long>> funcDerivedTypeMap;
  for (int i = 0; i <= (int)funcDecl->get_params().size(); i++) {
    if (i == 0) {
      std::vector<long> derivedType;
      unroll_derived_type(funcDecl->get_declarator(), derivedType);
      if (!derivedType.empty()) {
        // function cannot return an array
        if (derivedType[0] > 0) {
          success = false;
          error_messages.emplace_back("function cannot return an array");
        }
        derivedType.push_back((long)funcDecl->get_return_type());
        funcType.push_back(ast::ElemType::DERIVED);
        funcDerivedTypeMap[i] = derivedType;
      } else {
        funcType.push_back(funcDecl->get_return_type());
      }
    } else {
      std::vector<long> derivedType;
      auto param = funcDecl->get_params()[i - 1];
      unroll_derived_type(param->declarator, derivedType);
      if (!derivedType.empty()) {
        if (derivedType[0] > 0) {
          // implicit conversion from array to pointer
          derivedType.erase(derivedType.begin());
          derivedType.insert(derivedType.begin(), (long)ast::ElemType::POINTER);
        }
        derivedType.push_back((long)param->base_type);
        funcType.push_back(ast::ElemType::DERIVED);
        funcDerivedTypeMap[i] = derivedType;
      } else {
        funcType.push_back(param->base_type);
      }
    }
  }

  // If the function has already been declared, make sure that it has the
  // same type as the current declaration and has an acceptable linkage.
  if (globalSymbolTable.find(var_name) != globalSymbolTable.end()) {
    if (globalSymbolTable[var_name].type != symbolTable::symbolType::FUNCTION or
        globalSymbolTable[var_name].typeDef.size() != funcType.size()) {
      success = false;
      error_messages.emplace_back(var_name +
                                  " redeclared as a different kind of symbol");
    } else {
      bool same_type = true;
      for (int i = 0; i <= (int)funcDecl->get_params().size(); i++) {
        if (globalSymbolTable[var_name].typeDef[i] != funcType[i]) {
          same_type = false;
          break;
        } else if (funcType[i] == ast::ElemType::DERIVED) {
          if (globalSymbolTable[var_name].derivedTypeMap[i] !=
              funcDerivedTypeMap[i]) {
            same_type = false;
            break;
          }
        }
      }
      if (same_type) {
        symbol_table[{var_name, indx}] = globalSymbolTable[var_name];
        if (funcDecl->get_specifier() == ast::SpecifierType::EXTERN or
            funcDecl->get_specifier() == ast::SpecifierType::NONE) {
          // This function declaration will have the same linkage as the earlier
          // function declaration. Hence we don't need to do anything.
        } else if (funcDecl->get_specifier() == ast::SpecifierType::STATIC) {
          if (globalSymbolTable[var_name].link ==
              symbolTable::linkage::EXTERNAL) {
            // Two declarations of the same identifier with different linkages
            // is illegal
            success = false;
            error_messages.emplace_back(
                "Function " + var_name +
                " declared with static storage specifier after being declared "
                "with external linkage");
          }
        }
      } else {
        success = false;
        error_messages.emplace_back(
            var_name + " redeclared as a different kind of symbol");
      }
    }
  } else {
    // Add the function declaration with the correct function type
    // and linkage into the symbol table
    symbolTable::symbolInfo funcInfo;
    funcInfo.name = var_name;
    funcInfo.link = symbolTable::linkage::EXTERNAL;
    funcInfo.type = symbolTable::symbolType::FUNCTION;
    funcInfo.typeDef = funcType;
    funcInfo.derivedTypeMap = funcDerivedTypeMap;
    if (funcDecl->get_specifier() == ast::SpecifierType::STATIC) {
      funcInfo.link = symbolTable::linkage::INTERNAL;
    }
    symbol_table[{var_name, indx}] = funcInfo;
    globalSymbolTable[var_name] = funcInfo;
  }
}

void parser::analyze_local_variable_declaration(
    std::shared_ptr<ast::AST_variable_declaration_Node> varDecl,
    std::map<std::pair<std::string, int>, symbolTable::symbolInfo>
        &symbol_table,
    std::string &var_name, int indx) {
  if (varDecl->get_specifier() == ast::SpecifierType::EXTERN) {
    // extern variables within block scope can't have initializers
    if (varDecl->get_exp() != nullptr) {
      success = false;
      error_messages.emplace_back("Variable " + var_name +
                                  " has both 'extern' and initializer");
    }
    if (globalSymbolTable.find(var_name) != globalSymbolTable.end()) {
      // If the variable has already been declared, then it should have the
      // same type
      if (globalSymbolTable[var_name].type !=
          symbolTable::symbolType::VARIABLE) {
        success = false;
        error_messages.emplace_back(
            var_name + " redeclared as a different kind of symbol");
      } else {
        bool same_type = previous_declaration_has_same_type(
            globalSymbolTable[var_name].typeDef[0],
            globalSymbolTable[var_name].derivedTypeMap[0],
            varDecl->get_declarator(), varDecl->get_base_type());
        if (!same_type) {
          success = false;
          error_messages.emplace_back(
              var_name + " redeclared as a different kind of symbol");
        }
      }
      // do nothing as the variable has already been declared
      symbol_table[{var_name, indx}] = globalSymbolTable[var_name];
    } else {
      symbolTable::symbolInfo varInfo;
      varInfo.name = var_name;
      varInfo.link = symbolTable::linkage::EXTERNAL;
      varInfo.type = symbolTable::symbolType::VARIABLE;
      std::vector<long> derivedType;
      unroll_derived_type(varDecl->get_declarator(), derivedType);
      if (!derivedType.empty()) {
        derivedType.push_back((long)varDecl->get_base_type());
        varInfo.typeDef.push_back(ast::ElemType::DERIVED);
        varInfo.derivedTypeMap[0] = derivedType;
      } else {
        varInfo.typeDef.push_back(varDecl->get_base_type());
      }
      symbol_table[{var_name, indx}] = varInfo;
      globalSymbolTable[var_name] = varInfo;
    }
  } else if (varDecl->get_specifier() == ast::SpecifierType::STATIC) {
    std::string temp_name = get_temp_name(var_name);
    varDecl->get_identifier()->set_identifier(temp_name);

    symbolTable::symbolInfo varInfo;
    varInfo.name = temp_name;
    varInfo.link = symbolTable::linkage::INTERNAL;
    varInfo.type = symbolTable::symbolType::VARIABLE;
    std::vector<long> derivedType;
    unroll_derived_type(varDecl->get_declarator(), derivedType);
    if (!derivedType.empty()) {
      derivedType.push_back((long)varDecl->get_base_type());
      varInfo.typeDef.push_back(ast::ElemType::DERIVED);
      varInfo.derivedTypeMap[0] = derivedType;
    } else {
      varInfo.typeDef.push_back(varDecl->get_base_type());
    }
    varInfo.def = symbolTable::defType::TRUE;
    if (varDecl->get_exp() != nullptr) {
      if (!EXPISCONSTANT(varDecl->get_exp())) {
        success = false;
        error_messages.emplace_back(
            "Global variable " + var_name +
            " is not initialized with a constant integer");
      } else {
        varInfo.value = ast::castConstToElemType(varDecl->get_exp()
                                                     ->get_factor_node()
                                                     ->get_const_node()
                                                     ->get_constant(),
                                                 varInfo.typeDef[0]);
        if (varInfo.typeDef[0] == ast::ElemType::DERIVED and
            varInfo.value.get_value().i != 0) {
          success = false;
          error_messages.emplace_back(
              "Invalid initialization of derived type " + var_name);
        }
      }
    } else {
      INITZERO(varInfo.typeDef[0]);
      varInfo.value = constZero;
    }
    symbol_table[{var_name, indx}] = varInfo;
    globalSymbolTable[temp_name] = varInfo;
  } else {
    std::string temp_name = get_temp_name(var_name);
    varDecl->get_identifier()->set_identifier(temp_name);

    symbolTable::symbolInfo varInfo;
    varInfo.name = temp_name;
    varInfo.link = symbolTable::linkage::NONE;
    varInfo.type = symbolTable::symbolType::VARIABLE;
    std::vector<long> derivedType;
    unroll_derived_type(varDecl->get_declarator(), derivedType);
    if (!derivedType.empty()) {
      derivedType.push_back((long)varDecl->get_base_type());
      varInfo.typeDef.push_back(ast::ElemType::DERIVED);
      varInfo.derivedTypeMap[0] = derivedType;
    } else {
      varInfo.typeDef.push_back(varDecl->get_base_type());
    }
    symbol_table[{var_name, indx}] = varInfo;
    globalSymbolTable[temp_name] = varInfo;
    if (varDecl->get_exp() != nullptr) {
      symbol_table[{var_name, indx}].def = symbolTable::defType::TRUE;
      globalSymbolTable[temp_name].def = symbolTable::defType::TRUE;
      analyze_exp(varDecl->get_exp(), symbol_table, indx);
      auto expType = varDecl->get_exp()->get_type();
      auto expDerivedType = varDecl->get_exp()->get_derived_type();
      auto [castType, castDerivedType] =
          ast::getAssignType(varInfo.typeDef[0], varInfo.derivedTypeMap[0],
                             expType, expDerivedType, varDecl->get_exp());
      if (castType == ast::ElemType::NONE) {
        success = false;
        error_messages.emplace_back("Invalid assignment to variable " +
                                    var_name);
      } else {
        if (castType != expType or castDerivedType != expDerivedType) {
          add_cast_to_exp(varDecl->get_exp(), varInfo.typeDef[0],
                          varInfo.derivedTypeMap[0]);
        }
      }
    }
  }
}

} // namespace parser
} // namespace scarlet
