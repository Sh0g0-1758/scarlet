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
      switch (var->get_init()[0].get_type()) {
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
      case constant::Type::CHAR:
      case constant::Type::UCHAR:
        scasm_var->set_alignment(1);
        break;
      // FIXME: For Arrays
      case constant::Type::ZERO:
      case constant::Type::NONE:
        scasm_var->set_alignment(0);
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
      scasm_dst->set_identifier(
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
      scasm_dst->set_identifier(
          func->get_params()[double_param_indx[i]]->get_value());
      scasm_inst->set_dst(std::move(scasm_dst));
      scasm_func->add_instruction(std::move(scasm_inst));
    }

    // Move stack args
    for (int i = 0; i < (int)stack_param_indx.size(); i++) {
      MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
      scasm_inst->set_type(scasm::instruction_type::MOV);
      scasm_inst->set_asm_type(stack_param_indx[i].first);
      MAKE_SHARED(scasm::scasm_operand, scasm_src);
      scasm_src->set_type(scasm::operand_type::MEMORY);
      scasm_src->set_reg(scasm::register_type::BP);
      scasm_src->set_offset(16 + 8 * i);
      scasm_inst->set_src(std::move(scasm_src));
      MAKE_SHARED(scasm::scasm_operand, scasm_dst);
      scasm_dst->set_type(scasm::operand_type::PSEUDO);
      scasm_dst->set_identifier(
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
        scasm_src->set_identifier(inst->get_src1()->get_label());
        scasm_inst->set_src(std::move(scasm_src));
        scasm_func->add_instruction(std::move(scasm_inst));
      } else if (inst->get_type() == scar::instruction_type::JUMP) {
        MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
        scasm_inst->set_type(scasm::instruction_type::JMP);
        MAKE_SHARED(scasm::scasm_operand, scasm_src);
        scasm_src->set_type(scasm::operand_type::LABEL);
        scasm_src->set_identifier(inst->get_src1()->get_label());
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
        scasm_dst2->set_identifier(inst->get_dst()->get_label());
        scasm_inst2->set_dst(std::move(scasm_dst2));
        scasm_func->add_instruction(std::move(scasm_inst2));
      } else if (inst->get_type() == scar::instruction_type::CALL) {
        gen_funcall_scasm(inst, scasm_func, scasm_program);
      } else if (scar::is_type_cast(inst->get_type())) {
        gen_type_cast_scasm(inst, scasm_func, scasm_program);
      } else if (inst->get_type() == scar::instruction_type::LOAD) {
        // <QuadWord> Mov(src, reg(<R>))
        // <dstType> Mov(Mem(<R>), dst)

        MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
        scasm_inst->set_type(scasm::instruction_type::MOV);
        scasm_inst->set_asm_type(scasm::AssemblyType::QUAD_WORD);
        MAKE_SHARED(scasm::scasm_operand, scasm_src);
        SET_OPERAND(scasm_src, set_src, get_src1, scasm_inst);
        MAKE_SHARED(scasm::scasm_operand, scasm_dst);
        scasm_dst->set_type(scasm::operand_type::REG);
        scasm_dst->set_reg(scasm::register_type::AX);
        scasm_inst->set_dst(std::move(scasm_dst));
        scasm_func->add_instruction(std::move(scasm_inst));

        MAKE_SHARED(scasm::scasm_instruction, scasm_inst2);
        scasm_inst2->set_type(scasm::instruction_type::MOV);
        scasm_inst2->set_asm_type(valToAsmType(inst->get_dst()));
        MAKE_SHARED(scasm::scasm_operand, scasm_src2);
        scasm_src2->set_type(scasm::operand_type::MEMORY);
        scasm_src2->set_reg(scasm::register_type::AX);
        scasm_src2->set_offset(0);
        scasm_inst2->set_src(std::move(scasm_src2));
        MAKE_SHARED(scasm::scasm_operand, scasm_dst2);
        SET_OPERAND(scasm_dst2, set_dst, get_dst, scasm_inst2);
        scasm_func->add_instruction(std::move(scasm_inst2));
      } else if (inst->get_type() == scar::instruction_type::STORE) {
        // <QuadWord> Mov(dst, reg(<R>))
        // <srcType> Mov(src, Mem(<R>))

        MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
        scasm_inst->set_type(scasm::instruction_type::MOV);
        scasm_inst->set_asm_type(scasm::AssemblyType::QUAD_WORD);
        MAKE_SHARED(scasm::scasm_operand, scasm_src);
        SET_OPERAND(scasm_src, set_src, get_dst, scasm_inst);
        MAKE_SHARED(scasm::scasm_operand, scasm_dst);
        scasm_dst->set_type(scasm::operand_type::REG);
        scasm_dst->set_reg(scasm::register_type::AX);
        scasm_inst->set_dst(std::move(scasm_dst));
        scasm_func->add_instruction(std::move(scasm_inst));

        MAKE_SHARED(scasm::scasm_instruction, scasm_inst2);
        scasm_inst2->set_type(scasm::instruction_type::MOV);
        scasm_inst2->set_asm_type(instType);
        MAKE_SHARED(scasm::scasm_operand, scasm_src2);
        SET_OPERAND(scasm_src2, set_src, get_src1, scasm_inst2);
        MAKE_SHARED(scasm::scasm_operand, scasm_dst2);
        scasm_dst2->set_type(scasm::operand_type::MEMORY);
        scasm_dst2->set_reg(scasm::register_type::AX);
        scasm_dst2->set_offset(0);
        scasm_inst2->set_dst(std::move(scasm_dst2));
        scasm_func->add_instruction(std::move(scasm_inst2));
      } else if (inst->get_type() == scar::instruction_type::GET_ADDRESS) {
        // Lea(src, dst)

        MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
        scasm_inst->set_type(scasm::instruction_type::LEA);
        scasm_inst->set_asm_type(scasm::AssemblyType::QUAD_WORD);
        MAKE_SHARED(scasm::scasm_operand, scasm_src);
        SET_OPERAND(scasm_src, set_src, get_src1, scasm_inst);
        MAKE_SHARED(scasm::scasm_operand, scasm_dst);
        SET_OPERAND(scasm_dst, set_dst, get_dst, scasm_inst);
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
