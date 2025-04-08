#include <codegen/common.hh>

namespace scarlet {
namespace codegen {

void Codegen::gen_binop_scasm(std::shared_ptr<scar::scar_Instruction_Node> inst,
                              std::shared_ptr<scasm::scasm_function> scasm_func,
                              scasm::scasm_program &scasm_program) {
  scasm::AssemblyType instType = valToAsmType(inst->get_src1());

  if (binop::is_relational(inst->get_binop())) {
    // Cmp(src2, src1)
    // Mov(Imm(0), dst)
    // SetCC(conditional, dst)
    MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
    scasm_inst->set_type(scasm::instruction_type::CMP);
    scasm_inst->set_asm_type(instType);
    MAKE_SHARED(scasm::scasm_operand, scasm_src);
    SET_OPERAND(scasm_src, set_src, get_src2, scasm_inst);
    MAKE_SHARED(scasm::scasm_operand, scasm_dst);
    SET_OPERAND(scasm_dst, set_dst, get_src1, scasm_inst);
    scasm_func->add_instruction(std::move(scasm_inst));

    MAKE_SHARED(scasm::scasm_instruction, scasm_inst2);
    scasm_inst2->set_type(scasm::instruction_type::MOV);
    scasm_inst2->set_asm_type(valToAsmType(inst->get_dst()));
    MAKE_SHARED(scasm::scasm_operand, scasm_src2);
    scasm_src2->set_type(scasm::operand_type::IMM);
    constant::Constant zero;
    zero.set_type(constant::Type::INT);
    zero.set_value({.i = 0});
    scasm_src2->set_imm(zero);
    scasm_inst2->set_src(std::move(scasm_src2));
    MAKE_SHARED(scasm::scasm_operand, scasm_dst2);
    SET_OPERAND(scasm_dst2, set_dst, get_dst, scasm_inst2);
    scasm_func->add_instruction(std::move(scasm_inst2));

    MAKE_SHARED(scasm::scasm_instruction, scasm_inst3);
    scasm_inst3->set_type(scasm::instruction_type::SETCC);
    scasm_inst3->set_asm_type(scasm::AssemblyType::BYTE);
    MAKE_SHARED(scasm::scasm_operand, scasm_src3);
    scasm_src3->set_type(scasm::operand_type::COND);
    if (valToConstType(inst->get_src1()) == constant::Type::UINT or
        valToConstType(inst->get_src1()) == constant::Type::ULONG or
        valToConstType(inst->get_src1()) == constant::Type::DOUBLE) {
      switch (inst->get_binop()) {
      case binop::BINOP::EQUAL:
        scasm_src3->set_cond(scasm::cond_code::E);
        break;
      case binop::BINOP::NOTEQUAL:
        scasm_src3->set_cond(scasm::cond_code::NE);
        break;
      case binop::BINOP::LESSTHAN:
        scasm_src3->set_cond(scasm::cond_code::B);
        break;
      case binop::BINOP::LESSTHANEQUAL:
        scasm_src3->set_cond(scasm::cond_code::BE);
        break;
      case binop::BINOP::GREATERTHAN:
        scasm_src3->set_cond(scasm::cond_code::A);
        break;
      case binop::BINOP::GREATERTHANEQUAL:
        scasm_src3->set_cond(scasm::cond_code::AE);
        break;
      default:
        break;
      }
    } else {
      switch (inst->get_binop()) {
      case binop::BINOP::EQUAL:
        scasm_src3->set_cond(scasm::cond_code::E);
        break;
      case binop::BINOP::NOTEQUAL:
        scasm_src3->set_cond(scasm::cond_code::NE);
        break;
      case binop::BINOP::LESSTHAN:
        scasm_src3->set_cond(scasm::cond_code::L);
        break;
      case binop::BINOP::LESSTHANEQUAL:
        scasm_src3->set_cond(scasm::cond_code::LE);
        break;
      case binop::BINOP::GREATERTHAN:
        scasm_src3->set_cond(scasm::cond_code::G);
        break;
      case binop::BINOP::GREATERTHANEQUAL:
        scasm_src3->set_cond(scasm::cond_code::GE);
        break;
      default:
        break;
      }
    }
    scasm_inst3->set_src(std::move(scasm_src3));
    MAKE_SHARED(scasm::scasm_operand, scasm_dst3);
    SET_OPERAND(scasm_dst3, set_dst, get_dst, scasm_inst3);
    scasm_func->add_instruction(std::move(scasm_inst3));
  } else if ((inst->get_binop() == binop::BINOP::DIV or
              inst->get_binop() == binop::BINOP::MOD) and
             instType !=
                 scasm::AssemblyType::DOUBLE) { /* double division is handled
                                                   like integer mul|add|sub */
    // Mov(<src1 type>, src1, Reg(AX))
    // unsigned: Mov(<src1 type>, Imm(0), Reg(DX)) | signed: Cdq
    // unsigned: Div(<src1 type>, src2) | signed: Idiv(<src1 type>, src2)
    // Mov(<src1 type>, Reg(AX), dst) | Mov(<src1 type>, Reg(DX), dst)
    MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
    scasm_inst->set_type(scasm::instruction_type::MOV);
    scasm_inst->set_asm_type(instType);
    MAKE_SHARED(scasm::scasm_operand, scasm_src);
    SET_OPERAND(scasm_src, set_src, get_src1, scasm_inst);
    MAKE_SHARED(scasm::scasm_operand, scasm_dst);
    scasm_dst->set_type(scasm::operand_type::REG);
    scasm_dst->set_reg(scasm::register_type::AX);
    scasm_inst->set_dst(std::move(scasm_dst));
    scasm_func->add_instruction(std::move(scasm_inst));

    if (valToConstType(inst->get_src1()) == constant::Type::UINT or
        valToConstType(inst->get_src1()) == constant::Type::ULONG) {
      // Mov(<src1 type>, Imm(0), Reg(DX))
      MAKE_SHARED(scasm::scasm_instruction, scasm_inst2);
      scasm_inst2->set_type(scasm::instruction_type::MOV);
      scasm_inst2->set_asm_type(instType);
      MAKE_SHARED(scasm::scasm_operand, scasm_src1);
      scasm_src1->set_type(scasm::operand_type::IMM);
      constant::Constant zero;
      zero.set_type(constant::Type::INT);
      zero.set_value({.i = 0});
      scasm_src1->set_imm(zero);
      scasm_inst2->set_src(std::move(scasm_src1));
      MAKE_SHARED(scasm::scasm_operand, scasm_dst2);
      scasm_dst2->set_type(scasm::operand_type::REG);
      scasm_dst2->set_reg(scasm::register_type::DX);
      scasm_inst2->set_dst(std::move(scasm_dst2));
      scasm_func->add_instruction(std::move(scasm_inst2));
    } else {
      // cdq
      MAKE_SHARED(scasm::scasm_instruction, scasm_inst2);
      scasm_inst2->set_type(scasm::instruction_type::CDQ);
      scasm_inst2->set_asm_type(instType);
      scasm_func->add_instruction(std::move(scasm_inst2));
    }

    MAKE_SHARED(scasm::scasm_instruction, scasm_inst3);
    if (valToConstType(inst->get_src1()) == constant::Type::UINT or
        valToConstType(inst->get_src1()) == constant::Type::ULONG) {
      scasm_inst3->set_type(scasm::instruction_type::DIV);
    } else {
      scasm_inst3->set_type(scasm::instruction_type::IDIV);
    }
    scasm_inst3->set_asm_type(instType);
    MAKE_SHARED(scasm::scasm_operand, scasm_src2);
    SET_OPERAND(scasm_src2, set_src, get_src2, scasm_inst3);
    scasm_func->add_instruction(std::move(scasm_inst3));

    MAKE_SHARED(scasm::scasm_instruction, scasm_inst4);
    scasm_inst4->set_type(scasm::instruction_type::MOV);
    scasm_inst4->set_asm_type(instType);
    MAKE_SHARED(scasm::scasm_operand, scasm_dst3);
    SET_OPERAND(scasm_dst3, set_dst, get_dst, scasm_inst4);

    MAKE_SHARED(scasm::scasm_operand, scasm_src3);
    scasm_src3->set_type(scasm::operand_type::REG);
    if (inst->get_binop() == binop::BINOP::DIV) {
      scasm_src3->set_reg(scasm::register_type::AX);
    } else {
      scasm_src3->set_reg(scasm::register_type::DX);
    }
    scasm_inst4->set_src(std::move(scasm_src3));
    scasm_func->add_instruction(std::move(scasm_inst4));
  } else if (inst->get_binop() == binop::BINOP::LEFT_SHIFT or
             inst->get_binop() == binop::BINOP::RIGHT_SHIFT or
             inst->get_binop() == binop::BINOP::LOGICAL_RIGHT_SHIFT or
             inst->get_binop() == binop::BINOP::LOGICAL_LEFT_SHIFT) {
    // Mov(src1, dst)
    // Mov(src2, Reg(CX))
    // Binary(binary operand, CL, dst)

    MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
    scasm_inst->set_type(scasm::instruction_type::MOV);
    scasm_inst->set_asm_type(instType);
    MAKE_SHARED(scasm::scasm_operand, scasm_src);
    SET_OPERAND(scasm_src, set_src, get_src1, scasm_inst);
    MAKE_SHARED(scasm::scasm_operand, scasm_dst);
    SET_OPERAND(scasm_dst, set_dst, get_dst, scasm_inst);
    scasm_func->add_instruction(std::move(scasm_inst));

    MAKE_SHARED(scasm::scasm_instruction, scasm_inst2);
    scasm_inst2->set_type(scasm::instruction_type::MOV);
    scasm_inst2->set_asm_type(instType);
    MAKE_SHARED(scasm::scasm_operand, scasm_src2);
    SET_OPERAND(scasm_src2, set_src, get_src2, scasm_inst2);
    MAKE_SHARED(scasm::scasm_operand, scasm_dst2);
    scasm_dst2->set_type(scasm::operand_type::REG);
    scasm_dst2->set_reg(scasm::register_type::CX);
    scasm_inst2->set_dst(std::move(scasm_dst2));
    scasm_func->add_instruction(std::move(scasm_inst2));

    MAKE_SHARED(scasm::scasm_instruction, scasm_inst3);
    scasm_inst3->set_type(scasm::instruction_type::BINARY);
    scasm_inst3->set_asm_type(instType);
    scasm_inst3->set_binop(scasm::scar_binop_to_scasm_binop(inst->get_binop()));
    MAKE_SHARED(scasm::scasm_operand, scasm_src3);
    scasm_src3->set_type(scasm::operand_type::REG);
    scasm_src3->set_reg(scasm::register_type::CL);
    scasm_inst3->set_src(std::move(scasm_src3));
    MAKE_SHARED(scasm::scasm_operand, scasm_dst3);
    scasm_dst3->set_type(scasm::operand_type::PSEUDO);
    scasm_dst3->set_identifier(inst->get_dst()->get_reg());
    scasm_inst3->set_dst(std::move(scasm_dst3));
    scasm_func->add_instruction(std::move(scasm_inst3));

  } else {
    // Mov(src1, dst)
    // Binary(binary_operator, src2, dst)

    MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
    scasm_inst->set_type(scasm::instruction_type::MOV);
    scasm_inst->set_asm_type(instType);
    MAKE_SHARED(scasm::scasm_operand, scasm_src);
    SET_OPERAND(scasm_src, set_src, get_src1, scasm_inst);
    MAKE_SHARED(scasm::scasm_operand, scasm_dst);
    SET_OPERAND(scasm_dst, set_dst, get_dst, scasm_inst);
    scasm_func->add_instruction(std::move(scasm_inst));

    MAKE_SHARED(scasm::scasm_instruction, scasm_inst2);
    scasm_inst2->set_type(scasm::instruction_type::BINARY);
    scasm_inst2->set_asm_type(instType);
    scasm_inst2->set_binop(scasm::scar_binop_to_scasm_binop(inst->get_binop()));
    MAKE_SHARED(scasm::scasm_operand, scasm_src2);
    SET_OPERAND(scasm_src2, set_src, get_src2, scasm_inst2);
    MAKE_SHARED(scasm::scasm_operand, scasm_dst2);
    if (inst->get_dst()->get_type() == scar::val_type::VAR) {
      scasm_dst2->set_type(scasm::operand_type::PSEUDO);
      scasm_dst2->set_identifier(inst->get_dst()->get_reg());
    }
    scasm_inst2->set_dst(std::move(scasm_dst2));
    scasm_func->add_instruction(std::move(scasm_inst2));
  }
}

} // namespace codegen
} // namespace scarlet
