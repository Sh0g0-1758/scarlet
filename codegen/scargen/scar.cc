#include <codegen/codegen.hh>

namespace scarlet {
namespace codegen {

void Codegen::gen_scar() {
  scar::scar_Program_Node scar_program;
  for (auto it : program.get_functions()) {
    MAKE_SHARED(scar::scar_Function_Node, scar_function);
    MAKE_SHARED(scar::scar_Identifier_Node, identifier);
    identifier->set_value(it->get_identifier()->get_value());
    scar_function->set_identifier(identifier);

    gen_scar_block(it->get_block(), scar_function);

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

  this->scar = scar_program;
}

} // namespace codegen
} // namespace scarlet
