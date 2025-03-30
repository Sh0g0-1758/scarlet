#include <codegen/common.hh>

namespace scarlet {
namespace codegen {

void Codegen::gen_funcall_scasm(
    std::shared_ptr<scar::scar_Instruction_Node> inst,
    std::shared_ptr<scasm::scasm_function> scasm_func,
    scasm::scasm_program &scasm_program) {
  auto funcCall =
      std::static_pointer_cast<scar::scar_FunctionCall_Instruction_Node>(inst);
  int numArgs = funcCall->get_args().size();
  std::vector<constant::Type> param_types;
  for (int i = 0; i < numArgs; i++) {
    param_types.push_back(valToConstType(funcCall->get_args()[i]));
  }

  std::vector<std::pair<scasm::AssemblyType, int>> int_param_indx;
  std::vector<int> double_param_indx;
  std::vector<std::pair<scasm::AssemblyType, int>> stack_param_indx;
  calssify_parameters(param_types, int_param_indx, double_param_indx,
                      stack_param_indx);

  bool stack_padding = stack_param_indx.size() % 2;
  if (stack_padding) {
    MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
    scasm_inst->set_type(scasm::instruction_type::BINARY);
    scasm_inst->set_binop(scasm::Binop::SUB);
    scasm_inst->set_asm_type(scasm::AssemblyType::QUAD_WORD);

    MAKE_SHARED(scasm::scasm_operand, scasm_src);
    scasm_src->set_type(scasm::operand_type::IMM);
    constant::Constant cpd;
    cpd.set_type(constant::Type::INT);
    cpd.set_value({.i = 8});
    scasm_src->set_imm(cpd);
    scasm_inst->set_src(std::move(scasm_src));

    MAKE_SHARED(scasm::scasm_operand, scasm_dst);
    scasm_dst->set_type(scasm::operand_type::REG);
    scasm_dst->set_reg(scasm::register_type::SP);
    scasm_inst->set_dst(std::move(scasm_dst));

    scasm_func->add_instruction(std::move(scasm_inst));
  }

  // convert functions args to reg/stack
  // Move int args
  for (int i = 0; i < (int)int_param_indx.size(); i++) {
    MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
    scasm_inst->set_type(scasm::instruction_type::MOV);
    scasm_inst->set_asm_type(int_param_indx[i].first);
    MAKE_SHARED(scasm::scasm_operand, scasm_src);
    switch (funcCall->get_args()[int_param_indx[i].second]->get_type()) {
    case scar::val_type::VAR:
      scasm_src->set_type(scasm::operand_type::PSEUDO);
      scasm_src->set_identifier_stack(
          funcCall->get_args()[int_param_indx[i].second]->get_reg());
      break;
    case scar::val_type::CONSTANT:
      // only quad word and long word are allowed
      scasm_src->set_type(scasm::operand_type::IMM);
      scasm_src->set_imm(
          funcCall->get_args()[int_param_indx[i].second]->get_const_val());
      break;
    default:
      break;
    }
    scasm_inst->set_src(std::move(scasm_src));
    MAKE_SHARED(scasm::scasm_operand, scasm_dst);
    scasm_dst->set_type(scasm::operand_type::REG);
    scasm_dst->set_reg(int_argReg[i]);
    scasm_inst->set_dst(std::move(scasm_dst));
    scasm_func->add_instruction(std::move(scasm_inst));
  }

  // Move double args
  for (int i = 0; i < (int)double_param_indx.size(); i++) {
    MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
    scasm_inst->set_type(scasm::instruction_type::MOV);
    scasm_inst->set_asm_type(scasm::AssemblyType::DOUBLE);
    MAKE_SHARED(scasm::scasm_operand, scasm_src);
    switch (funcCall->get_args()[double_param_indx[i]]->get_type()) {
    case scar::val_type::VAR:
      scasm_src->set_type(scasm::operand_type::PSEUDO);
      scasm_src->set_identifier_stack(
          funcCall->get_args()[double_param_indx[i]]->get_reg());
      break;
    case scar::val_type::CONSTANT: {
      MAKE_DOUBLE_CONSTANT(
          scasm_src,
          funcCall->get_args()[double_param_indx[i]]->get_const_val(), 8);
    } break;
    default:
      break;
    }
    scasm_inst->set_src(std::move(scasm_src));
    MAKE_SHARED(scasm::scasm_operand, scasm_dst);
    scasm_dst->set_type(scasm::operand_type::REG);
    scasm_dst->set_reg(double_argReg[i]);
    scasm_inst->set_dst(std::move(scasm_dst));
    scasm_func->add_instruction(std::move(scasm_inst));
  }

  // Move stack args
  for (int i = stack_param_indx.size() - 1; i >= 0; i--) {
    MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
    MAKE_SHARED(scasm::scasm_operand, scasm_src);
    scasm_inst->set_asm_type(stack_param_indx[i].first);
    switch (funcCall->get_args()[stack_param_indx[i].second]->get_type()) {
    case scar::val_type::VAR:
      scasm_src->set_type(scasm::operand_type::PSEUDO);
      scasm_src->set_identifier_stack(funcCall->get_args()[i]->get_reg());
      break;
    case scar::val_type::CONSTANT: {
      if (funcCall->get_args()[i]->get_const_val().get_type() ==
          constant::Type::DOUBLE) {
        MAKE_DOUBLE_CONSTANT(scasm_src,
                             funcCall->get_args()[i]->get_const_val(), 8);
      } else {
        scasm_src->set_type(scasm::operand_type::IMM);
        scasm_src->set_imm(funcCall->get_args()[i]->get_const_val());
      }
    } break;
    default:
      break;
    }

    if (scasm_src->get_type() == scasm::operand_type::IMM or
        scasm_inst->get_asm_type() == scasm::AssemblyType::QUAD_WORD or
        scasm_inst->get_asm_type() == scasm::AssemblyType::DOUBLE) {
      scasm_inst->set_src(std::move(scasm_src));
      scasm_inst->set_type(scasm::instruction_type::PUSH);
      scasm_inst->set_asm_type(scasm::AssemblyType::QUAD_WORD);
      scasm_func->add_instruction(std::move(scasm_inst));
    } else {
      scasm_inst->set_src(std::move(scasm_src));
      scasm_inst->set_type(scasm::instruction_type::MOV);
      MAKE_SHARED(scasm::scasm_operand, scasm_dst);
      scasm_dst->set_type(scasm::operand_type::REG);
      scasm_dst->set_reg(scasm::register_type::AX);
      scasm_inst->set_dst(std::move(scasm_dst));
      scasm_func->add_instruction(std::move(scasm_inst));

      MAKE_SHARED(scasm::scasm_instruction, scasm_inst2);
      scasm_inst2->set_type(scasm::instruction_type::PUSH);
      scasm_inst2->set_asm_type(scasm::AssemblyType::QUAD_WORD);
      MAKE_SHARED(scasm::scasm_operand, scasm_src2);
      scasm_src2->set_type(scasm::operand_type::REG);
      scasm_src2->set_reg(scasm::register_type::AX);
      scasm_inst2->set_src(std::move(scasm_src2));
      scasm_func->add_instruction(std::move(scasm_inst2));
    }
  }

  MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
  scasm_inst->set_type(scasm::instruction_type::CALL);
  MAKE_SHARED(scasm::scasm_operand, scasm_src);
  scasm_src->set_type(scasm::operand_type::LABEL);
  scasm_src->set_identifier_stack(funcCall->get_name()->get_value());
  scasm_inst->set_src(std::move(scasm_src));
  scasm_func->add_instruction(std::move(scasm_inst));

  int bytesToRemove = stack_param_indx.size() * 8;
  bytesToRemove += stack_padding ? 8 : 0;

  if (bytesToRemove != 0) {
    MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
    scasm_inst->set_type(scasm::instruction_type::BINARY);
    scasm_inst->set_binop(scasm::Binop::ADD);
    scasm_inst->set_asm_type(scasm::AssemblyType::QUAD_WORD);

    MAKE_SHARED(scasm::scasm_operand, scasm_src);
    scasm_src->set_type(scasm::operand_type::IMM);
    constant::Constant cbtr;
    cbtr.set_type(constant::Type::INT);
    cbtr.set_value({.i = bytesToRemove});
    scasm_src->set_imm(cbtr);
    scasm_inst->set_src(std::move(scasm_src));

    MAKE_SHARED(scasm::scasm_operand, scasm_dst);
    scasm_dst->set_type(scasm::operand_type::REG);
    scasm_dst->set_reg(scasm::register_type::SP);
    scasm_inst->set_dst(std::move(scasm_dst));

    scasm_func->add_instruction(std::move(scasm_inst));
  }

  MAKE_SHARED(scasm::scasm_instruction, scasm_inst3);
  scasm_inst3->set_type(scasm::instruction_type::MOV);
  scasm_inst3->set_asm_type(valToAsmType(inst->get_dst()));
  MAKE_SHARED(scasm::scasm_operand, scasm_src3);
  scasm_src3->set_type(scasm::operand_type::REG);
  if (valToConstType(inst->get_dst()) == constant::Type::DOUBLE) {
    scasm_src3->set_reg(scasm::register_type::XMM0);
  } else {
    scasm_src3->set_reg(scasm::register_type::AX);
  }
  scasm_inst3->set_src(std::move(scasm_src3));
  MAKE_SHARED(scasm::scasm_operand, scasm_dst3);
  SET_OPERAND(scasm_dst3, set_dst, get_dst, scasm_inst3);
  scasm_func->add_instruction(std::move(scasm_inst3));
}

} // namespace codegen
} // namespace scarlet
