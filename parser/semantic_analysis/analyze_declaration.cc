#include <parser/common.hh>

namespace scarlet {
namespace parser {

void parser::analyze_declaration(
    std::shared_ptr<ast::AST_Declaration_Node> declaration,
    std::map<std::pair<std::string, int>, symbolInfo> &symbol_table, int indx) {
  std::string var_name = declaration->get_identifier()->get_value();
  if (declaration->get_type() == ast::DeclarationType::VARIABLE) {
    if (symbol_table.find({var_name, indx}) != symbol_table.end()) {
      // the symbol has been declared twice which is illegal
      success = false;
      error_messages.emplace_back("Variable " + var_name + " already declared");
    } else {
      std::string temp_name = get_temp_name(var_name);
      declaration->get_identifier()->set_identifier(temp_name);
      auto variable_declaration =
          std::static_pointer_cast<ast::AST_variable_declaration_Node>(
              declaration);
      symbol_table[{var_name, indx}] = {temp_name,
                                        linkage::INTERNAL,
                                        symbolType::VARIABLE,
                                        {variable_declaration->get_type()}};
      if (variable_declaration->get_exp() != nullptr) {
        symbol_table[{var_name, indx}].isDefined = true;
        analyze_exp(variable_declaration->get_exp(), symbol_table, indx);
      }
      globalSymbolTable[temp_name] = symbol_table[{var_name, indx}];
    }
  } else {
    // 1. Check that the variables always have different names.
    // 2. If the identifier has already been declared somewhere in the same
    //    scope then make sure that it is also function declaration.
    // 3. Add the function declaration with the correct function type
    //    (return type ++ args_type) into the symbol table
    // 4. Check that two instances of the function declaration don't have
    //    different types.
    auto function_declaration =
        std::static_pointer_cast<ast::AST_function_declaration_Node>(
            declaration);
    std::set<std::string> param_names;
    for (auto param : function_declaration->get_params()) {
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
      if (symbol_table[{var_name, indx}].type != symbolType::FUNCTION) {
        success = false;
        error_messages.emplace_back(
            var_name + " redeclared as a different kind of symbol");
      }
    }

    std::vector<ast::ElemType> funcType;
    funcType.emplace_back(function_declaration->get_return_type());
    for (auto param : function_declaration->get_params()) {
      funcType.emplace_back(param->type);
    }

    if (globalSymbolTable.find(var_name) != globalSymbolTable.end()) {
      if (globalSymbolTable[var_name].typeDef != funcType) {
        success = false;
        error_messages.emplace_back(var_name +
                                    " redeclared with different signature");
      } else if (globalSymbolTable[var_name].isDefined) {
        symbol_table[{var_name, indx}] = globalSymbolTable[var_name];
      }
    } else {
      symbol_table[{var_name, indx}] = {var_name, linkage::EXTERNAL,
                                        symbolType::FUNCTION, funcType};
      globalSymbolTable[var_name] = symbol_table[{var_name, indx}];
    }
  }
}

} // namespace parser
} // namespace scarlet
