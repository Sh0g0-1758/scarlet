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
  if (declaration->get_exp() != nullptr) {
    std::cout << "\t\t\t\texp=(" << std::endl;
    pretty_print_exp(declaration->get_exp());
    std::cout << "\t\t\t\t)," << std::endl;
  }
  std::cout << "\t\t\t)" << std::endl;
}

} // namespace parser
} // namespace scarlet
