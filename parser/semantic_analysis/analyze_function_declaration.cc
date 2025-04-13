#include <parser/common.hh>

namespace scarlet {
namespace parser {

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
        if (funcType[i] == ast::ElemType::DERIVED and
            !ast::validate_type_specifier(funcType[i], funcDerivedTypeMap[i])) {
          success = false;
          error_messages.emplace_back("Variable " + var_name +
                                      " cannot be declared as incomplete type");
        }
      } else {
        funcType.push_back(param->base_type);
        if (param->base_type == ast::ElemType::VOID) {
          success = false;
          error_messages.emplace_back("Function " + var_name +
                                      " cannot take void argument");
        }
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

} // namespace parser
} // namespace scarlet
