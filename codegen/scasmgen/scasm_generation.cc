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
    scasm_src->set_imm(inst->get_src1()->get_const_val());                     \
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
    if (elem->get_type() == scar::topLevelType::STATICVARIABLE) {
      auto var = std::static_pointer_cast<scar::scar_StaticVariable_Node>(elem);
      MAKE_SHARED(scasm::scasm_static_variable, scasm_var);
      scasm_var->set_name(var->get_identifier()->get_value());
      switch (var->get_init().get_type()) {
      case constant::Type::UINT:
      case constant::Type::INT:
        scasm_var->set_alignment(4);
        break;
      case constant::Type::ULONG:
      case constant::Type::LONG:
        scasm_var->set_alignment(8);
        break;
      case constant::Type::DOUBLE:
        scasm_var->set_alignment(8);
        break;
      case constant::Type::NONE:
        scasm_var->set_alignment(0);
        break;
      default:
        break;
      }
      scasm_var->set_init(var->get_init());
      scasm_var->set_global(elem->is_global());
      MAKE_SHARED(scasm::scasm_top_level, top_level_elem);
      top_level_elem =
          std::static_pointer_cast<scasm::scasm_top_level>(scasm_var);
      top_level_elem->set_type(scasm::scasm_top_level_type::STATIC_VARIABLE);
      scasm_program.add_elem(std::move(top_level_elem));
      continue;
    }
    auto func = std::static_pointer_cast<scar::scar_Function_Node>(elem);
    MAKE_SHARED(scasm::scasm_function, scasm_func);
    scasm_func->set_name(func->get_identifier()->get_value());
    scasm_func->set_global(elem->is_global());

    // Move function args from registers and stack to the callee stack frame
    int numParams = func->get_params().size();
    for (int i = 0; i < std::min(6, numParams); i++) {
      MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
      scasm_inst->set_type(scasm::instruction_type::MOV);
      scasm_inst->set_asm_type(
          elemToAsmType(globalSymbolTable[func->get_identifier()->get_value()]
                            .typeDef[i + 1]));
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
        scasm_inst->set_asm_type(
            elemToAsmType(globalSymbolTable[func->get_identifier()->get_value()]
                              .typeDef[i + 1]));
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
        scasm_inst->set_asm_type(valToAsmType(inst->get_src1()));
        MAKE_SHARED(scasm::scasm_operand, scasm_src);
        SET_MOV_SOURCE();
        /*
        START
        */
        switch (inst->get_src1()->get_type()) {
        case scar::val_type::VAR: {
          scasm_src->set_type(scasm::operand_type::PSEUDO);
          scasm_src->set_identifier_stack(inst->get_src1()->get_reg());
        } break;
        case scar::val_type::CONSTANT: {
          if (inst->get_src1()->get_const_val().get_type() ==
              constant::Type::DOUBLE) {
            MAKE_SHARED(scasm::scasm_static_constant, doubleConst);
            doubleConst->set_type(scasm::scasm_top_level_type::STATIC_CONSTANT);
          } else {
            scasm_src->set_type(scasm::operand_type::IMM);
            scasm_src->set_imm(inst->get_src1()->get_const_val());
          }
        } break;
        default:
          break;
        }
        scasm_inst->set_src(std::move(scasm_src))
            /*
            END
            */
            MAKE_SHARED(scasm::scasm_operand, scasm_dst);
        scasm_dst->set_type(scasm::operand_type::REG);
        scasm_dst->set_reg(scasm::register_type::AX);
        scasm_inst->set_dst(std::move(scasm_dst));
        scasm_func->add_instruction(std::move(scasm_inst));

        MAKE_SHARED(scasm::scasm_instruction, scasm_inst2);
        scasm_inst2->set_type(scasm::instruction_type::RET);
        scasm_func->add_instruction(std::move(scasm_inst2));

      } else if (inst->get_type() == scar::instruction_type::UNARY) {
        scasm::AssemblyType instType = valToAsmType(inst->get_src1());

        if (inst->get_unop() == unop::UNOP::NOT) {
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
          SET_DST(scasm_dst2);
          scasm_inst2->set_dst(std::move(scasm_dst2));
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
          SET_MOV_SOURCE();

          MAKE_SHARED(scasm::scasm_operand, scasm_dst);
          SET_DST(scasm_dst);

          scasm_inst->set_dst(scasm_dst);

          scasm_func->add_instruction(std::move(scasm_inst));

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst2);
          scasm_inst2->set_type(scasm::instruction_type::UNARY);
          scasm_inst2->set_asm_type(instType);
          scasm_inst2->set_unop(inst->get_unop());

          scasm_inst2->set_dst(std::move(scasm_dst));

          scasm_func->add_instruction(std::move(scasm_inst2));
        }
      } else if (inst->get_type() == scar::instruction_type::BINARY) {
        scasm::AssemblyType instType = valToAsmType(inst->get_src1());

        if (binop::is_relational(inst->get_binop())) {
          // Cmp(src2, src1)
          // Mov(Imm(0), dst)
          // SetCC(conditional, dst)
          MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
          scasm_inst->set_type(scasm::instruction_type::CMP);
          scasm_inst->set_asm_type(instType);
          MAKE_SHARED(scasm::scasm_operand, scasm_src);
          switch (inst->get_src2()->get_type()) {
          case scar::val_type::VAR:
            scasm_src->set_type(scasm::operand_type::PSEUDO);
            scasm_src->set_identifier_stack(inst->get_src2()->get_reg());
            break;
          case scar::val_type::CONSTANT:
            scasm_src->set_type(scasm::operand_type::IMM);
            scasm_src->set_imm(inst->get_src2()->get_const_val());
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
          constant::Constant zero;
          zero.set_type(constant::Type::INT);
          zero.set_value({.i = 0});
          scasm_src2->set_imm(zero);
          scasm_inst2->set_src(std::move(scasm_src2));
          MAKE_SHARED(scasm::scasm_operand, scasm_dst2);
          SET_DST(scasm_dst2);
          scasm_inst2->set_dst(std::move(scasm_dst2));
          scasm_func->add_instruction(std::move(scasm_inst2));

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst3);
          scasm_inst3->set_type(scasm::instruction_type::SETCC);
          scasm_inst3->set_asm_type(scasm::AssemblyType::BYTE);
          MAKE_SHARED(scasm::scasm_operand, scasm_src3);
          scasm_src3->set_type(scasm::operand_type::COND);
          if (valType(inst->get_src1()) == constant::Type::UINT or
              valType(inst->get_src1()) == constant::Type::ULONG) {
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
          SET_DST(scasm_dst3);
          scasm_inst3->set_dst(std::move(scasm_dst3));
          scasm_func->add_instruction(std::move(scasm_inst3));
        } else if (inst->get_binop() == binop::BINOP::DIV or
                   inst->get_binop() == binop::BINOP::MOD) {
          // Mov(<src1 type>, src1, Reg(AX))
          // unsigned: Mov(<src1 type>, Imm(0), Reg(DX))| signed: Cdq
          // unsigned: Div(<src1 type>, src2)|signed: Idiv(<src1 type>, src2)
          // Mov(<src1 type>, Reg(AX), dst)|Mov(<src1 type>, Reg(DX), dst)
          MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
          scasm_inst->set_type(scasm::instruction_type::MOV);
          scasm_inst->set_asm_type(instType);
          MAKE_SHARED(scasm::scasm_operand, scasm_src);
          SET_MOV_SOURCE();
          MAKE_SHARED(scasm::scasm_operand, scasm_dst);
          scasm_dst->set_type(scasm::operand_type::REG);
          scasm_dst->set_reg(scasm::register_type::AX);
          scasm_inst->set_dst(std::move(scasm_dst));
          scasm_func->add_instruction(std::move(scasm_inst));

          if (valType(inst->get_src1()) == constant::Type::UINT or
              valType(inst->get_src1()) == constant::Type::ULONG) {
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
          if (valType(inst->get_src1()) == constant::Type::UINT or
              valType(inst->get_src1()) == constant::Type::ULONG) {
            scasm_inst3->set_type(scasm::instruction_type::DIV);
          } else {
            scasm_inst3->set_type(scasm::instruction_type::IDIV);
          }
          scasm_inst3->set_asm_type(instType);
          MAKE_SHARED(scasm::scasm_operand, scasm_src2);
          if (inst->get_src2()->get_type() == scar::val_type::VAR) {
            scasm_src2->set_type(scasm::operand_type::PSEUDO);
            scasm_src2->set_identifier_stack(inst->get_src2()->get_reg());
          } else if (inst->get_src2()->get_type() == scar::val_type::CONSTANT) {
            scasm_src2->set_type(scasm::operand_type::IMM);
            scasm_src2->set_imm(inst->get_src2()->get_const_val());
          }
          scasm_inst3->set_src(std::move(scasm_src2));
          scasm_func->add_instruction(std::move(scasm_inst3));

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst4);
          scasm_inst4->set_type(scasm::instruction_type::MOV);
          scasm_inst4->set_asm_type(instType);
          MAKE_SHARED(scasm::scasm_operand, scasm_dst3);
          SET_DST(scasm_dst3);
          scasm_inst4->set_dst(std::move(scasm_dst3));
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
          scasm_inst2->set_asm_type(instType);
          MAKE_SHARED(scasm::scasm_operand, scasm_src2);
          if (inst->get_src2()->get_type() == scar::val_type::CONSTANT) {
            scasm_src2->set_type(scasm::operand_type::IMM);
            scasm_src2->set_imm(inst->get_src2()->get_const_val());
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
          scasm_inst3->set_asm_type(instType);
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
          scasm_inst->set_asm_type(instType);
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
          scasm_inst2->set_asm_type(instType);
          scasm_inst2->set_binop(
              scasm::scar_binop_to_scasm_binop(inst->get_binop()));
          MAKE_SHARED(scasm::scasm_operand, scasm_src2);
          if (inst->get_src2()->get_type() == scar::val_type::VAR) {
            scasm_src2->set_type(scasm::operand_type::PSEUDO);
            scasm_src2->set_identifier_stack(inst->get_src2()->get_reg());
          } else if (inst->get_src2()->get_type() == scar::val_type::CONSTANT) {
            scasm_src2->set_type(scasm::operand_type::IMM);
            scasm_src2->set_imm(inst->get_src2()->get_const_val());
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
        scasm_inst->set_asm_type(valToAsmType(inst->get_src1()));
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
        scasm_src->set_identifier_stack(inst->get_src1()->get_label());
        scasm_inst->set_src(std::move(scasm_src));
        scasm_func->add_instruction(std::move(scasm_inst));
      } else if (inst->get_type() == scar::instruction_type::JUMP) {
        MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
        scasm_inst->set_type(scasm::instruction_type::JMP);
        MAKE_SHARED(scasm::scasm_operand, scasm_src);
        scasm_src->set_type(scasm::operand_type::LABEL);
        scasm_src->set_identifier_stack(inst->get_src1()->get_label());
        scasm_inst->set_src(std::move(scasm_src));
        scasm_func->add_instruction(std::move(scasm_inst));
      } else if (inst->get_type() == scar::instruction_type::JUMP_IF_ZERO or
                 inst->get_type() == scar::instruction_type::JUMP_IF_NOT_ZERO) {
        // Cmp(Imm(0), condition)
        // JmpCC(E, label) | JmpCC(NE, label)
        MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
        scasm_inst->set_type(scasm::instruction_type::CMP);
        scasm_inst->set_asm_type(valToAsmType(inst->get_src1()));
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
        scasm_dst2->set_identifier_stack(inst->get_dst()->get_label());
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
        for (int i = 0; i < std::min(6, numArgs); i++) {
          MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
          scasm_inst->set_type(scasm::instruction_type::MOV);
          scasm_inst->set_asm_type(valToAsmType(funcCall->get_args()[i]));
          MAKE_SHARED(scasm::scasm_operand, scasm_src);
          switch (funcCall->get_args()[i]->get_type()) {
          case scar::val_type::VAR:
            scasm_src->set_type(scasm::operand_type::PSEUDO);
            scasm_src->set_identifier_stack(funcCall->get_args()[i]->get_reg());
            break;
          case scar::val_type::CONSTANT:
            scasm_src->set_type(scasm::operand_type::IMM);
            scasm_src->set_imm(funcCall->get_args()[i]->get_const_val());
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
            scasm_inst->set_asm_type(valToAsmType(funcCall->get_args()[i]));
            switch (funcCall->get_args()[i]->get_type()) {
            case scar::val_type::VAR:
              scasm_src->set_type(scasm::operand_type::PSEUDO);
              scasm_src->set_identifier_stack(
                  funcCall->get_args()[i]->get_reg());
              break;
            case scar::val_type::CONSTANT:
              scasm_src->set_type(scasm::operand_type::IMM);
              scasm_src->set_imm(funcCall->get_args()[i]->get_const_val());
              break;
            default:
              break;
            }

            if (scasm_src->get_type() == scasm::operand_type::IMM or
                scasm_inst->get_asm_type() == scasm::AssemblyType::QUAD_WORD) {
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
        scasm_src3->set_reg(scasm::register_type::AX);
        scasm_inst3->set_src(std::move(scasm_src3));
        MAKE_SHARED(scasm::scasm_operand, scasm_dst3);
        SET_DST(scasm_dst3);
        scasm_inst3->set_dst(std::move(scasm_dst3));
        scasm_func->add_instruction(std::move(scasm_inst3));
      } else if (inst->get_type() == scar::instruction_type::SIGN_EXTEND) {
        MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
        scasm_inst->set_type(scasm::instruction_type::MOVSX);
        scasm_inst->set_asm_type(scasm::AssemblyType::LONG_WORD);
        MAKE_SHARED(scasm::scasm_operand, scasm_src);
        SET_MOV_SOURCE();
        MAKE_SHARED(scasm::scasm_operand, scasm_dst);
        SET_DST(scasm_dst);
        scasm_inst->set_dst(std::move(scasm_dst));
        scasm_func->add_instruction(std::move(scasm_inst));
      } else if (inst->get_type() == scar::instruction_type::TRUNCATE) {
        MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
        scasm_inst->set_type(scasm::instruction_type::MOV);
        scasm_inst->set_asm_type(scasm::AssemblyType::LONG_WORD);
        MAKE_SHARED(scasm::scasm_operand, scasm_src);
        SET_MOV_SOURCE();
        MAKE_SHARED(scasm::scasm_operand, scasm_dst);
        SET_DST(scasm_dst);
        scasm_inst->set_dst(std::move(scasm_dst));
        scasm_func->add_instruction(std::move(scasm_inst));
      } else if (inst->get_type() == scar::instruction_type::ZERO_EXTEND) {
        MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
        scasm_inst->set_type(scasm::instruction_type::MOVZX);
        scasm_inst->set_asm_type(scasm::AssemblyType::LONG_WORD);
        MAKE_SHARED(scasm::scasm_operand, scasm_src);
        SET_MOV_SOURCE();
        MAKE_SHARED(scasm::scasm_operand, scasm_dst);
        SET_DST(scasm_dst);
        scasm_inst->set_dst(std::move(scasm_dst));
        scasm_func->add_instruction(std::move(scasm_inst));
      }
    }
    MAKE_SHARED(scasm::scasm_top_level, top_level_elem);
    top_level_elem =
        std::static_pointer_cast<scasm::scasm_top_level>(scasm_func);
    top_level_elem->set_type(scasm::scasm_top_level_type::FUNCTION);
    scasm_program.add_elem(std::move(top_level_elem));
  }

  // Make the backend symbol table
  for (auto it : globalSymbolTable) {
    if (it.second.type == symbolTable::symbolType::VARIABLE) {
      scasm::backendSymbol sym;
      sym.type = scasm::backendSymbolType::STATIC_VARIABLE;
      sym.asmType = elemToAsmType(it.second.typeDef[0]);
      if (it.second.link != symbolTable::linkage::NONE) {
        sym.isTopLevel = true;
      } else {
        sym.isTopLevel = false;
      }
      backendSymbolTable[it.first] = sym;
    } else {
      scasm::backendSymbol sym;
      sym.type = scasm::backendSymbolType::FUNCTION;
      if (globalSymbolTable[it.first].def == symbolTable::defType::TRUE) {
        sym.isDefined = true;
      } else {
        sym.isDefined = false;
      }
      backendSymbolTable[it.first] = sym;
    }
  }
  this->scasm = scasm_program;
}

} // namespace codegen
} // namespace scarlet
