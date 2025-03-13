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
        analyze_exp(variable_declaration->get_exp(), symbol_table, indx);
      }
    }
  } else {
    // 1. Check that the variables always have different names.
    // 2. If the identifier has already been declared somewhere in the same
    // scope
    //    then make sure that it is also function declaration.
    // 3. If we only have a function declaration, just add it to the symbol
    // table
    //    But if we have a function definition, then add the variables to a new
    //    scope
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

    symbol_table[{var_name, indx}] = {var_name, linkage::EXTERNAL,
                                      symbolType::FUNCTION, funcType};
  }
}

} // namespace parser
} // namespace scarlet
