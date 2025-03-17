#include <codegen/codegen.hh>

namespace scarlet {
namespace codegen {

#define SET_MOV_SOURCE()                                                       \
  switch (inst->get_src1()->get_type()) {                                      \
  case scar::val_type::VAR:                                                    \
    scasm_src->set_type(scasm::operand_type::PSEUDO);                          \
    scasm_src->set_identifier_stack(inst->get_src1()->get_reg());              \
    break;                                                                     \
  case scar::val_type::CONSTANT:                                               \
    scasm_src->set_type(scasm::operand_type::IMM);                             \
    scasm_src->set_imm(stoi(inst->get_src1()->get_value()));                   \
    break;                                                                     \
  default:                                                                     \
    break;                                                                     \
  }                                                                            \
  scasm_inst->set_src(std::move(scasm_src))

#define SET_DST(dst)                                                           \
  switch (inst->get_dst()->get_type()) {                                       \
  case scar::val_type::VAR:                                                    \
    dst->set_type(scasm::operand_type::PSEUDO);                                \
    dst->set_identifier_stack(inst->get_dst()->get_reg());                     \
    break;                                                                     \
  default:                                                                     \
    break;                                                                     \
  }

void Codegen::gen_scasm() {
  scasm::scasm_program scasm_program{};
  for (auto elem : scar.get_elems()) {
    if (elem->get_type() == scar::topLevelType::STATICVARIABLE)
      continue;
    auto func = std::static_pointer_cast<scar::scar_Function_Node>(elem);
    MAKE_SHARED(scasm::scasm_function, scasm_func);
    scasm_func->set_name(func->get_identifier()->get_value());

    // Move function args from registers and stack to the callee stack frame
    int numParams = func->get_params().size();
    for (int i = 0; i < std::min(6, numParams); i++) {
      MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
      scasm_inst->set_type(scasm::instruction_type::MOV);
      MAKE_SHARED(scasm::scasm_operand, scasm_src);
      scasm_src->set_type(scasm::operand_type::REG);
      scasm_src->set_reg(argReg[i]);
      scasm_inst->set_src(std::move(scasm_src));
      MAKE_SHARED(scasm::scasm_operand, scasm_dst);
      scasm_dst->set_type(scasm::operand_type::PSEUDO);
      scasm_dst->set_identifier_stack(func->get_params()[i]->get_value());
      scasm_inst->set_dst(std::move(scasm_dst));
      scasm_func->add_instruction(std::move(scasm_inst));
    }
    if (numParams > 6) {
      for (int i = 6; i < numParams; i++) {
        MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
        scasm_inst->set_type(scasm::instruction_type::MOV);
        MAKE_SHARED(scasm::scasm_operand, scasm_src);
        scasm_src->set_type(scasm::operand_type::STACK);
        scasm_src->set_identifier_stack(std::to_string(16 + 8 * (i - 6)) +
                                        "(%rbp)");
        scasm_inst->set_src(std::move(scasm_src));
        MAKE_SHARED(scasm::scasm_operand, scasm_dst);
        scasm_dst->set_type(scasm::operand_type::PSEUDO);
        scasm_dst->set_identifier_stack(func->get_params()[i]->get_value());
        scasm_inst->set_dst(std::move(scasm_dst));
        scasm_func->add_instruction(std::move(scasm_inst));
      }
    }
    for (auto inst : func->get_instructions()) {
      if (inst->get_type() == scar::instruction_type::RETURN) {
        MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
        scasm_inst->set_type(scasm::instruction_type::MOV);
        MAKE_SHARED(scasm::scasm_operand, scasm_src);
        SET_MOV_SOURCE();
        MAKE_SHARED(scasm::scasm_operand, scasm_dst);
        scasm_dst->set_type(scasm::operand_type::REG);
        scasm_dst->set_reg(scasm::register_type::AX);
        scasm_inst->set_dst(std::move(scasm_dst));
        scasm_func->add_instruction(std::move(scasm_inst));

        MAKE_SHARED(scasm::scasm_instruction, scasm_inst2);
        scasm_inst2->set_type(scasm::instruction_type::RET);
        scasm_func->add_instruction(std::move(scasm_inst2));

      } else if (inst->get_type() == scar::instruction_type::UNARY) {
        if (inst->get_unop() == unop::UNOP::NOT) {
          // Cmp(Imm(0), src)
          // Mov(Imm(0), dst)
          // SetCC(E, dst)

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
          scasm_inst->set_type(scasm::instruction_type::CMP);
          MAKE_SHARED(scasm::scasm_operand, scasm_src);
          scasm_src->set_type(scasm::operand_type::IMM);
          scasm_src->set_imm(0);
          scasm_inst->set_src(std::move(scasm_src));
          MAKE_SHARED(scasm::scasm_operand, scasm_dst);
          switch (inst->get_src1()->get_type()) {
          case scar::val_type::VAR:
            scasm_dst->set_type(scasm::operand_type::PSEUDO);
            scasm_dst->set_identifier_stack(inst->get_src1()->get_reg());
            break;
          case scar::val_type::CONSTANT:
            scasm_dst->set_type(scasm::operand_type::IMM);
            scasm_dst->set_imm(stoi(inst->get_src1()->get_value()));
            break;
          default:
            break;
          }
          scasm_inst->set_dst(std::move(scasm_dst));
          scasm_func->add_instruction(std::move(scasm_inst));

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst2);
          scasm_inst2->set_type(scasm::instruction_type::MOV);
          MAKE_SHARED(scasm::scasm_operand, scasm_src2);
          scasm_src2->set_type(scasm::operand_type::IMM);
          scasm_src2->set_imm(0);
          scasm_inst2->set_src(std::move(scasm_src2));
          MAKE_SHARED(scasm::scasm_operand, scasm_dst2);
          SET_DST(scasm_dst2);
          scasm_inst2->set_dst(std::move(scasm_dst2));
          scasm_func->add_instruction(std::move(scasm_inst2));

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst3);
          scasm_inst3->set_type(scasm::instruction_type::SETCC);
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
          MAKE_SHARED(scasm::scasm_operand, scasm_src);
          SET_MOV_SOURCE();

          MAKE_SHARED(scasm::scasm_operand, scasm_dst);
          if (inst->get_dst()->get_type() == scar::val_type::VAR) {
            scasm_dst->set_type(scasm::operand_type::PSEUDO);
            scasm_dst->set_identifier_stack(inst->get_dst()->get_reg());
          }
          scasm_inst->set_dst(std::move(scasm_dst));

          scasm_func->add_instruction(std::move(scasm_inst));

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst2);
          scasm_inst2->set_type(scasm::instruction_type::UNARY);
          scasm_inst2->set_unop(inst->get_unop());

          MAKE_SHARED(scasm::scasm_operand, scasm_dst2);
          if (inst->get_dst()->get_type() == scar::val_type::VAR) {
            scasm_dst2->set_type(scasm::operand_type::PSEUDO);
            scasm_dst2->set_identifier_stack(inst->get_dst()->get_reg());
          }
          scasm_inst2->set_dst(std::move(scasm_dst2));

          scasm_func->add_instruction(std::move(scasm_inst2));
        }
      } else if (inst->get_type() == scar::instruction_type::BINARY) {
        if (binop::is_relational(inst->get_binop())) {
          // Cmp(src2, src1)
          // Mov(Imm(0), dst)
          // SetCC(conditional, dst)
          MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
          scasm_inst->set_type(scasm::instruction_type::CMP);
          MAKE_SHARED(scasm::scasm_operand, scasm_src);
          switch (inst->get_src2()->get_type()) {
          case scar::val_type::VAR:
            scasm_src->set_type(scasm::operand_type::PSEUDO);
            scasm_src->set_identifier_stack(inst->get_src2()->get_reg());
            break;
          case scar::val_type::CONSTANT:
            scasm_src->set_type(scasm::operand_type::IMM);
            scasm_src->set_imm(stoi(inst->get_src2()->get_value()));
            break;
          default:
            break;
          }
          scasm_inst->set_src(std::move(scasm_src));
          MAKE_SHARED(scasm::scasm_operand, scasm_dst);
          switch (inst->get_src1()->get_type()) {
          case scar::val_type::VAR:
            scasm_dst->set_type(scasm::operand_type::PSEUDO);
            scasm_dst->set_identifier_stack(inst->get_src1()->get_reg());
            break;
          case scar::val_type::CONSTANT:
            scasm_dst->set_type(scasm::operand_type::IMM);
            scasm_dst->set_imm(stoi(inst->get_src1()->get_value()));
            break;
          default:
            break;
          }
          scasm_inst->set_dst(std::move(scasm_dst));
          scasm_func->add_instruction(std::move(scasm_inst));

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst2);
          scasm_inst2->set_type(scasm::instruction_type::MOV);
          MAKE_SHARED(scasm::scasm_operand, scasm_src2);
          scasm_src2->set_type(scasm::operand_type::IMM);
          scasm_src2->set_imm(0);
          scasm_inst2->set_src(std::move(scasm_src2));
          MAKE_SHARED(scasm::scasm_operand, scasm_dst2);
          SET_DST(scasm_dst2);
          scasm_inst2->set_dst(std::move(scasm_dst2));
          scasm_func->add_instruction(std::move(scasm_inst2));

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst3);
          scasm_inst3->set_type(scasm::instruction_type::SETCC);
          MAKE_SHARED(scasm::scasm_operand, scasm_src3);
          scasm_src3->set_type(scasm::operand_type::COND);
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
          scasm_inst3->set_src(std::move(scasm_src3));
          MAKE_SHARED(scasm::scasm_operand, scasm_dst3);
          SET_DST(scasm_dst3);
          scasm_inst3->set_dst(std::move(scasm_dst3));
          scasm_func->add_instruction(std::move(scasm_inst3));
        } else if (inst->get_binop() == binop::BINOP::DIV or
                   inst->get_binop() == binop::BINOP::MOD) {
          // Mov(src1, Reg(AX))
          // Cdq
          // Idiv(src2)
          // Mov(Reg(AX), dst) | Mov(Reg(DX), dst)

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
          scasm_inst->set_type(scasm::instruction_type::MOV);
          MAKE_SHARED(scasm::scasm_operand, scasm_src);
          SET_MOV_SOURCE();
          MAKE_SHARED(scasm::scasm_operand, scasm_dst);
          scasm_dst->set_type(scasm::operand_type::REG);
          scasm_dst->set_reg(scasm::register_type::AX);
          scasm_inst->set_dst(std::move(scasm_dst));
          scasm_func->add_instruction(std::move(scasm_inst));

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst2);
          scasm_inst2->set_type(scasm::instruction_type::CDQ);
          scasm_func->add_instruction(std::move(scasm_inst2));

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst3);
          scasm_inst3->set_type(scasm::instruction_type::IDIV);
          MAKE_SHARED(scasm::scasm_operand, scasm_src2);
          if (inst->get_src2()->get_type() == scar::val_type::VAR) {
            scasm_src2->set_type(scasm::operand_type::PSEUDO);
            scasm_src2->set_identifier_stack(inst->get_src2()->get_reg());
          } else if (inst->get_src2()->get_type() == scar::val_type::CONSTANT) {
            scasm_src2->set_type(scasm::operand_type::IMM);
            scasm_src2->set_imm(stoi(inst->get_src2()->get_value()));
          }
          scasm_inst3->set_src(std::move(scasm_src2));
          scasm_func->add_instruction(std::move(scasm_inst3));

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst4);
          scasm_inst4->set_type(scasm::instruction_type::MOV);
          MAKE_SHARED(scasm::scasm_operand, scasm_dst2);
          SET_DST(scasm_dst2);
          scasm_inst4->set_dst(std::move(scasm_dst2));
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
                   inst->get_binop() == binop::BINOP::RIGHT_SHIFT) {
          // Mov(src1,dst)
          // Mov(src2, Reg(CX))
          // Binary(binary operand, CL, dst)

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
          scasm_inst->set_type(scasm::instruction_type::MOV);
          MAKE_SHARED(scasm::scasm_operand, scasm_src);
          SET_MOV_SOURCE();
          MAKE_SHARED(scasm::scasm_operand, scasm_dst);
          if (inst->get_dst()->get_type() == scar::val_type::VAR) {
            scasm_dst->set_type(scasm::operand_type::PSEUDO);
            scasm_dst->set_identifier_stack(inst->get_dst()->get_reg());
          }
          scasm_inst->set_dst(std::move(scasm_dst));
          scasm_func->add_instruction(std::move(scasm_inst));

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst2);
          scasm_inst2->set_type(scasm::instruction_type::MOV);
          MAKE_SHARED(scasm::scasm_operand, scasm_src2);
          if (inst->get_src2()->get_type() == scar::val_type::CONSTANT) {
            scasm_src2->set_type(scasm::operand_type::IMM);
            scasm_src2->set_imm(stoi(inst->get_src2()->get_value()));
          } else if (inst->get_src2()->get_type() == scar::val_type::VAR) {
            scasm_src2->set_type(scasm::operand_type::PSEUDO);
            scasm_src2->set_identifier_stack(inst->get_src2()->get_reg());
          }
          scasm_inst2->set_src(std::move(scasm_src2));
          MAKE_SHARED(scasm::scasm_operand, scasm_dst2);
          scasm_dst2->set_type(scasm::operand_type::REG);
          scasm_dst2->set_reg(scasm::register_type::CX);
          scasm_inst2->set_dst(std::move(scasm_dst2));
          scasm_func->add_instruction(std::move(scasm_inst2));

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst3);
          scasm_inst3->set_type(scasm::instruction_type::BINARY);
          scasm_inst3->set_binop(
              scasm::scar_binop_to_scasm_binop(inst->get_binop()));
          MAKE_SHARED(scasm::scasm_operand, scasm_src3);
          scasm_src3->set_type(scasm::operand_type::REG);
          scasm_src3->set_reg(scasm::register_type::CL);
          scasm_inst3->set_src(std::move(scasm_src3));
          MAKE_SHARED(scasm::scasm_operand, scasm_dst3);
          scasm_dst3->set_type(scasm::operand_type::PSEUDO);
          scasm_dst3->set_identifier_stack(inst->get_dst()->get_reg());
          scasm_inst3->set_dst(std::move(scasm_dst3));
          scasm_func->add_instruction(std::move(scasm_inst3));

        } else {
          // Mov(src1, dst)
          // Binary(binary_operator, src2, dst)

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
          scasm_inst->set_type(scasm::instruction_type::MOV);
          MAKE_SHARED(scasm::scasm_operand, scasm_src);
          SET_MOV_SOURCE();
          MAKE_SHARED(scasm::scasm_operand, scasm_dst);
          if (inst->get_dst()->get_type() == scar::val_type::VAR) {
            scasm_dst->set_type(scasm::operand_type::PSEUDO);
            scasm_dst->set_identifier_stack(inst->get_dst()->get_reg());
          }
          scasm_inst->set_dst(std::move(scasm_dst));
          scasm_func->add_instruction(std::move(scasm_inst));

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst2);
          scasm_inst2->set_type(scasm::instruction_type::BINARY);
          scasm_inst2->set_binop(
              scasm::scar_binop_to_scasm_binop(inst->get_binop()));
          MAKE_SHARED(scasm::scasm_operand, scasm_src2);
          if (inst->get_src2()->get_type() == scar::val_type::VAR) {
            scasm_src2->set_type(scasm::operand_type::PSEUDO);
            scasm_src2->set_identifier_stack(inst->get_src2()->get_reg());
          } else if (inst->get_src2()->get_type() == scar::val_type::CONSTANT) {
            scasm_src2->set_type(scasm::operand_type::IMM);
            scasm_src2->set_imm(stoi(inst->get_src2()->get_value()));
          }
          scasm_inst2->set_src(std::move(scasm_src2));
          MAKE_SHARED(scasm::scasm_operand, scasm_dst2);
          if (inst->get_dst()->get_type() == scar::val_type::VAR) {
            scasm_dst2->set_type(scasm::operand_type::PSEUDO);
            scasm_dst2->set_identifier_stack(inst->get_dst()->get_reg());
          }
          scasm_inst2->set_dst(std::move(scasm_dst2));
          scasm_func->add_instruction(std::move(scasm_inst2));
        }
      } else if (inst->get_type() == scar::instruction_type::COPY) {
        MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
        scasm_inst->set_type(scasm::instruction_type::MOV);
        MAKE_SHARED(scasm::scasm_operand, scasm_src);
        SET_MOV_SOURCE();
        MAKE_SHARED(scasm::scasm_operand, scasm_dst);
        SET_DST(scasm_dst);
        scasm_inst->set_dst(std::move(scasm_dst));
        scasm_func->add_instruction(std::move(scasm_inst));
      } else if (inst->get_type() == scar::instruction_type::LABEL) {
        MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
        scasm_inst->set_type(scasm::instruction_type::LABEL);
        MAKE_SHARED(scasm::scasm_operand, scasm_src);
        scasm_src->set_type(scasm::operand_type::LABEL);
        scasm_src->set_identifier_stack(inst->get_src1()->get_value());
        scasm_inst->set_src(std::move(scasm_src));
        scasm_func->add_instruction(std::move(scasm_inst));
      } else if (inst->get_type() == scar::instruction_type::JUMP) {
        MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
        scasm_inst->set_type(scasm::instruction_type::JMP);
        MAKE_SHARED(scasm::scasm_operand, scasm_src);
        scasm_src->set_type(scasm::operand_type::LABEL);
        scasm_src->set_identifier_stack(inst->get_src1()->get_value());
        scasm_inst->set_src(std::move(scasm_src));
        scasm_func->add_instruction(std::move(scasm_inst));
      } else if (inst->get_type() == scar::instruction_type::JUMP_IF_ZERO or
                 inst->get_type() == scar::instruction_type::JUMP_IF_NOT_ZERO) {
        // Cmp(Imm(0), condition)
        // JmpCC(E, label) | JmpCC(NE, label)
        MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
        scasm_inst->set_type(scasm::instruction_type::CMP);
        MAKE_SHARED(scasm::scasm_operand, scasm_src);
        scasm_src->set_type(scasm::operand_type::IMM);
        scasm_src->set_imm(0);
        scasm_inst->set_src(std::move(scasm_src));
        MAKE_SHARED(scasm::scasm_operand, scasm_dst);
        switch (inst->get_src1()->get_type()) {
        case scar::val_type::VAR:
          scasm_dst->set_type(scasm::operand_type::PSEUDO);
          scasm_dst->set_identifier_stack(inst->get_src1()->get_reg());
          break;
        case scar::val_type::CONSTANT:
          scasm_dst->set_type(scasm::operand_type::IMM);
          scasm_dst->set_imm(stoi(inst->get_src1()->get_value()));
          break;
        default:
          break;
        }
        scasm_inst->set_dst(std::move(scasm_dst));
        scasm_func->add_instruction(std::move(scasm_inst));

        MAKE_SHARED(scasm::scasm_instruction, scasm_inst2);
        scasm_inst2->set_type(scasm::instruction_type::JMPCC);
        MAKE_SHARED(scasm::scasm_operand, scasm_src2);
        scasm_src2->set_type(scasm::operand_type::COND);
        if (inst->get_type() == scar::instruction_type::JUMP_IF_ZERO) {
          scasm_src2->set_cond(scasm::cond_code::E);
        } else {
          scasm_src2->set_cond(scasm::cond_code::NE);
        }
        scasm_inst2->set_src(std::move(scasm_src2));
        MAKE_SHARED(scasm::scasm_operand, scasm_dst2);
        scasm_dst2->set_type(scasm::operand_type::LABEL);
        scasm_dst2->set_identifier_stack(inst->get_dst()->get_value());
        scasm_inst2->set_dst(std::move(scasm_dst2));
        scasm_func->add_instruction(std::move(scasm_inst2));
      } else if (inst->get_type() == scar::instruction_type::CALL) {
        auto funcCall =
            std::static_pointer_cast<scar::scar_FunctionCall_Instruction_Node>(
                inst);
        int numArgs = funcCall->get_args().size();
        bool stack_padding =
            numArgs > 6 ? ((numArgs - 6) % 2 ? true : false) : false;
        if (stack_padding) {
          MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
          scasm_inst->set_type(scasm::instruction_type::ALLOCATE_STACK);
          MAKE_SHARED(scasm::scasm_operand, scasm_src);
          scasm_src->set_type(scasm::operand_type::IMM);
          scasm_src->set_imm(8);
          scasm_inst->set_src(std::move(scasm_src));
          scasm_func->add_instruction(std::move(scasm_inst));
        }
        for (int i = 0; i < std::min(6, numArgs); i++) {
          MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
          scasm_inst->set_type(scasm::instruction_type::MOV);
          MAKE_SHARED(scasm::scasm_operand, scasm_src);
          switch (funcCall->get_args()[i]->get_type()) {
          case scar::val_type::VAR:
            scasm_src->set_type(scasm::operand_type::PSEUDO);
            scasm_src->set_identifier_stack(funcCall->get_args()[i]->get_reg());
            break;
          case scar::val_type::CONSTANT:
            scasm_src->set_type(scasm::operand_type::IMM);
            scasm_src->set_imm(stoi(funcCall->get_args()[i]->get_value()));
            break;
          default:
            break;
          }
          scasm_inst->set_src(std::move(scasm_src));
          MAKE_SHARED(scasm::scasm_operand, scasm_dst);
          scasm_dst->set_type(scasm::operand_type::REG);
          scasm_dst->set_reg(argReg[i]);
          scasm_inst->set_dst(std::move(scasm_dst));
          scasm_func->add_instruction(std::move(scasm_inst));
        }
        if (numArgs > 6) {
          for (int i = numArgs - 1; i >= 6; i--) {
            MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
            MAKE_SHARED(scasm::scasm_operand, scasm_src);
            switch (funcCall->get_args()[i]->get_type()) {
            case scar::val_type::VAR:
              scasm_src->set_type(scasm::operand_type::PSEUDO);
              scasm_src->set_identifier_stack(
                  funcCall->get_args()[i]->get_reg());
              break;
            case scar::val_type::CONSTANT:
              scasm_src->set_type(scasm::operand_type::IMM);
              scasm_src->set_imm(stoi(funcCall->get_args()[i]->get_value()));
              break;
            default:
              break;
            }

            if (scasm_src->get_type() == scasm::operand_type::IMM or
                scasm_src->get_type() == scasm::operand_type::REG) {
              scasm_inst->set_src(std::move(scasm_src));
              scasm_inst->set_type(scasm::instruction_type::PUSH);
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
              MAKE_SHARED(scasm::scasm_operand, scasm_src2);
              scasm_src2->set_type(scasm::operand_type::REG);
              scasm_src2->set_reg(scasm::register_type::AX);
              scasm_inst2->set_src(std::move(scasm_src2));
              scasm_func->add_instruction(std::move(scasm_inst2));
            }
          }
        }

        MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
        scasm_inst->set_type(scasm::instruction_type::CALL);
        MAKE_SHARED(scasm::scasm_operand, scasm_src);
        scasm_src->set_type(scasm::operand_type::LABEL);
        scasm_src->set_identifier_stack(funcCall->get_name()->get_value());
        scasm_inst->set_src(std::move(scasm_src));
        scasm_func->add_instruction(std::move(scasm_inst));

        int bytesToRemove = numArgs > 6 ? 8 * (numArgs - 6) : 0;
        bytesToRemove += stack_padding ? 8 : 0;

        if (bytesToRemove != 0) {
          MAKE_SHARED(scasm::scasm_instruction, scasm_inst2);
          scasm_inst2->set_type(scasm::instruction_type::DEALLOCATE_STACK);
          MAKE_SHARED(scasm::scasm_operand, scasm_src2);
          scasm_src2->set_type(scasm::operand_type::IMM);
          scasm_src2->set_imm(bytesToRemove);
          scasm_inst2->set_src(std::move(scasm_src2));
          scasm_func->add_instruction(std::move(scasm_inst2));
        }

        MAKE_SHARED(scasm::scasm_instruction, scasm_inst3);
        scasm_inst3->set_type(scasm::instruction_type::MOV);
        MAKE_SHARED(scasm::scasm_operand, scasm_src3);
        scasm_src3->set_type(scasm::operand_type::REG);
        scasm_src3->set_reg(scasm::register_type::AX);
        scasm_inst3->set_src(std::move(scasm_src3));
        MAKE_SHARED(scasm::scasm_operand, scasm_dst3);
        SET_DST(scasm_dst3);
        scasm_inst3->set_dst(std::move(scasm_dst3));
        scasm_func->add_instruction(std::move(scasm_inst3));
      }
    }
    scasm_program.add_function(std::move(scasm_func));
  }

  this->scasm = scasm_program;
}

} // namespace codegen
} // namespace scarlet
