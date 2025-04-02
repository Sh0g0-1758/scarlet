#include <parser/common.hh>

namespace scarlet {
namespace parser {

#define INITZERO()                                                             \
  constant::Constant constZero;                                                \
                                                                               \
  switch (varDecl->get_type()) {                                               \
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
  case ast::ElemType::NONE:                                                    \
    UNREACHABLE();                                                             \
  }

void parser::analyze_declaration(
    std::shared_ptr<ast::AST_Declaration_Node> declaration,
    std::map<std::pair<std::string, int>, symbolTable::symbolInfo>
        &symbol_table,
    int indx) {
  std::string var_name =
      declaration->get_declarator()->get_identifier()->get_value();
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

void parser::analyze_global_variable_declaration(
    std::shared_ptr<ast::AST_variable_declaration_Node> varDecl,
    std::map<std::pair<std::string, int>, symbolTable::symbolInfo>
        &symbol_table) {
  std::string var_name =
      varDecl->get_declarator()->get_identifier()->get_value();

  // Check if the symbol has been declared before
  if (globalSymbolTable.find(var_name) != globalSymbolTable.end()) {
    // Ensure that the previous declaration has the same type
    if (globalSymbolTable[var_name].type != symbolTable::symbolType::VARIABLE or
        globalSymbolTable[var_name].typeDef[0] != varDecl->get_type()) {
      success = false;
      error_messages.emplace_back(var_name +
                                  " redeclared as a different kind of symbol");
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
            ast::castConstToVal(varDecl->get_exp()
                                    ->get_factor_node()
                                    ->get_const_node()
                                    ->get_constant(),
                                varDecl->get_type());
        symbol_table[{var_name, 0}].def = symbolTable::defType::TRUE;
        symbol_table[{var_name, 0}].value = globalSymbolTable[var_name].value;
      }
    } else if (varDecl->get_specifier() != ast::SpecifierType::EXTERN) {
      // If the variable has not been defined and is not extern,
      // mark it as a tentative definition
      if (globalSymbolTable[var_name].def == symbolTable::defType::FALSE) {
        INITZERO();
        symbol_table[{var_name, 0}].def = symbolTable::defType::TENTATIVE;
        globalSymbolTable[var_name].def = symbolTable::defType::TENTATIVE;
        symbol_table[{var_name, 0}].value = constZero;
        globalSymbolTable[var_name].value = constZero;
      }
    }
  } else { // symbol has not been declared before
    // Give appropriate linkage to the variable
    symbol_table[{var_name, 0}] = {var_name,
                                   symbolTable::linkage::EXTERNAL,
                                   symbolTable::symbolType::VARIABLE,
                                   {varDecl->get_type()},
                                   symbolTable::defType::TENTATIVE};
    if (varDecl->get_specifier() == ast::SpecifierType::STATIC) {
      symbol_table[{var_name, 0}].link = symbolTable::linkage::INTERNAL;
    } else if (varDecl->get_specifier() == ast::SpecifierType::EXTERN) {
      symbol_table[{var_name, 0}].def = symbolTable::defType::FALSE;
    }

    // If storage specifier is not extern, set the tentative value to zero
    if (varDecl->get_specifier() != ast::SpecifierType::EXTERN) {
      INITZERO();
      symbol_table[{var_name, 0}].value = constZero;
    }

    // Make sure that global variables are initialized only with
    // constant integers
    if (varDecl->get_exp() != nullptr) {
      symbol_table[{var_name, 0}].def = symbolTable::defType::TRUE;
      if (!EXPISCONSTANT(varDecl->get_exp())) {
        success = false;
        error_messages.emplace_back(
            "Global variable " + var_name +
            " is not initialized with a constant integer");
      } else {
        symbol_table[{var_name, 0}].value =
            ast::castConstToVal(varDecl->get_exp()
                                    ->get_factor_node()
                                    ->get_const_node()
                                    ->get_constant(),
                                varDecl->get_type());
      }
    }
    globalSymbolTable[var_name] = symbol_table[{var_name, 0}];
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
    for (auto param : funcDecl->get_params()) {
      std::string temp_name =
          get_temp_name(param->get_declarator()->get_identifier()->get_value());
      proxy_symbol_table[{
          param->get_declarator()->get_identifier()->get_value(), 1}] = {
          temp_name,
          symbolTable::linkage::NONE,
          symbolTable::symbolType::VARIABLE,
          {param->type}};
      globalSymbolTable[temp_name] = proxy_symbol_table[{
          param->get_declarator()->get_identifier()->get_value(), 1}];
      param->get_declarator()->get_identifier()->set_identifier(temp_name);
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
    if (param_names.find(
            param->get_declarator()->get_identifier()->get_value()) !=
        param_names.end()) {
      success = false;
      error_messages.emplace_back(
          "Variable " + param->get_declarator()->get_identifier()->get_value() +
          " already declared");
    } else {
      param_names.insert(
          param->get_declarator()->get_identifier()->get_value());
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
  funcType.emplace_back(funcDecl->get_return_type());
  for (auto param : funcDecl->get_params()) {
    funcType.emplace_back(param->type);
  }

  // If the function has already been declared, make sure that it has the
  // same type as the current declaration and has an acceptable linkage.
  if (globalSymbolTable.find(var_name) != globalSymbolTable.end()) {
    if (globalSymbolTable[var_name].typeDef != funcType or
        globalSymbolTable[var_name].type == symbolTable::symbolType::VARIABLE) {

      success = false;
      error_messages.emplace_back(var_name +
                                  " redeclared with different signature");
    } else {
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
    }
  } else {
    // Add the function declaration with the correct function type
    // and linkage into the symbol table
    symbol_table[{var_name, indx}] = {var_name, symbolTable::linkage::EXTERNAL,
                                      symbolTable::symbolType::FUNCTION,
                                      funcType};
    if (funcDecl->get_specifier() == ast::SpecifierType::STATIC) {
      symbol_table[{var_name, indx}].link = symbolTable::linkage::INTERNAL;
    }
    globalSymbolTable[var_name] = symbol_table[{var_name, indx}];
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
              symbolTable::symbolType::VARIABLE or
          globalSymbolTable[var_name].typeDef[0] != varDecl->get_type()) {
        success = false;
        error_messages.emplace_back(
            var_name + " redeclared as a different kind of symbol");
      }
      // do nothing as the variable has already been declared
      symbol_table[{var_name, indx}] = globalSymbolTable[var_name];
    } else {
      symbol_table[{var_name, indx}] = {var_name,
                                        symbolTable::linkage::EXTERNAL,
                                        symbolTable::symbolType::VARIABLE,
                                        {varDecl->get_type()}};
      globalSymbolTable[var_name] = symbol_table[{var_name, indx}];
    }
  } else if (varDecl->get_specifier() == ast::SpecifierType::STATIC) {
    std::string temp_name = get_temp_name(var_name);
    varDecl->get_declarator()->get_identifier()->set_identifier(temp_name);
    symbol_table[{var_name, indx}] = {temp_name,
                                      symbolTable::linkage::INTERNAL,
                                      symbolTable::symbolType::VARIABLE,
                                      {varDecl->get_type()}};
    symbol_table[{var_name, indx}].def = symbolTable::defType::TRUE;
    if (varDecl->get_exp() != nullptr) {
      if (!EXPISCONSTANT(varDecl->get_exp())) {
        success = false;
        error_messages.emplace_back(
            "Global variable " + var_name +
            " is not initialized with a constant integer");
      } else {
        symbol_table[{var_name, indx}].value =
            ast::castConstToVal(varDecl->get_exp()
                                    ->get_factor_node()
                                    ->get_const_node()
                                    ->get_constant(),
                                varDecl->get_type());
      }
    } else {
      INITZERO();
      symbol_table[{var_name, indx}].value = constZero;
    }
    globalSymbolTable[temp_name] = symbol_table[{var_name, indx}];
  } else {
    std::string temp_name = get_temp_name(var_name);
    varDecl->get_declarator()->get_identifier()->set_identifier(temp_name);
    symbol_table[{var_name, indx}] = {temp_name,
                                      symbolTable::linkage::NONE,
                                      symbolTable::symbolType::VARIABLE,
                                      {varDecl->get_type()}};

    globalSymbolTable[temp_name] = symbol_table[{var_name, indx}];
    if (varDecl->get_exp() != nullptr) {
      symbol_table[{var_name, indx}].def = symbolTable::defType::TRUE;
      globalSymbolTable[temp_name].def = symbolTable::defType::TRUE;
      analyze_exp(varDecl->get_exp(), symbol_table, indx);
      if (varDecl->get_exp()->get_type() != varDecl->get_type()) {
        add_cast_to_exp(varDecl->get_exp(), varDecl->get_type());
      }
    }
  }
}

} // namespace parser
} // namespace scarlet
