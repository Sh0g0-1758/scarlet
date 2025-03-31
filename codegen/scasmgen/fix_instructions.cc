#include <codegen/common.hh>

namespace scarlet {
namespace codegen {

void Codegen::fix_instructions() {
  // Allocate stack frame
  for (auto &elem : scasm.get_elems()) {
    if (elem->get_type() != scasm::scasm_top_level_type::FUNCTION) {
      continue;
    }
    auto funcs = std::static_pointer_cast<scasm::scasm_function>(elem);
    MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
    scasm_inst->set_type(scasm::instruction_type::BINARY);
    scasm_inst->set_binop(scasm::Binop::SUB);
    scasm_inst->set_asm_type(scasm::AssemblyType::QUAD_WORD);

    MAKE_SHARED(scasm::scasm_operand, scasm_src);
    scasm_src->set_type(scasm::operand_type::IMM);
    constant::Constant cfs;
    cfs.set_type(constant::Type::INT);
    cfs.set_value({.i = funcs->get_frame_size()});
    scasm_src->set_imm(cfs);
    scasm_inst->set_src(std::move(scasm_src));

    MAKE_SHARED(scasm::scasm_operand, scasm_dst);
    scasm_dst->set_type(scasm::operand_type::REG);
    scasm_dst->set_reg(scasm::register_type::SP);
    scasm_inst->set_dst(std::move(scasm_dst));

    funcs->get_instructions().insert(funcs->get_instructions().begin(),
                                     std::move(scasm_inst));
  }

  // Fixing up instructions in which both src and dst are Stack/Data
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
          ((*it)->get_dst()->get_type() == scasm::operand_type::STACK or
           (*it)->get_dst()->get_type() == scasm::operand_type::DATA)) {
        MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
        scasm_inst->set_type(scasm::instruction_type::MOV);
        scasm_inst->set_asm_type((*it)->get_asm_type());
        scasm_inst->set_src((*it)->get_src());

        MAKE_SHARED(scasm::scasm_operand, dst);
        dst->set_type(scasm::operand_type::REG);
        if ((*it)->get_asm_type() == scasm::AssemblyType::DOUBLE) {
          dst->set_reg(scasm::register_type::XMM14);
        } else {
          dst->set_reg(scasm::register_type::R10);
        }
        scasm_inst->set_dst(dst);

        (*it)->set_src(std::move(dst));
        it = funcs->get_instructions().insert(it, std::move(scasm_inst));
        it++;
      }
    }
  }

  // FIXES SPECIFIC TO X86_64
  for (auto &elem : scasm.get_elems()) {
    if (elem->get_type() != scasm::scasm_top_level_type::FUNCTION) {
      continue;
    }
    auto funcs = std::static_pointer_cast<scasm::scasm_function>(elem);
    for (auto it = funcs->get_instructions().begin();
         it != funcs->get_instructions().end(); it++) {

      if ((*it)->get_type() == scasm::instruction_type::CMP) {
        // cmpl stack/reg, $5
        //       |
        //       v
        // movl $5, %r11d
        // cmpl stack/reg, %r11d

        // comisd stack/reg, stack/imm
        //       |
        //       v
        // movsd stack/imm, %xmm15
        // comisd stack/reg, %xmm15

        bool requires_fixup =
            ((*it)->get_dst()->get_type() == scasm::operand_type::IMM or
             ((*it)->get_asm_type() == scasm::AssemblyType::DOUBLE and
              (*it)->get_dst()->get_type() != scasm::operand_type::REG));
        if (requires_fixup) {
          MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
          scasm_inst->set_type(scasm::instruction_type::MOV);
          scasm_inst->set_asm_type((*it)->get_asm_type());
          scasm_inst->set_src((*it)->get_dst());
          MAKE_SHARED(scasm::scasm_operand, dst);
          dst->set_type(scasm::operand_type::REG);
          if ((*it)->get_asm_type() == scasm::AssemblyType::DOUBLE) {
            dst->set_reg(scasm::register_type::XMM15);
          } else {
            dst->set_reg(scasm::register_type::R11);
          }
          scasm_inst->set_dst(dst);
          (*it)->set_dst(std::move(dst));
          it = funcs->get_instructions().insert(it, std::move(scasm_inst));
          it++;
        }
      } else if (((*it)->get_type() == scasm::instruction_type::IDIV or
                  (*it)->get_type() == scasm::instruction_type::DIV) and
                 (*it)->get_src()->get_type() == scasm::operand_type::IMM) {
        // idivl $3
        //   |
        //   v
        // movl $3, %r11d
        // idivl %r11d
        MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
        scasm_inst->set_type(scasm::instruction_type::MOV);
        scasm_inst->set_asm_type((*it)->get_asm_type());
        scasm_inst->set_src((*it)->get_src());
        MAKE_SHARED(scasm::scasm_operand, dst);
        dst->set_type(scasm::operand_type::REG);
        dst->set_reg(scasm::register_type::R11);
        scasm_inst->set_dst(dst);
        (*it)->set_src(std::move(dst));
        it = funcs->get_instructions().insert(it, std::move(scasm_inst));
        it++;
      } else if ((*it)->get_type() == scasm::instruction_type::MOVZX) {
        if ((*it)->get_dst()->get_type() == scasm::operand_type::STACK or
            (*it)->get_dst()->get_type() == scasm::operand_type::DATA) {
          MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
          scasm_inst->set_type(scasm::instruction_type::MOV);
          scasm_inst->set_asm_type(scasm::AssemblyType::LONG_WORD);
          scasm_inst->set_src((*it)->get_src());
          MAKE_SHARED(scasm::scasm_operand, dst);
          dst->set_type(scasm::operand_type::REG);
          dst->set_reg(scasm::register_type::R11);
          scasm_inst->set_dst(dst);
          (*it)->set_src(std::move(dst));
          (*it)->set_type(scasm::instruction_type::MOV);
          (*it)->set_asm_type(scasm::AssemblyType::QUAD_WORD);
          it = funcs->get_instructions().insert(it, std::move(scasm_inst));
          it++;
        } else {
          (*it)->set_type(scasm::instruction_type::MOV);
          (*it)->set_asm_type(scasm::AssemblyType::LONG_WORD);
          it++;
        }
      } else if ((*it)->get_type() == scasm::instruction_type::BINARY and
                 (*it)->get_binop() == scasm::Binop::MUL and
                 ((*it)->get_dst()->get_type() == scasm::operand_type::STACK or
                  (*it)->get_dst()->get_type() == scasm::operand_type::DATA)) {
        // imull $3, STACK/DATA
        //        |
        //        v
        // movl STACK, %r11d
        // imull $3, %r11d
        // movl %r11d, STACK/DATA

        MAKE_SHARED(scasm::scasm_operand, dst);
        dst->set_type(scasm::operand_type::REG);
        dst->set_reg(scasm::register_type::R11);

        MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
        scasm_inst->set_type(scasm::instruction_type::MOV);
        scasm_inst->set_asm_type((*it)->get_asm_type());
        scasm_inst->set_src((*it)->get_dst());
        scasm_inst->set_dst(dst);

        MAKE_SHARED(scasm::scasm_instruction, scasm_inst2);
        scasm_inst2->set_type(scasm::instruction_type::MOV);
        scasm_inst2->set_asm_type((*it)->get_asm_type());
        scasm_inst2->set_src(dst);
        scasm_inst2->set_dst((*it)->get_dst());

        (*it)->set_dst(std::move(dst));

        it = funcs->get_instructions().insert(it, std::move(scasm_inst));
        it++;
        it = funcs->get_instructions().insert(it + 1, std::move(scasm_inst2));
      } else if ((*it)->get_type() == scasm::instruction_type::MOVSX) {
        // Movsx $10, STACK/DATA
        //       |
        //       v
        // <<LONGWORD>>  movl $10, %r11l
        //               movsx %r11l, %r11d
        // <<QUADWORD>>  movq %r11d, STACK/DATA
        if ((*it)->get_src()->get_type() == scasm::operand_type::IMM) {
          MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
          scasm_inst->set_type(scasm::instruction_type::MOV);
          scasm_inst->set_asm_type(scasm::AssemblyType::LONG_WORD);

          MAKE_SHARED(scasm::scasm_operand, dst);
          dst->set_type(scasm::operand_type::REG);
          dst->set_reg(scasm::register_type::R11);

          scasm_inst->set_src((*it)->get_src());
          scasm_inst->set_dst(dst);

          (*it)->set_src(std::move(dst));

          it = funcs->get_instructions().insert(it, std::move(scasm_inst));
          it++;
        }
        if ((*it)->get_dst()->get_type() == scasm::operand_type::STACK or
            (*it)->get_dst()->get_type() == scasm::operand_type::DATA) {
          MAKE_SHARED(scasm::scasm_operand, src);
          src->set_type(scasm::operand_type::REG);
          src->set_reg(scasm::register_type::R11);

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
          scasm_inst->set_type(scasm::instruction_type::MOV);
          scasm_inst->set_asm_type(scasm::AssemblyType::QUAD_WORD);
          scasm_inst->set_src(src);
          scasm_inst->set_dst((*it)->get_dst());

          (*it)->set_dst(std::move(src));

          it = funcs->get_instructions().insert(it + 1, std::move(scasm_inst));
        }
      } else if ((*it)->get_asm_type() == scasm::AssemblyType::DOUBLE and
                 (*it)->get_type() == scasm::instruction_type::BINARY and
                 (*it)->get_dst()->get_type() != scasm::operand_type::REG) {
        // subsd/addsd/mulsd/divsd/xorpd stack/reg/imm, stack/data
        //        |
        //        v
        // movsd stack/data, %xmm15
        // subsd/addsd/mulsd/divsd stack/reg/imm , %xmm15
        MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
        scasm_inst->set_type(scasm::instruction_type::MOV);
        scasm_inst->set_asm_type((*it)->get_asm_type());
        scasm_inst->set_src((*it)->get_dst());

        MAKE_SHARED(scasm::scasm_operand, dst);
        dst->set_type(scasm::operand_type::REG);
        dst->set_reg(scasm::register_type::XMM15);
        scasm_inst->set_dst(dst);

        (*it)->set_dst(std::move(dst));
        it = funcs->get_instructions().insert(it, std::move(scasm_inst));
        it++;
      } else if ((*it)->get_type() == scasm::instruction_type::CVTSI2SD) {
        // cvtsi2sd imm, stack/data
        //        |
        //        v
        // movsd imm, %R11
        // cvtsi2sd %R11, %XMM15
        // movsd %XMM15, stack/data
        if ((*it)->get_src()->get_type() == scasm::operand_type::IMM) {
          MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
          scasm_inst->set_type(scasm::instruction_type::MOV);
          scasm_inst->set_asm_type((*it)->get_asm_type());
          scasm_inst->set_src((*it)->get_src());
          MAKE_SHARED(scasm::scasm_operand, dst);
          dst->set_type(scasm::operand_type::REG);
          dst->set_reg(scasm::register_type::R11);
          scasm_inst->set_dst(dst);
          it = funcs->get_instructions().insert(it, std::move(scasm_inst));
          it++;
          (*it)->set_src(std::move(dst)); // change the src to r11
        }

        if ((*it)->get_dst()->get_type() != scasm::operand_type::REG) {
          MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
          scasm_inst->set_type(scasm::instruction_type::MOV);
          scasm_inst->set_asm_type(scasm::AssemblyType::DOUBLE);
          MAKE_SHARED(scasm::scasm_operand, src);
          src->set_type(scasm::operand_type::REG);
          src->set_reg(scasm::register_type::XMM15);
          scasm_inst->set_src(src);
          scasm_inst->set_dst((*it)->get_dst());
          (*it)->set_dst(std::move(src)); // change the dst to xmm15
          it = funcs->get_instructions().insert(it + 1, std::move(scasm_inst));
          it++;
        }
      } else if ((*it)->get_type() == scasm::instruction_type::CVTTS2DI and
                 (*it)->get_dst()->get_type() != scasm::operand_type::REG) {
        // cvttsd2si stack/reg/imm, stack/data
        //        |
        //        v
        // movsd stack/reg/imm, %r11
        // cvttsd2si %r11, %xmm15
        MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
        scasm_inst->set_type(scasm::instruction_type::MOV);
        scasm_inst->set_asm_type((*it)->get_asm_type());
        MAKE_SHARED(scasm::scasm_operand, src);
        src->set_type(scasm::operand_type::REG);
        src->set_reg(scasm::register_type::R11);
        scasm_inst->set_src(src);
        scasm_inst->set_dst((*it)->get_dst());
        (*it)->set_dst(std::move(src)); // change the dst to xmm15
        it = funcs->get_instructions().insert(it + 1, std::move(scasm_inst));
        it++;
      }
    }

    for (auto it = funcs->get_instructions().begin();
         it != funcs->get_instructions().end(); it++) {
      // If the Immediate value cannot be represented as a signed 32 bit,
      // then it is moved to a register(r11) and then used.
      if (NOTNULL((*it)->get_src()) and
          (*it)->get_asm_type() == scasm::AssemblyType::QUAD_WORD and
          (*it)->get_src()->get_type() == scasm::operand_type::IMM) {
        if ((*it)->get_src()->get_imm().get_type() == constant::Type::LONG and
            (*it)->get_src()->get_imm().get_value().l > INT32_MAX) {
          MAKE_SHARED(scasm::scasm_operand, dst);
          dst->set_type(scasm::operand_type::REG);
          dst->set_reg(scasm::register_type::R11);

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
          scasm_inst->set_type(scasm::instruction_type::MOV);
          scasm_inst->set_asm_type(scasm::AssemblyType::QUAD_WORD);
          scasm_inst->set_src((*it)->get_src());
          scasm_inst->set_dst(dst);

          (*it)->set_src(std::move(dst));

          it = funcs->get_instructions().insert(it, std::move(scasm_inst));
          it++;
        } else if ((*it)->get_src()->get_imm().get_type() ==
                       constant::Type::UINT and
                   (*it)->get_src()->get_imm().get_value().ui > INT32_MAX) {
          MAKE_SHARED(scasm::scasm_operand, dst);
          dst->set_type(scasm::operand_type::REG);
          dst->set_reg(scasm::register_type::R11);

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
          scasm_inst->set_type(scasm::instruction_type::MOV);
          scasm_inst->set_asm_type(scasm::AssemblyType::LONG_WORD);
          scasm_inst->set_src((*it)->get_src());
          scasm_inst->set_dst(dst);

          (*it)->set_src(std::move(dst));

          it = funcs->get_instructions().insert(it, std::move(scasm_inst));
          it++;
        } else if ((*it)->get_src()->get_imm().get_type() ==
                       constant::Type::ULONG and
                   (*it)->get_src()->get_imm().get_value().ul > INT32_MAX) {
          MAKE_SHARED(scasm::scasm_operand, dst);
          dst->set_type(scasm::operand_type::REG);
          dst->set_reg(scasm::register_type::R11);

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
          scasm_inst->set_type(scasm::instruction_type::MOV);
          scasm_inst->set_asm_type(scasm::AssemblyType::QUAD_WORD);
          scasm_inst->set_src((*it)->get_src());
          scasm_inst->set_dst(dst);

          (*it)->set_src(std::move(dst));

          it = funcs->get_instructions().insert(it, std::move(scasm_inst));
          it++;
        }
      }

      // In case of trunacate instruction (movl), the assembler will
      // automatically truncate the immediate value if it exceeds INT32_MAX.
      // Though this might emit a warning. So instead, we do the truncation here
      // only.
      if (NOTNULL((*it)->get_src()) and
          (*it)->get_asm_type() == scasm::AssemblyType::LONG_WORD and
          (*it)->get_type() == scasm::instruction_type::MOV and
          (*it)->get_src()->get_type() == scasm::operand_type::IMM) {
        if ((*it)->get_src()->get_imm().get_type() == constant::Type::LONG and
            (*it)->get_src()->get_imm().get_value().l > INT32_MAX) {
          int trunc_val =
              static_cast<int>((*it)->get_src()->get_imm().get_value().l);
          constant::Constant trunc;
          trunc.set_type(constant::Type::INT);
          trunc.set_value({.i = trunc_val});

          (*it)->get_src()->set_imm(trunc);
        }
        if ((*it)->get_src()->get_imm().get_type() == constant::Type::ULONG and
            (*it)->get_src()->get_imm().get_value().ul > INT32_MAX) {
          int trunc_val =
              static_cast<int>((*it)->get_src()->get_imm().get_value().ul);
          constant::Constant trunc;
          trunc.set_type(constant::Type::INT);
          trunc.set_value({.i = trunc_val});

          (*it)->get_src()->set_imm(trunc);
        }
      }
    }
  }
}

} // namespace codegen
} // namespace scarlet
