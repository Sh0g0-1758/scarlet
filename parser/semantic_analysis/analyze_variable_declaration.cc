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
  case ast::ElemType::CHAR:                                                    \
    constZero.set_type(constant::Type::CHAR);                                  \
    constZero.set_value({.c = 0});                                             \
    break;                                                                     \
  case ast::ElemType::UCHAR:                                                   \
    constZero.set_type(constant::Type::UCHAR);                                 \
    constZero.set_value({.uc = 0});                                            \
    break;                                                                     \
  case ast::ElemType::STRUCT:                                                  \
  case ast::ElemType::DERIVED:                                                 \
  case ast::ElemType::POINTER:                                                 \
    constZero.set_type(constant::Type::ULONG);                                 \
    constZero.set_value({.ul = 0});                                            \
    break;                                                                     \
  case ast::ElemType::VOID:                                                    \
    break;                                                                     \
  case ast::ElemType::NONE:                                                    \
    UNREACHABLE();                                                             \
  }

void parser::analyze_global_variable_declaration(
    std::shared_ptr<ast::AST_variable_declaration_Node> varDecl,
    std::map<std::pair<std::string, int>, symbolTable::symbolInfo>
        &symbol_table) {
  std::string var_name = varDecl->get_identifier()->get_value();

  // Check if the symbol has been declared before
  if (globalSymbolTable.find(var_name) != globalSymbolTable.end()) {
    auto varInfo = globalSymbolTable[var_name];
    // Ensure that the previous declaration has the same type
    if (varInfo.type != symbolTable::symbolType::VARIABLE) {
      success = false;
      error_messages.emplace_back(var_name +
                                  " redeclared as a different kind of symbol");
    } else {
      bool same_type = previous_declaration_has_same_type(
          varInfo.typeDef[0], varInfo.derivedTypeMap[0],
          varDecl->get_declarator(), varDecl->get_base_type());
      if (!same_type) {
        success = false;
        error_messages.emplace_back(
            var_name + " redeclared as a different kind of symbol");
      }
    }

    if (!ast::validate_type_specifier(
            varInfo.typeDef[0], varInfo.derivedTypeMap[0], symbol_table, "",0)) {
      success = false;
      error_messages.emplace_back("Variable " + var_name +
                                  " cannot be declared as incomplete type");
    }

    // If the symbol has been redefined, throw an error
    if (varInfo.def == symbolTable::defType::TRUE and
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
      if (varInfo.link == symbolTable::linkage::EXTERNAL) {
        success = false;
        error_messages.emplace_back(
            "Variable " + var_name +
            " declared with static storage specifier after being declared "
            "with external linkage");
      }
    } else {
      // If the previous declaration was internal, throw an error
      if (varInfo.link == symbolTable::linkage::INTERNAL) {
        success = false;
        error_messages.emplace_back("Variable " + var_name +
                                    " declared with external linkage after "
                                    "being declared with internal linkage");
      }
    }

    if (varInfo.def != symbolTable::defType::TRUE) {
      initialize_global_variable(varInfo, varDecl, var_name);
    }

    if (varDecl->get_specifier() != ast::SpecifierType::EXTERN) {
      // If the variable has not been defined and is not extern,
      // mark it as a tentative definition
      if (varInfo.def == symbolTable::defType::FALSE) {
        varInfo.def = symbolTable::defType::TENTATIVE;
      }
    }

    symbol_table[{var_name, 0}] = varInfo;
    globalSymbolTable[var_name] = varInfo;
  } else { // symbol has not been declared before
    symbolTable::symbolInfo varInfo;
    varInfo.name = var_name;
    varInfo.link = symbolTable::linkage::EXTERNAL;
    varInfo.type = symbolTable::symbolType::VARIABLE;
    varInfo.def = symbolTable::defType::TENTATIVE;
    std::vector<long> derivedType;
    ast::unroll_derived_type(varDecl->get_declarator(), derivedType);
    if (!derivedType.empty()) {
      derivedType.push_back((long)varDecl->get_base_type());
      varInfo.typeDef.push_back(ast::ElemType::DERIVED);
      varInfo.derivedTypeMap[0] = derivedType;
    } else {
      varInfo.typeDef.push_back(varDecl->get_base_type());
    }

    if (!ast::validate_type_specifier(
            varInfo.typeDef[0], varInfo.derivedTypeMap[0], symbol_table, "",0)) {
      success = false;
      error_messages.emplace_back("Variable " + var_name +
                                  " cannot be declared as incomplete type");
    }

    if (varDecl->get_specifier() == ast::SpecifierType::STATIC) {
      varInfo.link = symbolTable::linkage::INTERNAL;
    } else if (varDecl->get_specifier() == ast::SpecifierType::EXTERN) {
      varInfo.def = symbolTable::defType::FALSE;
    }

    initialize_global_variable(varInfo, varDecl, var_name);

    if (varDecl->get_specifier() != ast::SpecifierType::EXTERN) {
      if (varInfo.def == symbolTable::defType::FALSE) {
        varInfo.def = symbolTable::defType::TENTATIVE;
      }
    }

    symbol_table[{var_name, 0}] = varInfo;
    globalSymbolTable[var_name] = varInfo;
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
      ast::unroll_derived_type(varDecl->get_declarator(), derivedType);
      if (!derivedType.empty()) {
        derivedType.push_back((long)varDecl->get_base_type());
        varInfo.typeDef.push_back(ast::ElemType::DERIVED);
        varInfo.derivedTypeMap[0] = derivedType;
      } else {
        varInfo.typeDef.push_back(varDecl->get_base_type());
      }
      varInfo.struct_identifier.set_string(
          varDecl->get_struct_identifier() == nullptr
              ? ""
              : varDecl->get_struct_identifier()->get_value());

      if (!ast::validate_type_specifier(varInfo.typeDef[0],
                                        varInfo.derivedTypeMap[0], symbol_table,
                                        "",indx)) {
        if(varInfo.typeDef[0] != ast::ElemType::STRUCT and (varInfo.typeDef[0] != ast::ElemType::DERIVED or
           (varInfo.derivedTypeMap[0].size() > 0 and
           varInfo.derivedTypeMap[0][varInfo.derivedTypeMap[0].size() - 1] !=
               (long)ast::ElemType::STRUCT))) {
          success = false;
          error_messages.emplace_back("Variable " + var_name +
                                      " cannot be declared as incomplete type");
        }
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
    ast::unroll_derived_type(varDecl->get_declarator(), derivedType);
    if (!derivedType.empty()) {
      derivedType.push_back((long)varDecl->get_base_type());
      varInfo.typeDef.push_back(ast::ElemType::DERIVED);
      varInfo.derivedTypeMap[0] = derivedType;
    } else {
      varInfo.typeDef.push_back(varDecl->get_base_type());
    }

    if (!ast::validate_type_specifier(
            varInfo.typeDef[0], varInfo.derivedTypeMap[0], symbol_table, "",indx)) {
      success = false;
      error_messages.emplace_back("Variable " + var_name +
                                  " cannot be declared as incomplete type");
    }

    varInfo.def = symbolTable::defType::TRUE;
    initialize_global_variable(varInfo, varDecl, temp_name);
    symbol_table[{var_name, indx}] = varInfo;
    globalSymbolTable[temp_name] = varInfo;
  } else {
    std::string temp_name = get_temp_name(var_name);
    varDecl->get_identifier()->set_identifier(temp_name);

    symbolTable::symbolInfo varInfo;
    varInfo.name = temp_name;
    varInfo.link = symbolTable::linkage::NONE;
    varInfo.type = symbolTable::symbolType::VARIABLE;
    constant::Constant structId;
    structId.set_string(varDecl->get_struct_identifier() == nullptr
                            ? ""
                            : varDecl->get_struct_identifier()->get_value());
    varInfo.struct_identifier = structId;
    std::vector<long> derivedType;
    ast::unroll_derived_type(varDecl->get_declarator(), derivedType);
    if (!derivedType.empty()) {
      derivedType.push_back((long)varDecl->get_base_type());
      varInfo.typeDef.push_back(ast::ElemType::DERIVED);
      varInfo.derivedTypeMap[0] = derivedType;
    } else {
      varInfo.typeDef.push_back(varDecl->get_base_type());
    }

    if (!ast::is_valid_declarator(
            varInfo.typeDef[0], varInfo.derivedTypeMap[0], symbol_table,
            varDecl->get_struct_identifier() == nullptr
                ? ""
                : varDecl->get_struct_identifier()->get_value(),indx)) {
      success = false;
      error_messages.emplace_back("Variable " + var_name + " cannot be declared with incomplete or void type");
    }
    symbol_table[{var_name, indx}] = varInfo;
    globalSymbolTable[temp_name] = varInfo;
    if (varDecl->get_exp() != nullptr) {
      symbol_table[{var_name, indx}].def = symbolTable::defType::TRUE;
      globalSymbolTable[temp_name].def = symbolTable::defType::TRUE;

      if (symbolTable::symbolInfo::is_array(varInfo)) {
        std::vector<long> arrDim{};
        ast::ElemType baseElemType{};
        std::vector<long> derivedElemType{};
        get_arrInfo(arrDim, baseElemType, derivedElemType, varInfo);
        // It is possible to initialize an array with a string literal
        if (ast::exp_is_string(varDecl->get_exp()) and
            (baseElemType == ast::ElemType::CHAR or
             baseElemType == ast::ElemType::UCHAR)) {
          if (arrDim.size() > 1) {
            success = false;
            error_messages.emplace_back("Cannot initialize multi-dimensional "
                                        "array with a string literal");
          }
          std::string str = varDecl->get_exp()
                                ->get_factor_node()
                                ->get_const_node()
                                ->get_constant()
                                .get_string();
          if ((int)str.size() > arrDim[0]) {
            success = false;
            error_messages.emplace_back(
                "String literal is larger than the array size");
          }

          if (!success)
            return;

          MAKE_SHARED(ast::initializer, init);
          for (auto ch : str) {
            MAKE_SHARED(ast::AST_exp_Node, exp);
            MAKE_SHARED(ast::AST_factor_Node, factor);
            MAKE_SHARED(ast::AST_const_Node, constNode);
            constant::Constant c;
            c.set_type(constant::Type::CHAR);
            c.set_value({.c = ch});
            constNode->set_constant(c);
            factor->set_const_node(std::move(constNode));
            factor->set_type(ast::ElemType::CHAR);
            exp->set_factor_node(std::move(factor));
            exp->set_type(ast::ElemType::CHAR);
            init->exp_list.emplace_back(std::move(exp));
          }
          varDecl->set_initializer(std::move(init));
          varDecl->set_exp(nullptr);
          analyze_array_initializer(varDecl->get_initializer(), symbol_table,
                                    indx, arrDim, baseElemType, varDecl->get_struct_identifier() == nullptr
                                        ? ""
                                        : varDecl->get_struct_identifier()
                                              ->get_value(),
                                    derivedElemType);
        } else {
          success = false;
          error_messages.emplace_back("Cannot initialize array with an "
                                      "expression, need an initializer list");
        }
        return;
      }

      analyze_exp(varDecl->get_exp(), symbol_table, indx);
      decay_arr_to_pointer(nullptr, varDecl->get_exp());

      auto expType = varDecl->get_exp()->get_type();
      auto expDerivedType = varDecl->get_exp()->get_derived_type();
      auto expStructName =
          varDecl->get_exp()->get_struct_identifier() == nullptr
              ? ""
              : varDecl->get_exp()->get_struct_identifier()->get_value();
      auto [castType, castDerivedType] =
          ast::getAssignType(varInfo.typeDef[0], varInfo.derivedTypeMap[0], varInfo.struct_identifier.get_string(),
                             expType, expDerivedType,expStructName,varDecl->get_exp());

      if (castType == ast::ElemType::NONE) {
        success = false;
        error_messages.emplace_back("Invalid assignment to variable " +
                                    var_name);
      } else {
        if (castType != expType or castDerivedType != expDerivedType) {
          add_cast_to_exp(varDecl->get_exp(), castType, castDerivedType);
        }
      }
    } else if (varDecl->get_initializer() != nullptr) {
      symbol_table[{var_name, indx}].def = symbolTable::defType::TRUE;
      globalSymbolTable[temp_name].def = symbolTable::defType::TRUE;
      if (symbolTable::symbolInfo::is_array(varInfo)) {
        std::vector<long> arrDim{};
        ast::ElemType baseElemType{};
        std::vector<long> derivedElemType{};
        get_arrInfo(arrDim, baseElemType, derivedElemType, varInfo);
        analyze_array_initializer(varDecl->get_initializer(), symbol_table,
                                  indx, arrDim, baseElemType, varDecl->get_struct_identifier() == nullptr ? "" : varDecl->get_struct_identifier()->get_value() ,derivedElemType);
      } else {
        success = false;
        error_messages.emplace_back("Invalid use of initializer list, it can "
                                    "only be used to initialize arrays");
      }
    }
  }
}

void parser::initialize_global_variable(
    symbolTable::symbolInfo &varInfo,
    std::shared_ptr<ast::AST_variable_declaration_Node> varDecl,
    std::string &var_name) {
  if (symbolTable::symbolInfo::is_array(varInfo)) {
    std::vector<long> arrDim{};
    ast::ElemType baseElemType{};
    std::vector<long> derivedElemType{};
    get_arrInfo(arrDim, baseElemType, derivedElemType, varInfo);

    if (varDecl->get_exp() != nullptr) {
      if (ast::exp_is_string(varDecl->get_exp()) and
          (baseElemType == ast::ElemType::CHAR or
           baseElemType == ast::ElemType::UCHAR)) {
        if (arrDim.size() > 1) {
          success = false;
          error_messages.emplace_back("Cannot initialize multi-dimensional "
                                      "array with a string literal");
        }
        std::string str = varDecl->get_exp()
                              ->get_factor_node()
                              ->get_const_node()
                              ->get_constant()
                              .get_string();
        if ((int)str.size() > arrDim[0]) {
          success = false;
          error_messages.emplace_back(
              "String literal is larger than the array size");
        }

        if (!success)
          return;

        MAKE_SHARED(ast::initializer, init);
        for (auto ch : str) {
          MAKE_SHARED(ast::AST_exp_Node, exp);
          MAKE_SHARED(ast::AST_factor_Node, factor);
          MAKE_SHARED(ast::AST_const_Node, constNode);
          constant::Constant c;
          c.set_type(constant::Type::CHAR);
          c.set_value({.c = ch});
          constNode->set_constant(c);
          factor->set_const_node(std::move(constNode));
          factor->set_type(ast::ElemType::CHAR);
          exp->set_factor_node(std::move(factor));
          exp->set_type(ast::ElemType::CHAR);
          init->exp_list.emplace_back(std::move(exp));
        }
        varDecl->set_initializer(std::move(init));
        varDecl->set_exp(nullptr);
        init_static_array_initializer(varDecl->get_initializer(), arrDim,
                                      baseElemType, derivedElemType, varInfo);
      } else {
        success = false;
        error_messages.emplace_back("Cannot initialize array with an "
                                    "expression, need an initializer list");
      }
    } else if (varDecl->get_initializer() != nullptr) {
      varInfo.def = symbolTable::defType::TRUE;
      init_static_array_initializer(varDecl->get_initializer(), arrDim,
                                    baseElemType, derivedElemType, varInfo);
    } else {
      constant::Constant constZero;
      constZero.set_type(constant::Type::ZERO);
      unsigned long num_bytes = ast::getSizeOfTypeOnArch(baseElemType);
      for (auto dim : arrDim) {
        num_bytes *= dim;
      }
      constZero.set_value({.ul = num_bytes});
      varInfo.value.push_back(constZero);
    }
  } else {
    if (varDecl->get_initializer() != nullptr) {
      success = false;
      error_messages.emplace_back("Invalid use of initializer list, it can "
                                  "only be used to initialize arrays");
    } else if (varDecl->get_exp() != nullptr) {
      if (varInfo.value.size() == 0)
        varInfo.value.resize(1);
      varInfo.def = symbolTable::defType::TRUE;
      if (!EXPISCONSTANT(varDecl->get_exp())) {
        success = false;
        error_messages.emplace_back(
            "Global variable " + var_name +
            " is not initialized with a constant integer");
      } else {
        if (ast::exp_is_string(varDecl->get_exp())) {
          // string literal can only be used to assign to a pointer to a char or
          // an array of char
          if (varInfo.typeDef[0] == ast::ElemType::DERIVED and
              varInfo.derivedTypeMap[0][0] == (long)ast::ElemType::POINTER and
              varInfo.derivedTypeMap[0][1] == (long)ast::ElemType::CHAR) {
            varInfo.value[0] = varDecl->get_exp()
                                   ->get_factor_node()
                                   ->get_const_node()
                                   ->get_constant();
          } else {
            success = false;
            error_messages.emplace_back(
                "Invalid initialization of " + var_name +
                ". String literal can only be used to assign to a pointer to a "
                "char or an array of char");
          }
        } else {
          varInfo.value[0] = ast::castConstToElemType(varDecl->get_exp()
                                                          ->get_factor_node()
                                                          ->get_const_node()
                                                          ->get_constant(),
                                                      varInfo.typeDef[0]);
          if (varInfo.typeDef[0] == ast::ElemType::DERIVED and
              varInfo.value[0].get_value().i != 0) {
            success = false;
            error_messages.emplace_back(
                "Invalid initialization of derived type " + var_name);
          }
        }
      }
    } else {
      if (varInfo.value.size() == 0)
        varInfo.value.resize(1);
      INITZERO(varInfo.typeDef[0]);
      varInfo.value[0] = constZero;
    }
  }
}

void parser::init_static_array_initializer(
    std::shared_ptr<ast::initializer> init, std::vector<long> arrDim,
    ast::ElemType baseElemType, std::vector<long> derivedElemType,
    symbolTable::symbolInfo &varInfo) {
  if (init == nullptr)
    return;

  if (baseElemType == ast::ElemType::CHAR or
      baseElemType == ast::ElemType::UCHAR) {
    for (int i = 0; i < (int)init->exp_list.size(); i++) {
      auto exp = init->exp_list[i];
      if (ast::exp_is_string(exp)) {
        auto str = exp->get_factor_node()
                       ->get_const_node()
                       ->get_constant()
                       .get_string();

        MAKE_SHARED(ast::initializer, string_init);
        for (auto ch : str) {
          MAKE_SHARED(ast::AST_exp_Node, exp);
          MAKE_SHARED(ast::AST_factor_Node, factor);
          MAKE_SHARED(ast::AST_const_Node, constNode);
          constant::Constant c;
          c.set_type(constant::Type::CHAR);
          c.set_value({.c = ch});
          constNode->set_constant(c);
          factor->set_const_node(std::move(constNode));
          factor->set_type(ast::ElemType::CHAR);
          exp->set_factor_node(std::move(factor));
          exp->set_type(ast::ElemType::CHAR);
          string_init->exp_list.emplace_back(std::move(exp));
        }

        init->initializer_list.insert(
            init->initializer_list.begin() + init->exp_indx[i], string_init);
      }
    }
  }

  if (!(init->initializer_list.empty())) {
    long currDim = arrDim[0];
    if ((long)init->initializer_list.size() > currDim) {
      success = false;
      error_messages.emplace_back(
          "Wrong number of elements in the initializer list");
    }
    arrDim.erase(arrDim.begin());
    for (int i = 0; i < (long)init->initializer_list.size(); i++) {
      currDim--;
      init_static_array_initializer(init->initializer_list[i], arrDim,
                                    baseElemType, derivedElemType, varInfo);
    }
    if (currDim != 0) {
      unsigned long num_bytes = ast::getSizeOfTypeOnArch(baseElemType);
      for (auto dim : arrDim) {
        num_bytes *= dim;
      }
      num_bytes *= currDim;
      constant::Constant constZero;
      constZero.set_type(constant::Type::ZERO);
      constZero.set_value({.ul = num_bytes});
      varInfo.value.push_back(constZero);
    }
  } else if (!(init->exp_list.empty())) {
    if (arrDim.size() != 1) {
      success = false;
      error_messages.emplace_back("Invalid use of initializer list, it does "
                                  "not respect the array dimensions");
    }
    if ((long)init->exp_list.size() > arrDim[0]) {
      success = false;
      error_messages.emplace_back(
          "Wrong number of elements in the initializer list");
    }
    unsigned long num_left = arrDim[0];
    for (int i = 0; i < (int)init->exp_list.size(); i++) {
      num_left--;
      auto child_exp = init->exp_list[i];
      if (!EXPISCONSTANT(child_exp)) {
        success = false;
        error_messages.emplace_back(
            "Static Array initializer list can only contain constant integers");
      } else {
        varInfo.value.push_back(ast::castConstToElemType(
            child_exp->get_factor_node()->get_const_node()->get_constant(),
            baseElemType));
        if (baseElemType == ast::ElemType::DERIVED and
            (varInfo.value[0].get_value().i != 0 or
             child_exp->get_factor_node()
                     ->get_const_node()
                     ->get_constant()
                     .get_type() == constant::Type::DOUBLE)) {
          success = false;
          error_messages.emplace_back("Invalid initialization of derived type "
                                      "in static array initializer");
        }
      }
    }
    if (num_left != 0) {
      unsigned long num_bytes =
          num_left * ast::getSizeOfTypeOnArch(baseElemType);
      constant::Constant constZero;
      constZero.set_type(constant::Type::ZERO);
      constZero.set_value({.ul = num_bytes});
      varInfo.value.push_back(constZero);
    }
  } else {
    unsigned long num_bytes = ast::getSizeOfTypeOnArch(baseElemType);
    for (auto dim : arrDim) {
      num_bytes *= dim;
    }
    constant::Constant constZero;
    constZero.set_type(constant::Type::ZERO);
    constZero.set_value({.ul = num_bytes});
    varInfo.value.push_back(constZero);
  }
}

void parser::analyze_array_initializer(
    std::shared_ptr<ast::initializer> init,
    std::map<std::pair<std::string, int>, symbolTable::symbolInfo>
        &symbol_table,
    int indx, std::vector<long> arrDim, ast::ElemType baseElemType, std::string struct_identifier,
    std::vector<long> derivedElemType) {
  if (init == nullptr)
    return;

  if (baseElemType == ast::ElemType::CHAR or
      baseElemType == ast::ElemType::UCHAR) {
    for (int i = 0; i < (int)init->exp_list.size(); i++) {
      auto exp = init->exp_list[i];
      if (ast::exp_is_string(exp)) {
        auto str = exp->get_factor_node()
                       ->get_const_node()
                       ->get_constant()
                       .get_string();

        MAKE_SHARED(ast::initializer, string_init);
        for (auto ch : str) {
          MAKE_SHARED(ast::AST_exp_Node, exp);
          MAKE_SHARED(ast::AST_factor_Node, factor);
          MAKE_SHARED(ast::AST_const_Node, constNode);
          constant::Constant c;
          c.set_type(constant::Type::CHAR);
          c.set_value({.c = ch});
          constNode->set_constant(c);
          factor->set_const_node(std::move(constNode));
          factor->set_type(ast::ElemType::CHAR);
          exp->set_factor_node(std::move(factor));
          exp->set_type(ast::ElemType::CHAR);
          string_init->exp_list.emplace_back(std::move(exp));
        }

        init->initializer_list.insert(
            init->initializer_list.begin() + init->exp_indx[i], string_init);
      }
    }
  }

  if (!(init->initializer_list.empty())) {
    long currDim = arrDim[0];
    if ((long)init->initializer_list.size() > currDim) {
      success = false;
      error_messages.emplace_back(
          "Wrong number of elements in the initializer list");
    }
    arrDim.erase(arrDim.begin());
    long i = 0;
    for (; i < (long)init->initializer_list.size(); i++) {
      analyze_array_initializer(init->initializer_list[i], symbol_table, indx,
                                arrDim, baseElemType,struct_identifier, derivedElemType);
    }
    for (; i < currDim; i++) {
      MAKE_SHARED(ast::initializer, child_init);
      analyze_array_initializer(child_init, symbol_table, indx, arrDim,
                                baseElemType,struct_identifier, derivedElemType);
      init->initializer_list.push_back(child_init);
    }
  } else if (!(init->exp_list.empty())) {
    if (arrDim.size() != 1) {
      success = false;
      error_messages.emplace_back("Invalid use of initializer list, it does "
                                  "not respect the array dimensions");
    }
    if ((long)init->exp_list.size() > arrDim[0]) {
      success = false;
      error_messages.emplace_back(
          "Wrong number of elements in the initializer list");
    }
    long i = 0;
    for (; i < (long)init->exp_list.size(); i++) {
      auto child_exp = init->exp_list[i];
      analyze_exp(child_exp, symbol_table, indx);
      decay_arr_to_pointer(nullptr, child_exp);
      auto expType = child_exp->get_type();
      auto expDerivedType = child_exp->get_derived_type();
      auto expStructName =
          child_exp->get_struct_identifier() == nullptr
              ? ""
              : child_exp->get_struct_identifier()->get_value();
      auto [castType, castDerivedType] = ast::getAssignType(
          baseElemType, derivedElemType, struct_identifier,expType, expDerivedType,expStructName,child_exp);
      if (castType == ast::ElemType::NONE) {
        success = false;
        error_messages.emplace_back("Invalid type in initializer list");
      } else {
        if (castType != expType or castDerivedType != expDerivedType) {
          add_cast_to_exp(child_exp, castType, castDerivedType);
        }
      }
    }
    for (; i < arrDim[0]; i++) {
      MAKE_SHARED(ast::AST_exp_Node, child_exp);
      child_exp->set_type(baseElemType);
      child_exp->set_derived_type(derivedElemType);

      MAKE_SHARED(ast::AST_factor_Node, child_factor);
      child_factor->set_type(baseElemType);
      child_factor->set_derived_type(derivedElemType);

      MAKE_SHARED(ast::AST_const_Node, child_zero);
      INITZERO(baseElemType);
      child_zero->set_constant(constZero);

      child_factor->set_const_node(child_zero);
      child_exp->set_factor_node(child_factor);

      init->exp_list.push_back(child_exp);
    }
  } else if (arrDim.size() > 1) {
    int currDim = arrDim[0];
    arrDim.erase(arrDim.begin());
    for (long i = 0; i < currDim; i++) {
      MAKE_SHARED(ast::initializer, child_init);
      analyze_array_initializer(child_init, symbol_table, indx, arrDim,
                                baseElemType, struct_identifier,derivedElemType);
      init->initializer_list.push_back(child_init);
    }
  } else if (arrDim.size() == 1) {
    for (long i = 0; i < arrDim[0]; i++) {
      MAKE_SHARED(ast::AST_exp_Node, child_exp);
      child_exp->set_type(baseElemType);
      child_exp->set_derived_type(derivedElemType);

      MAKE_SHARED(ast::AST_factor_Node, child_factor);
      child_factor->set_type(baseElemType);
      child_factor->set_derived_type(derivedElemType);

      MAKE_SHARED(ast::AST_const_Node, child_zero);
      INITZERO(baseElemType);
      child_zero->set_constant(constZero);

      child_factor->set_const_node(child_zero);
      child_exp->set_factor_node(child_factor);

      init->exp_list.push_back(child_exp);
    }
  }
}

void parser::get_arrInfo(std::vector<long> &arrDim, ast::ElemType &baseElemType,
                         std::vector<long> &derivedElemType,
                         symbolTable::symbolInfo &varInfo) {
  int i = 0;
  for (; i < (int)varInfo.derivedTypeMap[0].size(); i++) {
    if (varInfo.derivedTypeMap[0][i] > 0) {
      arrDim.push_back(varInfo.derivedTypeMap[0][i]);
    } else {
      break;
    }
  }
  baseElemType = (ast::ElemType)varInfo.derivedTypeMap[0][i];
  i++;
  for (; i < (int)varInfo.derivedTypeMap[0].size(); i++) {
    derivedElemType.push_back(varInfo.derivedTypeMap[0][i]);
  }
  if (!derivedElemType.empty()) {
    derivedElemType.insert(derivedElemType.begin(), (long)baseElemType);
    baseElemType = ast::ElemType::DERIVED;
  }
}

} // namespace parser
} // namespace scarlet
