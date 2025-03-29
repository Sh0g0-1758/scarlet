#include <codegen/common.hh>

namespace scarlet {
namespace codegen {

void Codegen::gen_unop_scasm(std::shared_ptr<scar::scar_Instruction_Node> inst,
                             std::shared_ptr<scasm::scasm_function> scasm_func,
                             scasm::scasm_program &scasm_program) {
  scasm::AssemblyType instType = valToAsmType(inst->get_src1());

  if (inst->get_unop() == unop::UNOP::NOT) {
    if (instType == scasm::AssemblyType::DOUBLE) {
      // Binary(Xor, Double, Reg(<X>), Reg(<X>))
      //  Cmp(Double, src, Reg(<X>))
      //  Mov(<dst type>, Imm(0), dst)
      //  SetCC(E, dst)

      MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
      scasm_inst->set_type(scasm::instruction_type::BINARY);
      scasm_inst->set_binop(scasm::Binop::XOR);
      scasm_inst->set_asm_type(scasm::AssemblyType::DOUBLE);
      MAKE_SHARED(scasm::scasm_operand, scasm_src);
      scasm_src->set_type(scasm::operand_type::REG);
      scasm_src->set_reg(scasm::register_type::XMM14);
      scasm_inst->set_src(std::move(scasm_src));
      MAKE_SHARED(scasm::scasm_operand, scasm_dst);
      scasm_dst->set_type(scasm::operand_type::REG);
      scasm_dst->set_reg(scasm::register_type::XMM14);
      scasm_inst->set_dst(std::move(scasm_dst));
      scasm_func->add_instruction(std::move(scasm_inst));

      MAKE_SHARED(scasm::scasm_instruction, scasm_inst2);
      scasm_inst2->set_type(scasm::instruction_type::CMP);
      scasm_inst2->set_asm_type(scasm::AssemblyType::DOUBLE);
      MAKE_SHARED(scasm::scasm_operand, scasm_src2);
      scasm_src2->set_type(scasm::operand_type::PSEUDO);
      switch (inst->get_src1()->get_type()) {
      case scar::val_type::VAR:
        scasm_src2->set_type(scasm::operand_type::PSEUDO);
        scasm_src2->set_identifier_stack(inst->get_src1()->get_reg());
        break;
      case scar::val_type::CONSTANT:
        scasm_src2->set_type(scasm::operand_type::IMM);
        scasm_src2->set_imm(inst->get_src1()->get_const_val());
        break;
      default:
        break;
      }
      scasm_inst2->set_src(std::move(scasm_src2));
      MAKE_SHARED(scasm::scasm_operand, scasm_dst2);
      scasm_dst2->set_type(scasm::operand_type::REG);
      scasm_dst2->set_reg(scasm::register_type::XMM14);
      scasm_inst2->set_dst(std::move(scasm_dst2));
      scasm_func->add_instruction(std::move(scasm_inst2));

      MAKE_SHARED(scasm::scasm_instruction, scasm_inst3);
      scasm_inst3->set_type(scasm::instruction_type::MOV);
      scasm_inst3->set_asm_type(valToAsmType(inst->get_dst()));
      MAKE_SHARED(scasm::scasm_operand, scasm_src3);
      scasm_src3->set_type(scasm::operand_type::IMM);
      constant::Constant zero;
      zero.set_type(constant::Type::INT);
      zero.set_value({.i = 0});
      scasm_src3->set_imm(zero);
      scasm_inst3->set_src(std::move(scasm_src3));
      MAKE_SHARED(scasm::scasm_operand, scasm_dst3);
      switch (inst->get_dst()->get_type()) {
      case scar::val_type::VAR:
        scasm_dst3->set_type(scasm::operand_type::PSEUDO);
        scasm_dst3->set_identifier_stack(inst->get_dst()->get_reg());
        break;
      default:
        break;
      }
      scasm_inst3->set_dst(std::move(scasm_dst3));
      scasm_func->add_instruction(std::move(scasm_inst3));

      MAKE_SHARED(scasm::scasm_instruction, scasm_inst4);
      scasm_inst4->set_type(scasm::instruction_type::SETCC);
      scasm_inst4->set_asm_type(scasm::AssemblyType::BYTE);
      MAKE_SHARED(scasm::scasm_operand, scasm_src4);
      scasm_src4->set_type(scasm::operand_type::COND);
      scasm_src4->set_cond(scasm::cond_code::E);
      scasm_inst4->set_src(std::move(scasm_src4));
      MAKE_SHARED(scasm::scasm_operand, scasm_dst4);
      switch (inst->get_dst()->get_type()) {
      case scar::val_type::VAR:
        scasm_dst4->set_type(scasm::operand_type::PSEUDO);
        scasm_dst4->set_identifier_stack(inst->get_dst()->get_reg());
        break;
      default:
        break;
      }
      scasm_inst4->set_dst(std::move(scasm_dst4));
      scasm_func->add_instruction(std::move(scasm_inst4));
    }
    // Cmp(Imm(0), src)
    // Mov(Imm(0), dst)
    // SetCC(E, dst)

    MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
    scasm_inst->set_type(scasm::instruction_type::CMP);
    scasm_inst->set_asm_type(instType);
    MAKE_SHARED(scasm::scasm_operand, scasm_src);
    scasm_src->set_type(scasm::operand_type::IMM);
    constant::Constant zero;
    zero.set_type(constant::Type::INT);
    zero.set_value({.i = 0});
    scasm_src->set_imm(zero);
    scasm_inst->set_src(std::move(scasm_src));
    MAKE_SHARED(scasm::scasm_operand, scasm_dst);
    switch (inst->get_src1()->get_type()) {
    case scar::val_type::VAR:
      scasm_dst->set_type(scasm::operand_type::PSEUDO);
      scasm_dst->set_identifier_stack(inst->get_src1()->get_reg());
      break;
    case scar::val_type::CONSTANT:
      scasm_dst->set_type(scasm::operand_type::IMM);
      scasm_dst->set_imm(inst->get_src1()->get_const_val());
      break;
    default:
      break;
    }
    scasm_inst->set_dst(std::move(scasm_dst));
    scasm_func->add_instruction(std::move(scasm_inst));

    MAKE_SHARED(scasm::scasm_instruction, scasm_inst2);
    scasm_inst2->set_type(scasm::instruction_type::MOV);
    scasm_inst2->set_asm_type(valToAsmType(inst->get_dst()));
    MAKE_SHARED(scasm::scasm_operand, scasm_src2);
    scasm_src2->set_type(scasm::operand_type::IMM);
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
    scasm_src3->set_cond(scasm::cond_code::E);
    scasm_inst3->set_src(std::move(scasm_src3));
    MAKE_SHARED(scasm::scasm_operand, scasm_dst3);
    switch (inst->get_dst()->get_type()) {
    case scar::val_type::VAR:
      scasm_dst3->set_type(scasm::operand_type::PSEUDO);
      scasm_dst3->set_identifier_stack(inst->get_dst()->get_reg());
      break;
    default:
      break;
    }
    scasm_inst3->set_dst(std::move(scasm_dst3));
    scasm_func->add_instruction(std::move(scasm_inst3));
  } else {
    MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
    scasm_inst->set_type(scasm::instruction_type::MOV);
    scasm_inst->set_asm_type(instType);
    MAKE_SHARED(scasm::scasm_operand, scasm_src);
    SET_OPERAND(scasm_src, set_src, get_src1, scasm_inst);

    MAKE_SHARED(scasm::scasm_operand, scasm_dst);
    SET_OPERAND(scasm_dst, set_dst, get_dst, scasm_inst);

    scasm_func->add_instruction(std::move(scasm_inst));

    MAKE_SHARED(scasm::scasm_instruction, scasm_inst2);
    scasm_inst2->set_type(scasm::instruction_type::UNARY);
    scasm_inst2->set_asm_type(instType);
    scasm_inst2->set_unop(inst->get_unop());

    scasm_inst2->set_dst(std::move(scasm_dst));

    scasm_func->add_instruction(std::move(scasm_inst2));
  }
}

} // namespace codegen
} // namespace scarlet
