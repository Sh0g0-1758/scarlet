#include <codegen/common.hh>

namespace scarlet {
namespace codegen {

void Codegen::gen_scar_declaration(
    std::shared_ptr<ast::AST_Declaration_Node> declaration,
    std::shared_ptr<scar::scar_Function_Node> scar_function,
    scar::scar_Program_Node &scar_program) {
  // if there is no definition, we ignore it
  if (declaration->get_type() == ast::DeclarationType::VARIABLE) {
    auto variable_declaration =
        std::static_pointer_cast<ast::AST_variable_declaration_Node>(
            declaration);

    // if the declaration is local static, then we put it at the global level
    if (variable_declaration->get_specifier() == ast::SpecifierType::STATIC) {
      MAKE_SHARED(scar::scar_StaticVariable_Node, static_variable);
      MAKE_SHARED(scar::scar_Identifier_Node, identifier);
      identifier->set_value(
          variable_declaration->get_identifier()->get_value());
      static_variable->set_identifier(std::move(identifier));
      static_variable->set_global(false);
      // the init value for a local static variable will always be a constant
      // integer
      static_variable->set_init(
          globalSymbolTable[variable_declaration->get_identifier()->get_value()]
              .value);
      MAKE_SHARED(scar::scar_Top_Level_Node, top_level);
      top_level =
          std::static_pointer_cast<scar::scar_Top_Level_Node>(static_variable);
      top_level->set_type(scar::topLevelType::STATICVARIABLE);
      scar_program.add_elem(std::move(top_level));
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
        scar_val_dst->set_reg_name(
            variable_declaration->get_identifier()->get_value());
        scar_instruction->set_dst(scar_val_dst);
        scar_function->add_instruction(scar_instruction);
      }
    }
  }
}

} // namespace codegen
} // namespace scarlet
