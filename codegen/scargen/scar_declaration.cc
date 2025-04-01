#include <codegen/common.hh>

namespace scarlet {
namespace codegen {

void Codegen::gen_scar_declaration(
    std::shared_ptr<ast::AST_Declaration_Node> declaration,
    std::shared_ptr<scar::scar_Function_Node> scar_function) {
  // if there is no definition, we ignore it
  if (declaration->get_type() == ast::DeclarationType::VARIABLE) {
    auto variable_declaration =
        std::static_pointer_cast<ast::AST_variable_declaration_Node>(
            declaration);

    // if the declaration is local static, do nothing as it will be handled
    // later
    if (variable_declaration->get_specifier() == ast::SpecifierType::STATIC) {
      ;
    } else {
      if (variable_declaration->get_exp() != nullptr) {
        gen_scar_exp(variable_declaration->get_exp(), scar_function);
        MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction);
        scar_instruction->set_type(scar::instruction_type::COPY);
        MAKE_SHARED(scar::scar_Val_Node, scar_val_src);
        SETVARCONSTANTREG(scar_val_src);
        scar_instruction->set_src1(scar_val_src);
        MAKE_SHARED(scar::scar_Val_Node, scar_val_dst);
        scar_val_dst->set_type(scar::val_type::VAR);
        scar_val_dst->set_reg_name(variable_declaration->get_declarator()
                                       ->get_identifier()
                                       ->get_value());
        scar_instruction->set_dst(scar_val_dst);
        scar_function->add_instruction(scar_instruction);
      }
    }
  }
}

} // namespace codegen
} // namespace scarlet
