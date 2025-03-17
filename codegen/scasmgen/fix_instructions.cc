#include <codegen/codegen.hh>

namespace scarlet {
namespace codegen {

void Codegen::fix_instructions() {
  // Allocate stack frame
  for (auto &elem : scasm.get_elems()) {
    if (elem->get_type() != scasm::scasm_top_level_type::FUNCTION) {
      continue;
    }
    auto funcs = std::static_pointer_cast<scasm::scasm_function>(elem);
    MAKE_SHARED(scasm::scasm_instruction, scasm_stack_instr);
    scasm_stack_instr->set_type(scasm::instruction_type::ALLOCATE_STACK);
    MAKE_SHARED(scasm::scasm_operand, val);
    val->set_type(scasm::operand_type::IMM);
    val->set_imm(funcs->get_frame_size());
    scasm_stack_instr->set_src(std::move(val));
    funcs->get_instructions().insert(funcs->get_instructions().begin(),
                                     std::move(scasm_stack_instr));
  }

  // Fixing up Stack/Data to Stack/Data move
  for (auto &elem : scasm.get_elems()) {
    if (elem->get_type() != scasm::scasm_top_level_type::FUNCTION) {
      continue;
    }
    auto funcs = std::static_pointer_cast<scasm::scasm_function>(elem);
    for (auto it = funcs->get_instructions().begin();
         it != funcs->get_instructions().end(); it++) {
      if (NOTNULL((*it)->get_src()) && NOTNULL((*it)->get_dst()) &&
              ((*it)->get_src()->get_type() == scasm::operand_type::STACK or
               (*it)->get_src()->get_type() == scasm::operand_type::DATA) &&
              ((*it)->get_dst()->get_type() == scasm::operand_type::STACK) or
          (*it)->get_dst()->get_type() == scasm::operand_type::DATA) {
        MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
        scasm_inst->set_type(scasm::instruction_type::MOV);
        scasm_inst->set_src((*it)->get_src());

        MAKE_SHARED(scasm::scasm_operand, dst);
        dst->set_type(scasm::operand_type::REG);
        dst->set_reg(scasm::register_type::R10);
        scasm_inst->set_dst(dst);

        (*it)->set_src(std::move(dst));
        it = funcs->get_instructions().insert(it, std::move(scasm_inst));
        it++;
      }
    }
  }

  // case when DIV uses a constant as an operand
  // case when MUL have dst as a stack value
  for (auto &elem : scasm.get_elems()) {
    if (elem->get_type() != scasm::scasm_top_level_type::FUNCTION) {
      continue;
    }
    auto funcs = std::static_pointer_cast<scasm::scasm_function>(elem);
    for (auto it = funcs->get_instructions().begin();
         it != funcs->get_instructions().end(); it++) {

      if ((*it)->get_type() == scasm::instruction_type::CMP and
          (*it)->get_dst()->get_type() == scasm::operand_type::IMM) {
        // cmpl stack/reg, $5
        //       |
        //       v
        // movl $5, %r11d
        // cmpl stack/reg, %r11d
        MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
        scasm_inst->set_type(scasm::instruction_type::MOV);
        scasm_inst->set_src((*it)->get_dst());
        MAKE_SHARED(scasm::scasm_operand, dst);
        dst->set_type(scasm::operand_type::REG);
        dst->set_reg(scasm::register_type::R11);
        scasm_inst->set_dst(dst);
        (*it)->set_dst(std::move(dst));
        it = funcs->get_instructions().insert(it, std::move(scasm_inst));
        it++;
      }
      if ((*it)->get_type() == scasm::instruction_type::IDIV and
          (*it)->get_src()->get_type() == scasm::operand_type::IMM) {
        // idivl $3
        //   |
        //   v
        // movl $3, %r10d
        // idivl %r10d
        MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
        scasm_inst->set_type(scasm::instruction_type::MOV);
        scasm_inst->set_src((*it)->get_src());
        MAKE_SHARED(scasm::scasm_operand, dst);
        dst->set_type(scasm::operand_type::REG);
        dst->set_reg(scasm::register_type::R10);
        scasm_inst->set_dst(dst);
        (*it)->set_src(std::move(dst));
        it = funcs->get_instructions().insert(it, std::move(scasm_inst));
        it++;
      } else if ((*it)->get_type() == scasm::instruction_type::BINARY and
                 (*it)->get_binop() == scasm::Binop::MUL and
                 (*it)->get_dst()->get_type() == scasm::operand_type::STACK) {
        // imull $3, STACK
        //        |
        //        v
        // movl STACK, %r11d
        // imull $3, %r11d
        // movl %r11d, STACK

        MAKE_SHARED(scasm::scasm_operand, dst);
        dst->set_type(scasm::operand_type::REG);
        dst->set_reg(scasm::register_type::R11);

        MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
        scasm_inst->set_type(scasm::instruction_type::MOV);
        scasm_inst->set_src((*it)->get_dst());
        scasm_inst->set_dst(dst);

        MAKE_SHARED(scasm::scasm_instruction, scasm_inst2);
        scasm_inst2->set_type(scasm::instruction_type::MOV);
        scasm_inst2->set_src(dst);
        scasm_inst2->set_dst((*it)->get_dst());

        (*it)->set_dst(std::move(dst));

        it = funcs->get_instructions().insert(it, std::move(scasm_inst));
        it++;
        it = funcs->get_instructions().insert(it + 1, std::move(scasm_inst2));
      }
    }
  }
}

} // namespace codegen
} // namespace scarlet
