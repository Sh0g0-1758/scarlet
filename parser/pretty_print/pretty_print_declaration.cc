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
  } else if (declaration->get_type() == ast::DeclarationType::FUNCTION) {
    std::cout << "\tFunction Declaration=(" << std::endl;
    std::cout << "\t\tidentifier=\""
              << declaration->get_identifier()->get_value() << "\","
              << std::endl;
    pretty_print_function_declaration(
        std::static_pointer_cast<ast::AST_function_declaration_Node>(
            declaration));
  } else {
    std::cout << "\tStruct Declaration=(" << std::endl;
    std::cout << "\t\tidentifier=\""
              << declaration->get_identifier()->get_value() << "\","
              << std::endl;
    std::cout << "\t\tmembers=[" << std::endl;
    pretty_print_struct_declaration(
        std::static_pointer_cast<ast::AST_struct_declaration_Node>(
            declaration));
    std::cout << "\t\t]," << std::endl;
  }
}

void parser::pretty_print_struct_declaration(
    std::shared_ptr<ast::AST_struct_declaration_Node> decl) {
  for (auto member : decl->get_members()) {
    std::cout << "\t\t\tMember=(" << std::endl;
    std::cout << "\t\t\t\tIdentifier=" << member->get_identifier()->get_value()
              << "," << std::endl;
    pretty_print_member_declaration(member);
    std::cout << "\t\t\t)," << std::endl;
  }
  std::cout << "\t\t]," << std::endl;
}

void parser::pretty_print_member_declaration(
    std::shared_ptr<ast::AST_member_declaration_Node> member) {
  std::cout << "\t\t\t\ttype=";
  pretty_print_declarator(member->get_declarator());
  std::cout << ast::to_string(member->get_base_type());
  if (member->get_struct_identifier() != nullptr) {
    std::cout << " " << member->get_struct_identifier()->get_value()
              << std::endl;
  }
}

void parser::pretty_print_variable_declaration(
    std::shared_ptr<ast::AST_variable_declaration_Node> declaration) {
  std::cout << "\t\tStorageSpecifier="
            << ast::to_string(declaration->get_specifier()) << "," << std::endl;
  std::cout << "\t\ttype=";
  pretty_print_declarator(declaration->get_declarator());
  std::cout << ast::to_string(declaration->get_base_type());
  if (declaration->get_struct_identifier() != nullptr) {
    std::cout << " " << declaration->get_struct_identifier()->get_value()
              << std::endl;
  }
  std::cout << "," << std::endl;
  if (declaration->get_exp() != nullptr) {
    std::cout << "\t\texp=(" << std::endl;
    pretty_print_exp(declaration->get_exp());
    std::cout << "\t\t)" << std::endl;
  }
  if (declaration->get_initializer() != nullptr) {
    std::cout << "\t\tArrayInit=(" << std::endl;
    pretty_print_initializer(declaration->get_initializer());
    std::cout << "\t\t)" << std::endl;
  }
  std::cout << "\t)," << std::endl;
}

void parser::pretty_print_initializer(std::shared_ptr<ast::initializer> init) {
  if (init == nullptr)
    return;
  if (!(init->initializer_list.empty())) {
    std::cout << "\t\t\tInitializerList=[" << std::endl;
    for (auto child : init->initializer_list) {
      pretty_print_initializer(child);
    }
    std::cout << "\t\t\t]," << std::endl;
  } else if (!(init->exp_list.empty())) {
    std::cout << "\t\t\tExpList=[" << std::endl;
    for (auto child : init->exp_list) {
      pretty_print_exp(child);
    }
    std::cout << "\t\t\t]," << std::endl;
  }
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
