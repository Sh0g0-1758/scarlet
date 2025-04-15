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

  // FIXES SPECIFIC TO X86_64
  /* **NOTE**
   * For this pass, use only R10 register because we use R11 for another pass
   * and the fixes required for some intruction can require both these passes.
   * As such the registers can coalesce
   */
  for (auto &elem : scasm.get_elems()) {
    if (elem->get_type() != scasm::scasm_top_level_type::FUNCTION) {
      continue;
    }
    auto funcs = std::static_pointer_cast<scasm::scasm_function>(elem);
    for (auto it = funcs->get_instructions().begin();
         it != funcs->get_instructions().end(); it++) {

      if ((*it)->get_asm_type() == scasm::AssemblyType::DOUBLE and
          (*it)->get_type() == scasm::instruction_type::BINARY and
          (*it)->get_dst()->get_type() != scasm::operand_type::REG) {
        // subsd/addsd/mulsd/divsd/xorpd stack/reg/imm, stack/data
        //        |
        //        v
        // movsd stack/data, %xmm15
        // subsd/addsd/mulsd/divsd stack/reg/imm , %xmm15
        // movsd %xmm15, stack/data
        MAKE_SHARED(scasm::scasm_operand, double_reg);
        double_reg->set_type(scasm::operand_type::REG);
        double_reg->set_reg(scasm::register_type::XMM15);

        MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
        scasm_inst->set_type(scasm::instruction_type::MOV);
        scasm_inst->set_asm_type(scasm::AssemblyType::DOUBLE);
        scasm_inst->set_src((*it)->get_dst());
        scasm_inst->set_dst(double_reg);

        MAKE_SHARED(scasm::scasm_instruction, scasm_inst2);
        scasm_inst2->set_type(scasm::instruction_type::MOV);
        scasm_inst2->set_asm_type(scasm::AssemblyType::DOUBLE);
        scasm_inst2->set_src(double_reg);
        scasm_inst2->set_dst((*it)->get_dst());

        (*it)->set_dst(std::move(double_reg));

        it = funcs->get_instructions().insert(it, std::move(scasm_inst));
        it++;
        it = funcs->get_instructions().insert(it + 1, std::move(scasm_inst2));
      } else if ((*it)->get_type() == scasm::instruction_type::CVTSI2SD) {
        // cvtsi2sd imm, stack/data
        //        |
        //        v
        // movsd imm, %R10
        // cvtsi2sd %R10, %XMM15
        // movsd %XMM15, stack/data
        if ((*it)->get_src()->get_type() == scasm::operand_type::IMM) {
          MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
          scasm_inst->set_type(scasm::instruction_type::MOV);
          scasm_inst->set_asm_type((*it)->get_asm_type());
          scasm_inst->set_src((*it)->get_src());
          MAKE_SHARED(scasm::scasm_operand, dst);
          dst->set_type(scasm::operand_type::REG);
          dst->set_reg(scasm::register_type::R10);
          scasm_inst->set_dst(dst);
          (*it)->set_src(std::move(dst));
          it = funcs->get_instructions().insert(it, std::move(scasm_inst));
          it++;
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
          (*it)->set_dst(std::move(src));
          it = funcs->get_instructions().insert(it + 1, std::move(scasm_inst));
        }
      } else if ((*it)->get_type() == scasm::instruction_type::CVTTS2DI and
                 (*it)->get_dst()->get_type() != scasm::operand_type::REG) {
        // cvttsd2si stack/reg/imm, stack/data
        //        |
        //        v
        // cvttsd2si stack/reg/imm, %R10
        // mov %r10, stack/data
        MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
        scasm_inst->set_type(scasm::instruction_type::MOV);
        scasm_inst->set_asm_type((*it)->get_asm_type());
        MAKE_SHARED(scasm::scasm_operand, src);
        src->set_type(scasm::operand_type::REG);
        src->set_reg(scasm::register_type::R10);
        scasm_inst->set_src(src);
        scasm_inst->set_dst((*it)->get_dst());
        (*it)->set_dst(std::move(src));
        it = funcs->get_instructions().insert(it + 1, std::move(scasm_inst));
      } else if ((*it)->get_type() == scasm::instruction_type::CMP) {
        // cmpl stack/reg, $5
        //       |
        //       v
        // movl $5, %r10d
        // cmpl stack/reg, %r10d

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
            dst->set_reg(scasm::register_type::R10);
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
        // movl $3, %r10d
        // idivl %r10d
        MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
        scasm_inst->set_type(scasm::instruction_type::MOV);
        scasm_inst->set_asm_type((*it)->get_asm_type());
        scasm_inst->set_src((*it)->get_src());
        MAKE_SHARED(scasm::scasm_operand, dst);
        dst->set_type(scasm::operand_type::REG);
        dst->set_reg(scasm::register_type::R10);
        scasm_inst->set_dst(dst);
        (*it)->set_src(std::move(dst));
        it = funcs->get_instructions().insert(it, std::move(scasm_inst));
        it++;
      } else if ((*it)->get_type() == scasm::instruction_type::MOVZX) {
        if ((*it)->get_asm_type() == scasm::AssemblyType::BYTE) {
          // MovZeroExtend imm, Stack/Data
          //     |
          //     v
          // mov  imm, %r10
          // movz %r10, %r11
          // mov  %r11, Stack/Data

          if ((*it)->get_src()->get_type() == scasm::operand_type::IMM) {
            MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
            scasm_inst->set_type(scasm::instruction_type::MOV);
            scasm_inst->set_asm_type((*it)->get_asm_type());
            scasm_inst->set_src((*it)->get_src());
            MAKE_SHARED(scasm::scasm_operand, dst);
            dst->set_type(scasm::operand_type::REG);
            dst->set_reg(scasm::register_type::R10);
            scasm_inst->set_dst(dst);
            (*it)->set_src(std::move(dst));
            it = funcs->get_instructions().insert(it, std::move(scasm_inst));
            it++;
          }

          if ((*it)->get_dst()->get_type() != scasm::operand_type::REG) {
            MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
            scasm_inst->set_type(scasm::instruction_type::MOV);
            scasm_inst->set_asm_type((*it)->get_dst_type());
            scasm_inst->set_dst((*it)->get_dst());
            MAKE_SHARED(scasm::scasm_operand, src);
            src->set_type(scasm::operand_type::REG);
            src->set_reg(scasm::register_type::R11);
            scasm_inst->set_src(src);
            (*it)->set_dst(std::move(src));
            it =
                funcs->get_instructions().insert(it + 1, std::move(scasm_inst));
          }
        } else {
          // NOTE: MovZeroExtend simply uses movl because movl zeroes out the
          // upper 32 bits of the register
          if ((*it)->get_dst()->get_type() == scasm::operand_type::MEMORY or
              (*it)->get_dst()->get_type() == scasm::operand_type::DATA) {
            // MovZeroExtend(Stack/Data/Reg , Stack/Data)
            //      |
            //      v
            // movl Stack/Data/Reg, %r10d
            // movq %r10d, Stack/Data
            MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
            scasm_inst->set_type(scasm::instruction_type::MOV);
            scasm_inst->set_asm_type((*it)->get_asm_type());
            scasm_inst->set_src((*it)->get_src());
            MAKE_SHARED(scasm::scasm_operand, dst);
            dst->set_type(scasm::operand_type::REG);
            dst->set_reg(scasm::register_type::R10);
            scasm_inst->set_dst(dst);
            (*it)->set_src(std::move(dst));
            (*it)->set_type(scasm::instruction_type::MOV);
            (*it)->set_asm_type((*it)->get_dst_type());
            it = funcs->get_instructions().insert(it, std::move(scasm_inst));
            it++;
          } else {
            // MovZeroExtend(Stack/Data/Reg , Reg)
            //     |
            //     v
            // mov Stack/Data/Reg, Reg
            (*it)->set_type(scasm::instruction_type::MOV);
          }
        }
      } else if ((*it)->get_type() == scasm::instruction_type::BINARY and
                 (*it)->get_binop() == scasm::Binop::MUL and
                 ((*it)->get_dst()->get_type() == scasm::operand_type::MEMORY or
                  (*it)->get_dst()->get_type() == scasm::operand_type::DATA)) {
        // imull $3, STACK/DATA
        //        |
        //        v
        // movl STACK, %r10d
        // imull $3, %r10d
        // movl %r10d, STACK/DATA

        MAKE_SHARED(scasm::scasm_operand, dst);
        dst->set_type(scasm::operand_type::REG);
        dst->set_reg(scasm::register_type::R10);

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
        // <<SRC_TYPE>>  mov $10, %r10l
        //               movsx %r10l, %r10d
        // <<DST_TYPE>>  mov %r10d, STACK/DATA
        if ((*it)->get_src()->get_type() == scasm::operand_type::IMM) {
          MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
          scasm_inst->set_type(scasm::instruction_type::MOV);
          scasm_inst->set_asm_type((*it)->get_asm_type());

          MAKE_SHARED(scasm::scasm_operand, dst);
          dst->set_type(scasm::operand_type::REG);
          dst->set_reg(scasm::register_type::R10);

          scasm_inst->set_src((*it)->get_src());
          scasm_inst->set_dst(dst);

          (*it)->set_src(std::move(dst));

          it = funcs->get_instructions().insert(it, std::move(scasm_inst));
          it++;
        }
        if ((*it)->get_dst()->get_type() != scasm::operand_type::REG) {
          MAKE_SHARED(scasm::scasm_operand, src);
          src->set_type(scasm::operand_type::REG);
          src->set_reg(scasm::register_type::R10);

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
          scasm_inst->set_type(scasm::instruction_type::MOV);
          scasm_inst->set_asm_type((*it)->get_dst_type());
          scasm_inst->set_src(src);
          scasm_inst->set_dst((*it)->get_dst());

          (*it)->set_dst(std::move(src));

          it = funcs->get_instructions().insert(it + 1, std::move(scasm_inst));
        }
      } else if ((*it)->get_type() == scasm::instruction_type::LEA) {
        // lea stack/data/reg, stack/data
        //     |
        //     v
        // lea stack/data/reg, %r10
        // movq %r10, stack/data

        if ((*it)->get_dst()->get_type() != scasm::operand_type::REG) {
          MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
          scasm_inst->set_type(scasm::instruction_type::MOV);
          scasm_inst->set_asm_type((*it)->get_asm_type());
          MAKE_SHARED(scasm::scasm_operand, reg);
          reg->set_type(scasm::operand_type::REG);
          reg->set_reg(scasm::register_type::R10);
          scasm_inst->set_dst((*it)->get_dst());
          scasm_inst->set_src(reg);

          (*it)->set_dst(std::move(reg));

          it = funcs->get_instructions().insert(it + 1, std::move(scasm_inst));
        }
      } else if ((*it)->get_type() == scasm::instruction_type::PUSH) {
        if ((*it)->get_src()->get_type() == scasm::operand_type::REG and
            scasm::RegIsXMM((*it)->get_src()->get_reg())) {
          // pushq reg(xmm)
          //        |
          //        v
          // subq $8, %rsp
          // movq reg(xmm), (%rsp)

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
          scasm_inst->set_type(scasm::instruction_type::BINARY);
          scasm_inst->set_binop(scasm::Binop::SUB);
          scasm_inst->set_asm_type(scasm::AssemblyType::QUAD_WORD);
          MAKE_SHARED(scasm::scasm_operand, scasm_src);
          scasm_src->set_type(scasm::operand_type::IMM);
          constant::Constant cfs;
          cfs.set_type(constant::Type::INT);
          cfs.set_value({.i = 8});
          scasm_src->set_imm(cfs);
          scasm_inst->set_src(std::move(scasm_src));
          MAKE_SHARED(scasm::scasm_operand, scasm_dst);
          scasm_dst->set_type(scasm::operand_type::REG);
          scasm_dst->set_reg(scasm::register_type::SP);
          scasm_inst->set_dst(std::move(scasm_dst));

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst2);
          scasm_inst2->set_type(scasm::instruction_type::MOV);
          scasm_inst2->set_asm_type(scasm::AssemblyType::QUAD_WORD);
          MAKE_SHARED(scasm::scasm_operand, src);
          src->set_type(scasm::operand_type::REG);
          src->set_reg((*it)->get_src()->get_reg());
          scasm_inst2->set_src(src);
          MAKE_SHARED(scasm::scasm_operand, dst);
          dst->set_type(scasm::operand_type::MEMORY);
          dst->set_offset(0);
          dst->set_reg(scasm::register_type::SP);
          scasm_inst2->set_dst(dst);

          it = funcs->get_instructions().insert(it, std::move(scasm_inst));
          it++;
          funcs->get_instructions().erase(it);
          it = funcs->get_instructions().insert(it, std::move(scasm_inst2));
        }
      }
    }

    // Fixing up instructions in which both src and dst are Stack/Data
    /* **NOTE**
     * For this pass, use only R10 register because we use R11 for another pass
     * and the fixes required for some intruction can require both these passes.
     * As such the registers can coalesce
     */
    for (auto it = funcs->get_instructions().begin();
         it != funcs->get_instructions().end(); it++) {
      if (NOTNULL((*it)->get_src()) && NOTNULL((*it)->get_dst()) &&
          ((*it)->get_src()->get_type() == scasm::operand_type::MEMORY or
           (*it)->get_src()->get_type() == scasm::operand_type::DATA) &&
          ((*it)->get_dst()->get_type() == scasm::operand_type::MEMORY or
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

    /* **NOTE**
     * For this pass, use only R11 register because we use R10 for another pass
     * and the fixes required for some intruction can require both these passes.
     * As such the registers can coalesce
     */
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

      // If the source of a movb instruction is an immediate value that can’t
      // fit in a single byte, we will reduce it modulo 256
      if (NOTNULL((*it)->get_src()) and
          (*it)->get_asm_type() == scasm::AssemblyType::BYTE and
          (*it)->get_type() == scasm::instruction_type::MOV and
          (*it)->get_src()->get_type() == scasm::operand_type::IMM and
          (*it)->get_src()->get_imm().get_type() != constant::Type::CHAR and
          (*it)->get_src()->get_imm().get_type() != constant::Type::UCHAR) {
        (*it)->get_src()->set_imm(ast::castConstToElemType(
            (*it)->get_src()->get_imm(), ast::ElemType::UCHAR));
      }
    }
  }
}

} // namespace codegen
} // namespace scarlet
