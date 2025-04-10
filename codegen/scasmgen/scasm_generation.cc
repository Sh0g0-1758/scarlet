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
      if (is_array(globalSymbolTable[var->get_identifier()->get_value()])) {
        // allignment for array is set later
      } else {
        switch (var->get_init()[0].get_type()) {
        case constant::Type::UINT:
        case constant::Type::INT:
          backendSymbolTable[var->get_identifier()->get_value()].alignment = 4;
          break;
        case constant::Type::ULONG:
        case constant::Type::LONG:
          backendSymbolTable[var->get_identifier()->get_value()].alignment = 8;
          break;
        case constant::Type::DOUBLE:
          backendSymbolTable[var->get_identifier()->get_value()].alignment = 8;
          break;
        // These two cases should never be reached as constant type zero is only
        // used to store information about uninitialized static arrays
        case constant::Type::ZERO:
        case constant::Type::NONE:
          backendSymbolTable[var->get_identifier()->get_value()].alignment =
              INT_MIN;
          break;
        }
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
        if (inst->get_src1()->get_type() == scar::val_type::VAR and
            backendSymbolTable[inst->get_src1()->get_reg()].asmType ==
                scasm::AssemblyType::BYTE_ARRAY) {
          scasm_src->set_type(scasm::operand_type::PSEUDO_MEM);
          scasm_src->set_offset(0);
          scasm_src->set_identifier(inst->get_src1()->get_reg());
        } else {
          SET_OPERAND(scasm_src, set_src, get_src1, scasm_inst);
        }
        MAKE_SHARED(scasm::scasm_operand, scasm_dst);
        SET_OPERAND(scasm_dst, set_dst, get_dst, scasm_inst);
        scasm_func->add_instruction(std::move(scasm_inst));
      } else if (inst->get_type() == scar::instruction_type::COPY_TO_OFFSET) {
        // mov (<src type>, src, PsuedoMem(dst, offset))
        // FIXME?: assumes only array types are passed
        MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
        scasm_inst->set_type(scasm::instruction_type::MOV);
        scasm_inst->set_asm_type(valToAsmType(inst->get_src1()));
        MAKE_SHARED(scasm::scasm_operand, scasm_src);
        SET_OPERAND(scasm_src, set_src, get_src1, scasm_inst);
        MAKE_SHARED(scasm::scasm_operand, scasm_dst);
        scasm_dst->set_type(scasm::operand_type::PSEUDO_MEM);
        scasm_dst->set_identifier(inst->get_dst()->get_reg());
        scasm_dst->set_offset(inst->get_offset());
        scasm_inst->set_dst(std::move(scasm_dst));
        scasm_func->add_instruction(std::move(scasm_inst));
      } else if (inst->get_type() == scar::instruction_type::ADD_PTR) {
        // if scale if one of 1,2,4,8
        // mov(<quadword>, ptr, Reg(AX))
        // mov(<quadword>, index, Reg(AX))
        // lea(Indexed(AX, DX, scale), dst)

        // if scale is not one of 1,2,4,8
        // mov(<quadword>, ptr, Reg(AX))
        // mov(<quadword>, index, Reg(AX))
        // Binary(MUL, <quadword>, Imm(scale), Reg(AX))
        // lea(Indexed(AX, DX, 1), dst)

        // if index is a constant
        // mov(<quadword>, ptr, Reg(AX))
        // lea(Memory(AX, index*scale), dst)

        MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
        scasm_inst->set_type(scasm::instruction_type::MOV);
        scasm_inst->set_asm_type(scasm::AssemblyType::QUAD_WORD);
        MAKE_SHARED(scasm::scasm_operand, scasm_src);
        SET_OPERAND(scasm_src, set_src, get_src1,
                    scasm_inst); // check if pointer goes to the psuedo operand
        MAKE_SHARED(scasm::scasm_operand, scasm_dst);
        scasm_dst->set_type(scasm::operand_type::REG);
        scasm_dst->set_reg(scasm::register_type::AX);
        scasm_inst->set_dst(std::move(scasm_dst));
        scasm_func->add_instruction(std::move(scasm_inst));
        if (inst->get_src2()->get_type() == scar::val_type::VAR) {
          MAKE_SHARED(scasm::scasm_instruction, scasm_inst2);
          scasm_inst2->set_type(scasm::instruction_type::MOV);
          scasm_inst2->set_asm_type(scasm::AssemblyType::QUAD_WORD);
          MAKE_SHARED(scasm::scasm_operand, scasm_src2);
          SET_OPERAND(scasm_src2, set_src, get_src2,
                      scasm_inst2); // use index (in the src2 field)
          MAKE_SHARED(scasm::scasm_operand, scasm_dst2);
          scasm_dst2->set_type(scasm::operand_type::REG);
          scasm_dst2->set_reg(scasm::register_type::DX);
          scasm_inst2->set_dst(std::move(scasm_dst2));
          scasm_func->add_instruction(std::move(scasm_inst2));
          if (inst->get_offset() == 1 or inst->get_offset() == 2 or
              inst->get_offset() == 4 or inst->get_offset() == 8) {
            MAKE_SHARED(scasm::scasm_instruction, scasm_inst3);
            scasm_inst3->set_type(scasm::instruction_type::LEA);
            scasm_inst3->set_asm_type(scasm::AssemblyType::QUAD_WORD);
            MAKE_SHARED(scasm::scasm_operand, scasm_src3);
            scasm_src3->set_type(scasm::operand_type::INDEXED);
            scasm_src3->set_reg(scasm::register_type::AX);
            scasm_src3->set_index(scasm::register_type::DX);
            // use scale 1 for indexed operand
            scasm_src3->set_offset(inst->get_offset());
            scasm_inst3->set_src(std::move(scasm_src3));
            MAKE_SHARED(scasm::scasm_operand, scasm_dst3);
            SET_OPERAND(scasm_dst3, set_dst, get_dst, scasm_inst3);
            scasm_func->add_instruction(std::move(scasm_inst3));

          } else {
            // multipy offset with index
            MAKE_SHARED(scasm::scasm_instruction, scasm_inst3);
            scasm_inst3->set_type(scasm::instruction_type::BINARY);
            scasm_inst3->set_binop(scasm::Binop::MUL);
            scasm_inst3->set_asm_type(scasm::AssemblyType::QUAD_WORD);
            MAKE_SHARED(scasm::scasm_operand, scasm_src3);
            scasm_src3->set_type(scasm::operand_type::IMM);
            constant::Constant offset;
            offset.set_type(constant::Type::LONG);
            offset.set_value({.l = inst->get_offset()});
            scasm_src3->set_imm(offset);
            scasm_inst3->set_src(std::move(scasm_src3));
            MAKE_SHARED(scasm::scasm_operand, scasm_dst3);
            scasm_dst3->set_type(scasm::operand_type::REG);
            scasm_dst3->set_reg(scasm::register_type::DX);
            scasm_inst3->set_dst(std::move(scasm_dst3));
            scasm_func->add_instruction(std::move(scasm_inst3));

            MAKE_SHARED(scasm::scasm_instruction, scasm_inst4);
            scasm_inst4->set_type(scasm::instruction_type::LEA);
            scasm_inst4->set_asm_type(scasm::AssemblyType::QUAD_WORD);
            MAKE_SHARED(scasm::scasm_operand, scasm_src4);
            scasm_src4->set_type(scasm::operand_type::INDEXED);
            scasm_src4->set_reg(scasm::register_type::AX);
            scasm_src4->set_index(scasm::register_type::DX);
            // use scale 1 for indexed operand
            scasm_src4->set_offset(1);
            scasm_inst4->set_src(std::move(scasm_src4));
            MAKE_SHARED(scasm::scasm_operand, scasm_dst4);
            SET_OPERAND(scasm_dst4, set_dst, get_dst, scasm_inst4);
            scasm_func->add_instruction(std::move(scasm_inst4));
          }
        } else {
          MAKE_SHARED(scasm::scasm_instruction, scasm_inst2);
          scasm_inst2->set_type(scasm::instruction_type::LEA);
          scasm_inst2->set_asm_type(scasm::AssemblyType::QUAD_WORD);
          MAKE_SHARED(scasm::scasm_operand, scasm_src2);
          scasm_src2->set_type(scasm::operand_type::MEMORY);
          scasm_src2->set_reg(scasm::register_type::AX);
          long total_offset = inst->get_offset() *
                              inst->get_src2()->get_const_val().get_value().l;
          scasm_src2->set_offset(total_offset);
          scasm_inst2->set_src(std::move(scasm_src2));
          MAKE_SHARED(scasm::scasm_operand, scasm_dst2);
          SET_OPERAND(scasm_dst2, set_dst, get_dst, scasm_inst2);
          scasm_func->add_instruction(std::move(scasm_inst2));
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
      scasm::backendSymbol sym = backendSymbolTable[it.first];
      sym.type = scasm::backendSymbolType::STATIC_VARIABLE;
      sym.asmType =
          elemToAsmType(it.second.typeDef[0], it.second.derivedTypeMap[0]);
      if (it.second.typeDef[0] == ast::ElemType::DERIVED and
          it.second.derivedTypeMap[0][0] > 0) {
        auto derivedType = it.second.derivedTypeMap[0];
        sym.size = ast::getSizeOfArrayTypeOnArch(derivedType);
        if (sym.size > 16) {
          sym.alignment = 16;
        } else {
          int i = 0;
          for (i = 0; i < (int)derivedType.size(); i++) {
            if (derivedType[i] < 0)
              break;
          }
          sym.alignment =
              ast::getSizeOfTypeOnArch((ast::ElemType)derivedType[i]);
        }
      }
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
