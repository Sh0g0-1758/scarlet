#include <parser/common.hh>

namespace scarlet {
namespace parser {

void parser::pretty_print_declaration(
    std::shared_ptr<ast::AST_Declaration_Node> declaration) {
  if (declaration == nullptr)
    return;
  std::cout << "\t\t\tDeclaration=(" << std::endl;
  std::cout << "\t\t\t\tidentifier=\""
            << declaration->get_identifier()->get_value() << "\"," << std::endl;
  if (declaration->get_type() == ast::DeclarationType::VARIABLE) {
    pretty_print_variable_declaration(
        std::static_pointer_cast<ast::AST_variable_declaration_Node>(
            declaration));
  } else {
    pretty_print_function_declaration(
        std::static_pointer_cast<ast::AST_function_declaration_Node>(
            declaration));
  }
}

void parser::pretty_print_variable_declaration(
    std::shared_ptr<ast::AST_variable_declaration_Node> declaration) {
  std::cout << "\t\t\t\ttype=" << type_to_string(declaration->get_type()) << ","
            << std::endl;
  std::cout << "\t\t\t\texp=(" << std::endl;
  pretty_print_exp(declaration->get_exp());
  std::cout << "\t\t\t\t)" << std::endl;
  std::cout << "\t\t\t)," << std::endl;
}

void parser::pretty_print_function_declaration(
    std::shared_ptr<ast::AST_function_declaration_Node> decl) {
  std::cout << "\t\t\t\treturn_type=" << type_to_string(decl->get_return_type())
            << "," << std::endl;
  std::cout << "\t\t\t\tparams=[" << std::endl;
  for (auto param : decl->get_params()) {
    std::cout << "\t\t\t\t\tParam=(" << std::endl;
    std::cout << "\t\t\t\t\t\ttype=" << type_to_string(param->type) << ","
              << std::endl;
    std::cout << "\t\t\t\t\t\tidentifier=\"" << param->identifier->get_value()
              << "\"" << std::endl;
    std::cout << "\t\t\t\t\t)," << std::endl;
  }
  std::cout << "\t\t\t\t]" << std::endl;
}

} // namespace parser
} // namespace scarlet
