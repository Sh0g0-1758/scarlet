#include <codegen/codegen.hh>

namespace scarlet {
namespace codegen {

void Codegen::gen_scar() {
  scar::scar_Program_Node scar_program;
  for (auto it : program.get_declarations()) {
    if (it->get_type() == ast::DeclarationType::VARIABLE) {

    } else if (it->get_type() == ast::DeclarationType::FUNCTION) {
      // If its a function declaration (no body) skip it
      auto funDecl =
          std::static_pointer_cast<ast::AST_function_declaration_Node>(it);
      if (funDecl->get_block() == nullptr)
        continue;

      MAKE_SHARED(scar::scar_Function_Node, scar_function);
      MAKE_SHARED(scar::scar_Identifier_Node, identifier);
      identifier->set_value(funDecl->get_identifier()->get_value());
      scar_function->set_identifier(identifier);

      for (auto param : funDecl->get_params()) {
        MAKE_SHARED(scar::scar_Identifier_Node, param_id);
        param_id->set_value(param->identifier->get_value());
        scar_function->add_param(param_id);
      }

      gen_scar_block(funDecl->get_block(), scar_function);

      // Add a complementary return 0 at the end of the function
      // in case there is no return statement
      MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction);
      scar_instruction->set_type(scar::instruction_type::RETURN);
      MAKE_SHARED(scar::scar_Val_Node, scar_val_src);
      scar_val_src->set_type(scar::val_type::CONSTANT);
      scar_val_src->set_value("0");
      scar_instruction->set_src1(scar_val_src);
      scar_function->add_instruction(scar_instruction);

      scar_program.add_function(scar_function);
    }
  }

  this->scar = scar_program;
}

} // namespace codegen
} // namespace scarlet
