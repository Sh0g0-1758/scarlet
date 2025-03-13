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
    auto function_declaration =
        std::static_pointer_cast<ast::AST_function_declaration_Node>(
            declaration);
  }
}

} // namespace parser
} // namespace scarlet
