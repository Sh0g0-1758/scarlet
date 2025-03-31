#include <codegen/common.hh>

namespace scarlet {
namespace codegen {

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
    std::vector<constant::Type> param_types;
    for (int i = 0; i < numParams; i++) {
      param_types.push_back(ast::elemTypeToConstType(
          globalSymbolTable[func->get_identifier()->get_value()]
              .typeDef[i + 1]));
    }

    std::vector<std::pair<scasm::AssemblyType, int>> int_param_indx;
    std::vector<int> double_param_indx;
    std::vector<std::pair<scasm::AssemblyType, int>> stack_param_indx;
    calssify_parameters(param_types, int_param_indx, double_param_indx,
                        stack_param_indx);

    // Move int args
    for (int i = 0; i < (int)int_param_indx.size(); i++) {
      MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
      scasm_inst->set_type(scasm::instruction_type::MOV);
      scasm_inst->set_asm_type(int_param_indx[i].first);
      MAKE_SHARED(scasm::scasm_operand, scasm_src);
      scasm_src->set_type(scasm::operand_type::REG);
      scasm_src->set_reg(int_argReg[i]);
      scasm_inst->set_src(std::move(scasm_src));
      MAKE_SHARED(scasm::scasm_operand, scasm_dst);
      scasm_dst->set_type(scasm::operand_type::PSEUDO);
      scasm_dst->set_identifier_stack(
          func->get_params()[int_param_indx[i].second]->get_value());
      scasm_inst->set_dst(std::move(scasm_dst));
      scasm_func->add_instruction(std::move(scasm_inst));
    }

    // Move double args
    for (int i = 0; i < (int)double_param_indx.size(); i++) {
      MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
      scasm_inst->set_type(scasm::instruction_type::MOV);
      scasm_inst->set_asm_type(scasm::AssemblyType::DOUBLE);
      MAKE_SHARED(scasm::scasm_operand, scasm_src);
      scasm_src->set_type(scasm::operand_type::REG);
      scasm_src->set_reg(double_argReg[i]);
      scasm_inst->set_src(std::move(scasm_src));
      MAKE_SHARED(scasm::scasm_operand, scasm_dst);
      scasm_dst->set_type(scasm::operand_type::PSEUDO);
      scasm_dst->set_identifier_stack(
          func->get_params()[double_param_indx[i]]->get_value());
      scasm_inst->set_dst(std::move(scasm_dst));
      scasm_func->add_instruction(std::move(scasm_inst));
    }

    // Move stack args
    for (int i = stack_param_indx.size() - 1; i >= 0; i--) {
      MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
      scasm_inst->set_type(scasm::instruction_type::MOV);
      scasm_inst->set_asm_type(stack_param_indx[i].first);
      MAKE_SHARED(scasm::scasm_operand, scasm_src);
      scasm_src->set_type(scasm::operand_type::STACK);
      scasm_src->set_identifier_stack(std::to_string(16 + 8 * i) + "(%rbp)");
      scasm_inst->set_src(std::move(scasm_src));
      MAKE_SHARED(scasm::scasm_operand, scasm_dst);
      scasm_dst->set_type(scasm::operand_type::PSEUDO);
      scasm_dst->set_identifier_stack(
          func->get_params()[stack_param_indx[i].second]->get_value());
      scasm_inst->set_dst(std::move(scasm_dst));
      scasm_func->add_instruction(std::move(scasm_inst));
    }

    for (auto inst : func->get_instructions()) {      
      scasm::AssemblyType instType = valToAsmType(inst->get_src1());
      if (inst->get_type() == scar::instruction_type::RETURN) {
        MAKE_SHARED(scasm::scasm_instruction, scasm_inst);

        scasm_inst->set_type(scasm::instruction_type::MOV);
        scasm_inst->set_asm_type(instType);

        MAKE_SHARED(scasm::scasm_operand, scasm_src);
        SET_OPERAND(scasm_src, set_src, get_src1, scasm_inst);

        MAKE_SHARED(scasm::scasm_operand, scasm_dst);
        scasm_dst->set_type(scasm::operand_type::REG);
        if (instType == scasm::AssemblyType::DOUBLE) {
          scasm_dst->set_reg(scasm::register_type::XMM0);
        } else {
          scasm_dst->set_reg(scasm::register_type::AX);
        }

        scasm_inst->set_dst(std::move(scasm_dst));
        scasm_func->add_instruction(std::move(scasm_inst));

        MAKE_SHARED(scasm::scasm_instruction, scasm_inst2);
        scasm_inst2->set_type(scasm::instruction_type::RET);
        scasm_func->add_instruction(std::move(scasm_inst2));
      } else if (inst->get_type() == scar::instruction_type::UNARY) {
        gen_unop_scasm(inst, scasm_func, scasm_program);
      } else if (inst->get_type() == scar::instruction_type::BINARY) {
        gen_binop_scasm(inst, scasm_func, scasm_program);
      } else if (inst->get_type() == scar::instruction_type::COPY) {
        MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
        scasm_inst->set_type(scasm::instruction_type::MOV);
        scasm_inst->set_asm_type(instType);
        MAKE_SHARED(scasm::scasm_operand, scasm_src);
        SET_OPERAND(scasm_src, set_src, get_src1, scasm_inst);
        MAKE_SHARED(scasm::scasm_operand, scasm_dst);
        SET_OPERAND(scasm_dst, set_dst, get_dst, scasm_inst);
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
        // IF DOUBLE
        //  Binary(Xor, Reg(<X>), Reg(<X>))
        //  Cmp(src, Reg(<X>))
        //  JmpCC(E, label) | JmpCC(NE, label)
        // ELSE
        //  Cmp(Imm(0), src)
        //  JmpCC(E, label) | JmpCC(NE, label)

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
        gen_funcall_scasm(inst, scasm_func, scasm_program);
      } else if (inst->get_type() == scar::instruction_type::SIGN_EXTEND) {
        MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
        scasm_inst->set_type(scasm::instruction_type::MOVSX);
        scasm_inst->set_asm_type(scasm::AssemblyType::LONG_WORD);
        MAKE_SHARED(scasm::scasm_operand, scasm_src);
        SET_OPERAND(scasm_src, set_src, get_src1, scasm_inst);
        MAKE_SHARED(scasm::scasm_operand, scasm_dst);
        SET_OPERAND(scasm_dst, set_dst, get_dst, scasm_inst);
        scasm_func->add_instruction(std::move(scasm_inst));
      } else if (inst->get_type() == scar::instruction_type::TRUNCATE) {
        MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
        scasm_inst->set_type(scasm::instruction_type::MOV);
        scasm_inst->set_asm_type(scasm::AssemblyType::LONG_WORD);
        MAKE_SHARED(scasm::scasm_operand, scasm_src);
        SET_OPERAND(scasm_src, set_src, get_src1, scasm_inst);
        MAKE_SHARED(scasm::scasm_operand, scasm_dst);
        SET_OPERAND(scasm_dst, set_dst, get_dst, scasm_inst);
        scasm_func->add_instruction(std::move(scasm_inst));
      } else if (inst->get_type() == scar::instruction_type::ZERO_EXTEND) {
        MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
        scasm_inst->set_type(scasm::instruction_type::MOVZX);
        scasm_inst->set_asm_type(scasm::AssemblyType::LONG_WORD);
        MAKE_SHARED(scasm::scasm_operand, scasm_src);
        SET_OPERAND(scasm_src, set_src, get_src1, scasm_inst);
        MAKE_SHARED(scasm::scasm_operand, scasm_dst);
        SET_OPERAND(scasm_dst, set_dst, get_dst, scasm_inst);
        scasm_func->add_instruction(std::move(scasm_inst));
      } else if (inst->get_type() == scar::instruction_type::DOUBLE_TO_INT) {
        MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
        scasm_inst->set_type(scasm::instruction_type::CVTTS2DI);
        scasm_inst->set_asm_type(valToAsmType(inst->get_dst()));
        MAKE_SHARED(scasm::scasm_operand, scasm_src);
        SET_OPERAND(scasm_src, set_src, get_src1, scasm_inst);
        MAKE_SHARED(scasm::scasm_operand, scasm_dst);
        SET_OPERAND(scasm_dst, set_dst, get_dst, scasm_inst);
        scasm_func->add_instruction(std::move(scasm_inst));
      } else if (inst->get_type() == scar::instruction_type::INT_TO_DOUBLE) {
        MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
        scasm_inst->set_type(scasm::instruction_type::CVTSI2SD);
        scasm_inst->set_asm_type(instType);
        MAKE_SHARED(scasm::scasm_operand, scasm_src);
        SET_OPERAND(scasm_src, set_src, get_src1, scasm_inst);
        MAKE_SHARED(scasm::scasm_operand, scasm_dst);
        SET_OPERAND(scasm_dst, set_dst, get_dst, scasm_inst);
        scasm_func->add_instruction(std::move(scasm_inst));
      } else if (inst->get_type() == scar::instruction_type::UINT_TO_DOUBLE) {
        if (instType == scasm::AssemblyType::LONG_WORD) {
          // MovZeroExtend(src, Reg(<R>))
          // Cvtsi2sd(Quadword, Reg(<R>), dst)

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
          scasm_inst->set_type(scasm::instruction_type::MOVZX);
          scasm_inst->set_asm_type(instType);
          MAKE_SHARED(scasm::scasm_operand, scasm_src);
          SET_OPERAND(scasm_src, set_src, get_src1, scasm_inst);
          MAKE_SHARED(scasm::scasm_operand, scasm_reg);
          scasm_reg->set_type(scasm::operand_type::REG);
          scasm_reg->set_reg(scasm::register_type::AX);
          scasm_inst->set_dst(scasm_reg);
          scasm_func->add_instruction(std::move(scasm_inst));

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst2);
          scasm_inst2->set_type(scasm::instruction_type::CVTSI2SD);
          scasm_inst2->set_asm_type(scasm::AssemblyType::QUAD_WORD);
          scasm_inst2->set_src(scasm_reg);
          MAKE_SHARED(scasm::scasm_operand, scasm_dst);
          SET_OPERAND(scasm_dst, set_dst, get_dst, scasm_inst2);
          scasm_inst2->set_dst(std::move(scasm_dst));
          scasm_func->add_instruction(std::move(scasm_inst2));
        } else {
          // Cmp(Quadword, Imm(0), src)
          // JmpCC(L, <label1>)
          // Cvtsi2sd(Quadword, src, dst)
          // Jmp(<label2>)
          // Label(<label1>)
          // Mov(Quadword, src, Reg(<R1>))
          // Mov(Quadword, Reg(<R1>), Reg(<R2>))
          // Unary(Shr, Quadword, Reg(<R2>))
          // Binary(And, Quadword, Imm(1), Reg(<R1>))
          // Binary(Or, Quadword, Reg(<R1>), Reg(<R2>))
          // Cvtsi2sd(Quadword, Reg(<R2>), dst)
          // Binary(Add, Double, dst, dst)
          // Label(<label2>)

          /* SETUP */
          std::string label1 = "UL2D." + std::to_string(doubleCastCounter++);
          std::string label2 = "UL2D." + std::to_string(doubleCastCounter++);

          MAKE_SHARED(scasm::scasm_operand, scasm_reg1);
          scasm_reg1->set_type(scasm::operand_type::REG);
          scasm_reg1->set_reg(scasm::register_type::AX);

          MAKE_SHARED(scasm::scasm_operand, scasm_reg2);
          scasm_reg2->set_type(scasm::operand_type::REG);
          scasm_reg2->set_reg(scasm::register_type::DX);

          MAKE_SHARED(scasm::scasm_operand, scasm_src);
          SETVAL_OPERAND(scasm_src, get_src1);

          MAKE_SHARED(scasm::scasm_operand, scasm_dst);
          SETVAL_OPERAND(scasm_dst, get_dst);

          MAKE_SHARED(scasm::scasm_operand, scasm_label1);
          scasm_label1->set_type(scasm::operand_type::LABEL);
          scasm_label1->set_identifier_stack(label1);

          MAKE_SHARED(scasm::scasm_operand, scasm_label2);
          scasm_label2->set_type(scasm::operand_type::LABEL);
          scasm_label2->set_identifier_stack(label2);

          /* SCASM GEN */
          MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
          scasm_inst->set_type(scasm::instruction_type::CMP);
          scasm_inst->set_asm_type(instType);
          MAKE_SHARED(scasm::scasm_operand, scasm_zero);
          scasm_zero->set_type(scasm::operand_type::IMM);
          constant::Constant zero;
          zero.set_type(constant::Type::INT);
          zero.set_value({.i = 0});
          scasm_zero->set_imm(zero);
          scasm_inst->set_src(std::move(scasm_zero));
          scasm_inst->set_dst(scasm_src);
          scasm_func->add_instruction(std::move(scasm_inst));

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst2);
          scasm_inst2->set_type(scasm::instruction_type::JMPCC);
          MAKE_SHARED(scasm::scasm_operand, scasm_src2);
          scasm_src2->set_type(scasm::operand_type::COND);
          scasm_src2->set_cond(scasm::cond_code::L);
          scasm_inst2->set_src(std::move(scasm_src2));
          scasm_inst2->set_dst(scasm_label1);
          scasm_func->add_instruction(std::move(scasm_inst2));

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst3);
          scasm_inst3->set_type(scasm::instruction_type::CVTSI2SD);
          scasm_inst3->set_asm_type(instType);
          scasm_inst3->set_src(scasm_src);
          scasm_inst3->set_dst(scasm_dst);
          scasm_func->add_instruction(std::move(scasm_inst3));

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst4);
          scasm_inst4->set_type(scasm::instruction_type::JMP);
          scasm_inst4->set_src(scasm_label2);
          scasm_func->add_instruction(std::move(scasm_inst4));

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst5);
          scasm_inst5->set_type(scasm::instruction_type::LABEL);
          scasm_inst5->set_src(scasm_label1);
          scasm_func->add_instruction(std::move(scasm_inst5));

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst6);
          scasm_inst6->set_type(scasm::instruction_type::MOV);
          scasm_inst6->set_asm_type(instType);
          scasm_inst6->set_src(scasm_src);
          scasm_inst6->set_dst(scasm_reg1);
          scasm_func->add_instruction(std::move(scasm_inst6));

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst7);
          scasm_inst7->set_type(scasm::instruction_type::MOV);
          scasm_inst7->set_asm_type(instType);
          scasm_inst7->set_src(scasm_reg1);
          scasm_inst7->set_dst(scasm_reg2);
          scasm_func->add_instruction(std::move(scasm_inst7));

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst8);
          scasm_inst8->set_type(scasm::instruction_type::UNARY);
          scasm_inst8->set_asm_type(instType);
          scasm_inst8->set_unop(scasm::Unop::SHR);
          scasm_inst8->set_dst(scasm_reg2);
          scasm_func->add_instruction(std::move(scasm_inst8));

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst9);
          scasm_inst9->set_type(scasm::instruction_type::BINARY);
          scasm_inst9->set_asm_type(instType);
          scasm_inst9->set_binop(scasm::Binop::AAND);
          MAKE_SHARED(scasm::scasm_operand, scasm_src9);
          scasm_src9->set_type(scasm::operand_type::IMM);
          constant::Constant one;
          one.set_type(constant::Type::INT);
          one.set_value({.i = 1});
          scasm_src9->set_imm(one);
          scasm_inst9->set_src(std::move(scasm_src9));
          scasm_inst9->set_dst(scasm_reg1);
          scasm_func->add_instruction(std::move(scasm_inst9));

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst10);
          scasm_inst10->set_type(scasm::instruction_type::BINARY);
          scasm_inst10->set_asm_type(instType);
          scasm_inst10->set_binop(scasm::Binop::AOR);
          scasm_inst10->set_src(scasm_reg1);
          scasm_inst10->set_dst(scasm_reg2);
          scasm_func->add_instruction(std::move(scasm_inst10));

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst11);
          scasm_inst11->set_type(scasm::instruction_type::CVTSI2SD);
          scasm_inst11->set_asm_type(instType);
          scasm_inst11->set_src(scasm_reg2);
          scasm_inst11->set_dst(scasm_dst);
          scasm_func->add_instruction(std::move(scasm_inst11));

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst12);
          scasm_inst12->set_type(scasm::instruction_type::BINARY);
          scasm_inst12->set_asm_type(scasm::AssemblyType::DOUBLE);
          scasm_inst12->set_binop(scasm::Binop::ADD);
          scasm_inst12->set_src(scasm_dst);
          scasm_inst12->set_dst(scasm_dst);
          scasm_func->add_instruction(std::move(scasm_inst12));

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst13);
          scasm_inst13->set_type(scasm::instruction_type::LABEL);
          scasm_inst13->set_src(scasm_label2);
          scasm_func->add_instruction(std::move(scasm_inst13));
        }
      } else if (inst->get_type() == scar::instruction_type::DOUBLE_TO_UINT) {
        instType = valToAsmType(inst->get_dst());
        if (instType == scasm::AssemblyType::LONG_WORD) {
          // Cvttsd2si(Quadword, src, Reg(<R>))
          // Mov(Longword, Reg(<R>), dst)

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
          scasm_inst->set_type(scasm::instruction_type::CVTTS2DI);
          scasm_inst->set_asm_type(scasm::AssemblyType::QUAD_WORD);
          MAKE_SHARED(scasm::scasm_operand, scasm_src);
          SET_OPERAND(scasm_src, set_src, get_src1, scasm_inst);
          MAKE_SHARED(scasm::scasm_operand, scasm_reg);
          scasm_reg->set_type(scasm::operand_type::REG);
          scasm_reg->set_reg(scasm::register_type::AX);
          scasm_inst->set_dst(scasm_reg);
          scasm_func->add_instruction(std::move(scasm_inst));

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst2);
          scasm_inst2->set_type(scasm::instruction_type::MOV);
          scasm_inst2->set_asm_type(instType);
          scasm_inst2->set_src(std::move(scasm_reg));
          MAKE_SHARED(scasm::scasm_operand, scasm_dst);
          SET_OPERAND(scasm_dst, set_dst, get_dst, scasm_inst2);
          scasm_inst2->set_dst(std::move(scasm_dst));
          scasm_func->add_instruction(std::move(scasm_inst2));
        } else {
          // StaticConstant(<upper-bound>, 8, DoubleInit(9223372036854775808.0))
          //
          // Cmp(Double, Data(<upper-bound>), src)
          // JmpCC(AE, <label1>)
          // Cvttsd2si(Quadword, src, dst)
          // Jmp(<label2>)
          // Label(<label1>)
          // Mov(Double, src, Reg(<X>))
          // Binary(Sub, Double, Data(<upper-bound>), Reg(<X>))
          // Cvttsd2si(Quadword, Reg(<X>), dst)
          // Mov(Quadword, Imm(9223372036854775808), Reg(<R>))
          // Binary(Add, Quadword, Reg(<R>), dst)
          // Label(<label2>)

          /* SETUP */
          std::string label1 = "D2UL." + std::to_string(doubleCastCounter++);
          std::string label2 = "D2UL." + std::to_string(doubleCastCounter++);

          MAKE_SHARED(scasm::scasm_operand, scasm_regx);
          scasm_regx->set_type(scasm::operand_type::REG);
          scasm_regx->set_reg(scasm::register_type::XMM0);

          MAKE_SHARED(scasm::scasm_operand, scasm_regr);
          scasm_regr->set_type(scasm::operand_type::REG);
          scasm_regr->set_reg(scasm::register_type::AX);

          MAKE_SHARED(scasm::scasm_operand, scasm_src);
          SETVAL_OPERAND(scasm_src, get_src1);

          MAKE_SHARED(scasm::scasm_operand, scasm_dst);
          SETVAL_OPERAND(scasm_dst, get_dst);

          MAKE_SHARED(scasm::scasm_operand, scasm_label1);
          scasm_label1->set_type(scasm::operand_type::LABEL);
          scasm_label1->set_identifier_stack(label1);

          MAKE_SHARED(scasm::scasm_operand, scasm_label2);
          scasm_label2->set_type(scasm::operand_type::LABEL);
          scasm_label2->set_identifier_stack(label2);

          constant::Constant constVal;
          constVal.set_type(constant::Type::DOUBLE);
          constVal.set_value({.d = 9223372036854775808.0});
          std::string doubleName;

          if (doubleLabelMap.find(constVal.get_value().d) ==
              doubleLabelMap.end()) {
            /* declare a top level constant for the double */
            doubleName = get_const_label_name();
            MAKE_SHARED(scasm::scasm_static_constant, doubleConst);
            doubleConst->set_name(doubleName);
            doubleConst->set_init(constVal);
            doubleConst->set_alignment(8);
            auto top_level_elem =
                std::static_pointer_cast<scasm::scasm_top_level>(doubleConst);
            top_level_elem->set_type(
                scasm::scasm_top_level_type::STATIC_CONSTANT);
            top_level_elem->set_global(false);
            scasm_program.add_elem(std::move(top_level_elem));
            /* add the constant in the backend symbol table */
            scasm::backendSymbol sym;
            sym.type = scasm::backendSymbolType::STATIC_CONSTANT;
            sym.isTopLevel = true;
            sym.asmType = scasm::AssemblyType::DOUBLE;
            backendSymbolTable[doubleName] = sym;
            /* add it to the double map so that it can be used again */
            doubleLabelMap[constVal.get_value().d] = doubleName;
          } else {
            /* get the identifier from doubleMap and put it in target */
            doubleName = doubleLabelMap[constVal.get_value().d];
          }

          /* SCASM GEN */

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
          scasm_inst->set_type(scasm::instruction_type::CMP);
          scasm_inst->set_asm_type(scasm::AssemblyType::DOUBLE);
          MAKE_SHARED(scasm::scasm_operand, scasm_doubleConst);
          scasm_doubleConst->set_type(scasm::operand_type::DATA);
          scasm_doubleConst->set_identifier_stack(doubleName);
          scasm_inst->set_src(std::move(scasm_doubleConst));
          scasm_inst->set_dst(scasm_src);
          scasm_func->add_instruction(std::move(scasm_inst));

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst2);
          scasm_inst2->set_type(scasm::instruction_type::JMPCC);
          MAKE_SHARED(scasm::scasm_operand, scasm_src2);
          scasm_src2->set_type(scasm::operand_type::COND);
          scasm_src2->set_cond(scasm::cond_code::AE);
          scasm_inst2->set_src(std::move(scasm_src2));
          scasm_inst2->set_dst(scasm_label1);
          scasm_func->add_instruction(std::move(scasm_inst2));

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst3);
          scasm_inst3->set_type(scasm::instruction_type::CVTTS2DI);
          scasm_inst3->set_asm_type(instType);
          scasm_inst3->set_src(scasm_src);
          scasm_inst3->set_dst(scasm_dst);
          scasm_func->add_instruction(std::move(scasm_inst3));

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst4);
          scasm_inst4->set_type(scasm::instruction_type::JMP);
          scasm_inst4->set_src(scasm_label2);
          scasm_func->add_instruction(std::move(scasm_inst4));

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst5);
          scasm_inst5->set_type(scasm::instruction_type::LABEL);
          scasm_inst5->set_src(scasm_label1);
          scasm_func->add_instruction(std::move(scasm_inst5));

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst6);
          scasm_inst6->set_type(scasm::instruction_type::MOV);
          scasm_inst6->set_asm_type(scasm::AssemblyType::DOUBLE);
          scasm_inst6->set_src(scasm_src);
          scasm_inst6->set_dst(scasm_regx);
          scasm_func->add_instruction(std::move(scasm_inst6));

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst7);
          scasm_inst7->set_type(scasm::instruction_type::BINARY);
          scasm_inst7->set_asm_type(scasm::AssemblyType::DOUBLE);
          scasm_inst7->set_binop(scasm::Binop::SUB);
          MAKE_SHARED(scasm::scasm_operand, scasm_src7);
          scasm_src7->set_type(scasm::operand_type::DATA);
          scasm_src7->set_identifier_stack(doubleName);
          scasm_inst7->set_src(std::move(scasm_src7));
          scasm_inst7->set_dst(scasm_regx);
          scasm_func->add_instruction(std::move(scasm_inst7));

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst8);
          scasm_inst8->set_type(scasm::instruction_type::CVTTS2DI);
          scasm_inst8->set_asm_type(instType);
          scasm_inst8->set_src(scasm_regx);
          scasm_inst8->set_dst(scasm_dst);
          scasm_func->add_instruction(std::move(scasm_inst8));

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst9);
          scasm_inst9->set_type(scasm::instruction_type::MOV);
          scasm_inst9->set_asm_type(instType);
          MAKE_SHARED(scasm::scasm_operand, scasm_src9);
          scasm_src9->set_type(scasm::operand_type::IMM);
          constant::Constant constVal2;
          constVal2.set_type(constant::Type::ULONG);
          constVal2.set_value({.ul = 9223372036854775808ul});
          scasm_src9->set_imm(constVal2);
          scasm_inst9->set_src(std::move(scasm_src9));
          scasm_inst9->set_dst(scasm_regr);
          scasm_func->add_instruction(std::move(scasm_inst9));

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst10);
          scasm_inst10->set_type(scasm::instruction_type::BINARY);
          scasm_inst10->set_asm_type(instType);
          scasm_inst10->set_binop(scasm::Binop::ADD);
          scasm_inst10->set_src(scasm_regr);
          scasm_inst10->set_dst(scasm_dst);
          scasm_func->add_instruction(std::move(scasm_inst10));

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst11);
          scasm_inst11->set_type(scasm::instruction_type::LABEL);
          scasm_inst11->set_src(scasm_label2);
          scasm_func->add_instruction(std::move(scasm_inst11));
        }
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
