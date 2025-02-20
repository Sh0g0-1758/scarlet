#include <codegen/codegen.hh>

namespace scarlet {
namespace codegen {

#define FIX_PSEUDO(target)                                                     \
  if (NOTNULL(inst->get_##target()) &&                                         \
      inst->get_##target()->get_type() == scasm::operand_type::PSEUDO) {       \
    if (pseduo_registers.find(inst->get_##target()->get_identifier_stack()) != \
        pseduo_registers.end()) {                                              \
      inst->get_##target()->set_identifier_stack(                              \
          pseduo_registers[inst->get_##target()->get_identifier_stack()]);     \
    } else {                                                                   \
      std::string temp = inst->get_##target()->get_identifier_stack();         \
      inst->get_##target()->set_identifier_stack(                              \
          "-" + std::to_string(offset * 4) + "(%rbp)");                        \
      pseduo_registers[temp] = inst->get_##target()->get_identifier_stack();   \
      offset++;                                                                \
    }                                                                          \
    inst->get_##target()->set_type(scasm::operand_type::STACK);                \
  }

void Codegen::fix_pseudo_registers() {
  int offset = 1;
  for (auto &funcs : scasm.get_functions()) {
    for (auto &inst : funcs->get_instructions()) {
      FIX_PSEUDO(src);
      FIX_PSEUDO(dst);
    }
  }
  stack_offset = 4 * (offset - 1);
}

} // namespace codegen
} // namespace scarlet
