#include <parser/common.hh>

namespace scarlet {
namespace parser {

void parser::analyze_declaration(
    std::shared_ptr<ast::AST_Declaration_Node> declaration,
    std::map<std::pair<std::string, int>, std::string> &symbol_table,
    int indx) {
  std::string var_name = declaration->get_identifier()->get_value();
  if (symbol_table.find({var_name, indx}) != symbol_table.end()) {
    // the symbol has been declared twice which is illegal
    success = false;
    error_messages.emplace_back("Variable " + var_name + " already declared");
  } else {
    symbol_table[{var_name, indx}] = get_temp_name(var_name);
    declaration->get_identifier()->set_identifier(
        symbol_table[{var_name, indx}]);
    if (declaration->get_type() == ast::DeclarationType::VARIABLE) {
      auto variable_declaration =
          std::static_pointer_cast<ast::AST_variable_declaration_Node>(
              declaration);
      if (variable_declaration->get_exp() != nullptr) {
        analyze_exp(variable_declaration->get_exp(), symbol_table, indx);
      }
    } else {
      auto function_declaration =
          std::static_pointer_cast<ast::AST_function_declaration_Node>(
              declaration);
    }
  }
}

} // namespace parser
} // namespace scarlet
