#include <parser/common.hh>

namespace scarlet {
namespace parser {

void parser::pretty_print_declarator(
    std::shared_ptr<ast::AST_declarator_Node> declarator) {
  if (declarator == nullptr)
    return;
  pretty_print_declarator(declarator->get_child());

  if (declarator->is_pointer())
    std::cout << "*->";
  for (auto it : declarator->get_arrDim()) {
    std::cout << "[" << it << "]";
  }
  if (declarator->get_arrDim().size() != 0) {
    std::cout << "->";
  }
}

void parser::pretty_print_declaration(
    std::shared_ptr<ast::AST_Declaration_Node> declaration) {
  if (declaration == nullptr)
    return;
  if (declaration->get_type() == ast::DeclarationType::VARIABLE) {
    std::cout << "\tVariable Declaration=(" << std::endl;
    std::cout << "\t\tidentifier=\""
              << declaration->get_identifier()->get_value() << "\","
              << std::endl;
    pretty_print_variable_declaration(
        std::static_pointer_cast<ast::AST_variable_declaration_Node>(
            declaration));
  } else {
    std::cout << "\tFunction Declaration=(" << std::endl;
    std::cout << "\t\tidentifier=\""
              << declaration->get_identifier()->get_value() << "\","
              << std::endl;
    pretty_print_function_declaration(
        std::static_pointer_cast<ast::AST_function_declaration_Node>(
            declaration));
  }
}

void parser::pretty_print_variable_declaration(
    std::shared_ptr<ast::AST_variable_declaration_Node> declaration) {
  std::cout << "\t\tStorageSpecifier="
            << ast::to_string(declaration->get_specifier()) << "," << std::endl;
  std::cout << "\t\ttype=";
  pretty_print_declarator(declaration->get_declarator());
  std::cout << ast::to_string(declaration->get_base_type());
  std::cout << "," << std::endl;
  std::cout << "\t\texp=(" << std::endl;
  pretty_print_exp(declaration->get_exp());
  std::cout << "\t\t)" << std::endl;
  std::cout << "\t)," << std::endl;
}

void parser::pretty_print_function_declaration(
    std::shared_ptr<ast::AST_function_declaration_Node> decl) {
  std::cout << "\t\tStorageSpecifier=" << ast::to_string(decl->get_specifier())
            << "," << std::endl;
  std::cout << "\t\treturn_type=";
  pretty_print_declarator(decl->get_declarator());
  std::cout << ast::to_string(decl->get_return_type());
  std::cout << "," << std::endl;
  std::cout << "\t\tparams=[" << std::endl;
  for (auto param : decl->get_params()) {
    std::cout << "\t\t\tParam=(" << std::endl;
    std::cout << "\t\t\t\ttype=";
    pretty_print_declarator(param->declarator);
    std::cout << ast::to_string(param->base_type);
    std::cout << "," << std::endl;
    std::cout << "\t\t\t\tidentifier=\"" << param->identifier->get_value()
              << "\"" << std::endl;
    std::cout << "\t\t\t)," << std::endl;
  }
  std::cout << "\t\t]," << std::endl;
  std::cout << "\t\tbody=[" << std::endl;
  pretty_print_block(decl->get_block());
  std::cout << "\t\t]" << std::endl;
  std::cout << "\t)," << std::endl;
}

} // namespace parser
} // namespace scarlet
