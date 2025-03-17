#include <parser/common.hh>

namespace scarlet {
namespace parser {

#define EXPISCONSTANT(exp)                                                     \
  (exp->get_binop_node() == nullptr and exp->get_left() == nullptr and         \
   exp->get_right() == nullptr and exp->get_factor_node() != nullptr and       \
   exp->get_factor_node()->get_int_node() != nullptr and                       \
   exp->get_factor_node()->get_unop_nodes().empty() and                        \
   exp->get_factor_node()->get_identifier_node() == nullptr and                \
   exp->get_factor_node()->get_exp_node() == nullptr)

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
        ; // do nothing
      } else {
        success = false;
        error_messages.emplace_back("Variable " + var_name +
                                    " already declared");
      }
    } else {
      auto varDecl =
          std::static_pointer_cast<ast::AST_variable_declaration_Node>(
              declaration);
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
          if (globalSymbolTable[var_name].typeDef.size() > 1 or
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
        varDecl->get_identifier()->set_identifier(temp_name);
        symbol_table[{var_name, indx}] = {temp_name,
                                          symbolTable::linkage::NONE,
                                          symbolTable::symbolType::VARIABLE,
                                          {varDecl->get_type()}};
        if (!EXPISCONSTANT(varDecl->get_exp())) {
          success = false;
          error_messages.emplace_back(
              "Global variable " + var_name +
              " is not initialized with a constant integer");
        } else {
          symbol_table[{var_name, indx}].value =
              std::stoi(varDecl->get_exp()
                            ->get_factor_node()
                            ->get_int_node()
                            ->get_value());
        }
      } else {
        std::string temp_name = get_temp_name(var_name);
        varDecl->get_identifier()->set_identifier(temp_name);
        symbol_table[{var_name, indx}] = {temp_name,
                                          symbolTable::linkage::NONE,
                                          symbolTable::symbolType::VARIABLE,
                                          {varDecl->get_type()}};
        if (varDecl->get_exp() != nullptr) {
          symbol_table[{var_name, indx}].def = symbolTable::defType::TRUE;
          analyze_exp(varDecl->get_exp(), symbol_table, indx);
        }
        globalSymbolTable[temp_name] = symbol_table[{var_name, indx}];
      }
    }
  } else {
    // 1. Check that the function parameters always have different names.
    // 2. If the identifier has already been declared somewhere in the same
    //    scope then make sure that it is also function declaration.
    // 3. Add the function declaration with the correct function type
    //    (return type ++ args_type) into the symbol table
    // 4. Check that two instances of the function declaration don't have
    //    different types.
    // 5. Make sure that function declarations at block scope don't have
    //    static storage specifier.
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
    std::set<std::string> param_names;
    for (auto param : funcDecl->get_params()) {
      if (param_names.find(param->identifier->get_value()) !=
          param_names.end()) {
        success = false;
        error_messages.emplace_back(
            "Variable " + param->identifier->get_value() + " already declared");
      } else {
        param_names.insert(param->identifier->get_value());
      }
    }

    if (symbol_table.find({var_name, indx}) != symbol_table.end()) {
      if (symbol_table[{var_name, indx}].type !=
          symbolTable::symbolType::FUNCTION) {
        success = false;
        error_messages.emplace_back(
            var_name + " redeclared as a different kind of symbol");
      }
    }

    std::vector<ast::ElemType> funcType;
    funcType.emplace_back(funcDecl->get_return_type());
    for (auto param : funcDecl->get_params()) {
      funcType.emplace_back(param->type);
    }

    if (globalSymbolTable.find(var_name) != globalSymbolTable.end()) {
      if (globalSymbolTable[var_name].typeDef != funcType) {
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
      symbol_table[{var_name, indx}] = {
          var_name, symbolTable::linkage::EXTERNAL,
          symbolTable::symbolType::FUNCTION, funcType};
      if (funcDecl->get_specifier() == ast::SpecifierType::STATIC) {
        symbol_table[{var_name, indx}].link = symbolTable::linkage::INTERNAL;
      }
      globalSymbolTable[var_name] = symbol_table[{var_name, indx}];
    }
  }
}

} // namespace parser
} // namespace scarlet
