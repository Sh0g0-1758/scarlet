#include <codegen/common.hh>

namespace scarlet {
namespace codegen {

void Codegen::gen_unop_scasm(std::shared_ptr<scar::scar_Instruction_Node> inst,
                             std::shared_ptr<scasm::scasm_function> scasm_func,
                             scasm::scasm_program &scasm_program) {
  scasm::AssemblyType instType = valToAsmType(inst->get_src1());

  if (inst->get_unop() == unop::UNOP::NOT) {
    // IF DOUBLE
    //  Binary(Xor, Reg(<X>), Reg(<X>))
    //  Cmp(src, Reg(<X>))
    //  Mov(Imm(0), dst)
    //  SetCC(E, dst)
    // ELSE
    //  Cmp(Imm(0), src)
    //  Mov(Imm(0), dst)
    //  SetCC(E, dst)
    if (instType == scasm::AssemblyType::DOUBLE) {
      MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
      scasm_inst->set_type(scasm::instruction_type::BINARY);
      scasm_inst->set_binop(scasm::Binop::XOR);
      scasm_inst->set_asm_type(scasm::AssemblyType::DOUBLE);
      MAKE_SHARED(scasm::scasm_operand, scasm_reg);
      scasm_reg->set_type(scasm::operand_type::REG);
      scasm_reg->set_reg(scasm::register_type::XMM0);
      scasm_inst->set_src(scasm_reg);
      scasm_inst->set_dst(scasm_reg);
      scasm_func->add_instruction(std::move(scasm_inst));

      MAKE_SHARED(scasm::scasm_instruction, scasm_inst2);
      scasm_inst2->set_type(scasm::instruction_type::CMP);
      scasm_inst2->set_asm_type(scasm::AssemblyType::DOUBLE);
      MAKE_SHARED(scasm::scasm_operand, scasm_src2);
      SET_OPERAND(scasm_src2, set_src, get_src1, scasm_inst2);
      scasm_inst2->set_dst(std::move(scasm_reg));
      scasm_func->add_instruction(std::move(scasm_inst2));
    } else {
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
      SET_OPERAND(scasm_dst, set_dst, get_src1, scasm_inst);
      scasm_func->add_instruction(std::move(scasm_inst));
    }

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
    scasm_src3->set_cond(scasm::cond_code::E);
    scasm_inst3->set_src(std::move(scasm_src3));
    MAKE_SHARED(scasm::scasm_operand, scasm_dst3);
    SET_OPERAND(scasm_dst3, set_dst, get_dst, scasm_inst3);
    scasm_func->add_instruction(std::move(scasm_inst3));
  } else if (inst->get_unop() == unop::UNOP::NEGATE and
             instType == scasm::AssemblyType::DOUBLE) {
    // mov(src, dst)
    // xor(-0.0, dst)
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
    scasm_inst2->set_binop(scasm::Binop::XOR);

    MAKE_SHARED(scasm::scasm_operand, scasm_src2);
    constant::Constant negzero;
    negzero.set_type(constant::Type::DOUBLE);
    negzero.set_value({.d = -0.0});
    MAKE_DOUBLE_CONSTANT(scasm_src2, negzero, 16);
    scasm_inst2->set_src(std::move(scasm_src2));

    MAKE_SHARED(scasm::scasm_operand, scasm_dst2);
    SET_OPERAND(scasm_dst2, set_dst, get_dst, scasm_inst2);

    scasm_func->add_instruction(std::move(scasm_inst2));
  } else {
    MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
    scasm_inst->set_type(scasm::instruction_type::MOV);
    scasm_inst->set_asm_type(instType);
    MAKE_SHARED(scasm::scasm_operand, scasm_src);
    SET_OPERAND(scasm_src, set_src, get_src1, scasm_inst);

    MAKE_SHARED(scasm::scasm_operand, scasm_dst);
    SETVAL_OPERAND(scasm_dst, get_dst);
    scasm_inst->set_dst(scasm_dst);

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
