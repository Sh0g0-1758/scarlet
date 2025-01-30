#include "codegen.hh"
namespace scarlet {
namespace codegen {

void Codegen::gen_scar_factor(ast::AST_factor_Node &factor,
                              scar::scar_Function_Node &scar_function) {
  // firstly put all the unops (if they exist) in the unop buffer
  for (auto it : factor.get_unop_nodes()) {
    unop_buffer.emplace_back(it.get_op());
  }
  // If we have an integer node and unops to operate on, proceed...
  if (!factor.get_int_node().get_value().empty() and !unop_buffer.empty()) {
    int num_unpos = unop_buffer.size();
    for (int i = num_unpos - 1; i >= 0; i--) {
      scar::scar_Instruction_Node scar_instruction;
      scar_instruction.set_type(scar::instruction_type::UNARY);
      scar_instruction.set_unop(unop_buffer[i]);

      scar::scar_Val_Node scar_val_src;
      scar::scar_Val_Node scar_val_dst;

      // deal with the source
      if (i == num_unpos - 1) {
        scar_val_src.set_type(scar::val_type::CONSTANT);
        scar_val_src.set_value(factor.get_int_node().get_value());
        scar_instruction.set_src_ret(scar_val_src);
      } else {
        scar_val_src.set_type(scar::val_type::VAR);
        scar_val_src.set_reg_name(get_prev_reg_name());
        scar_instruction.set_src_ret(scar_val_src);
      }

      // deal with the destination
      scar_val_dst.set_type(scar::val_type::VAR);
      scar_val_dst.set_reg_name(get_reg_name());
      scar_instruction.set_dst(scar_val_dst);

      scar_function.add_instruction(scar_instruction);
    }
    // empty the unop buffer
    unop_buffer.clear();
  } else {
    // save it for later return
    constant_buffer = factor.get_int_node().get_value();
  }
  gen_scar_exp(factor.get_exp_node(), scar_function);
}

void Codegen::gen_scar_exp(ast::AST_exp_Node *exp,
                           scar::scar_Function_Node &scar_function) {
  if (exp == nullptr)
    return;
  gen_scar_exp(exp->get_left(), scar_function);
  if (exp->get_binop_node().get_op() != binop::BINOP::UNKNOWN) {
    // when we have a binary operator
    scar::scar_Instruction_Node scar_instruction;
    scar_instruction.set_type(scar::instruction_type::BINARY);
    scar_instruction.set_binop(exp->get_binop_node().get_op());
    scar::scar_Val_Node scar_val_src1;
    scar::scar_Val_Node scar_val_src2;
    scar::scar_Val_Node scar_val_dst;

    if (exp->get_left() == nullptr) {
      gen_scar_factor(exp->get_factor_node(), scar_function);
      if (constant_buffer.empty()) {
        scar_val_src1.set_type(scar::val_type::VAR);
        scar_val_src1.set_reg_name(get_prev_reg_name());
      } else {
        scar_val_src1.set_type(scar::val_type::CONSTANT);
        scar_val_src1.set_value(constant_buffer);
        constant_buffer.clear();
      }
    } else {
      scar_val_src1.set_type(scar::val_type::VAR);
      scar_val_src1.set_reg_name(get_prev_reg_name());
    }

    gen_scar_exp(exp->get_right(), scar_function);
    if (constant_buffer.empty()) {
      scar_val_src2.set_type(scar::val_type::VAR);
      scar_val_src2.set_reg_name(get_prev_reg_name());
    } else {
      scar_val_src2.set_type(scar::val_type::CONSTANT);
      scar_val_src2.set_value(constant_buffer);
      constant_buffer.clear();
    }

    scar_val_dst.set_type(scar::val_type::VAR);
    scar_val_dst.set_reg_name(get_reg_name());

    scar_instruction.set_src_ret(scar_val_src1);
    scar_instruction.set_src2(scar_val_src2);
    scar_instruction.set_dst(scar_val_dst);

    scar_function.add_instruction(scar_instruction);
  } else {
    // When we do not have a binary operator, so only parse the factor node
    ast::AST_factor_Node factor = exp->get_factor_node();
    gen_scar_factor(factor, scar_function);
  }
}

void Codegen::gen_scar() {
  scar::scar_Program_Node scar_program;
  for (auto it : program.get_functions()) {
    scar::scar_Function_Node scar_function;
    scar::scar_Identifier_Node identifier;
    identifier.set_value(it.get_identifier().get_value());
    scar_function.set_identifier(identifier);
    for (auto inst : it.get_statements()) {
      if (inst.get_type() == "Return") {
        for (auto exp : inst.get_exps()) {
          gen_scar_exp(exp, scar_function);
        }
        scar::scar_Instruction_Node scar_instruction;
        scar_instruction.set_type(scar::instruction_type::RETURN);
        scar::scar_Val_Node scar_val_ret;
        if (constant_buffer.empty()) {
          scar_val_ret.set_type(scar::val_type::VAR);
          scar_val_ret.set_reg_name(get_prev_reg_name());
        } else {
          scar_val_ret.set_type(scar::val_type::CONSTANT);
          scar_val_ret.set_value(constant_buffer);
          constant_buffer.clear();
        }
        scar_instruction.set_src_ret(scar_val_ret);
        scar_function.add_instruction(scar_instruction);
      }
    }
    scar_program.add_function(scar_function);
  }

  this->scar = scar_program;
}

void Codegen::pretty_print() {
  std::cerr << "Program(" << std::endl;
  for (auto function : scar.get_functions()) {
    std::cerr << "\tFunction(" << std::endl;
    std::cerr << "\t\tname=\"" << function.get_identifier().get_value() << "\","
              << std::endl;
    std::cerr << "\t\tbody=[" << std::endl;
    for (auto statement : function.get_instructions()) {
      std::cerr << "\t\t\t" << to_string(statement.get_type()) << "(";
      if (statement.get_type() == scar::instruction_type::RETURN) {
        if (statement.get_src_ret().get_type() == scar::val_type::CONSTANT) {
          std::cerr << "Constant(" << statement.get_src_ret().get_value()
                    << ")";
        } else if (statement.get_src_ret().get_type() == scar::val_type::VAR) {
          std::cerr << "Var(" << statement.get_src_ret().get_reg() << ")";
        }
        std::cerr << ")" << std::endl;
      } else if (statement.get_type() == scar::instruction_type::UNARY) {
        if (statement.get_unop() == unop::UNOP::COMPLEMENT) {
          std::cerr << "Complement, ";
        } else if (statement.get_unop() == unop::UNOP::NEGATE) {
          std::cerr << "Negate, ";
        }
        if (statement.get_src_ret().get_type() == scar::val_type::CONSTANT) {
          std::cerr << "Constant(" << statement.get_src_ret().get_value()
                    << ")";
        } else if (statement.get_src_ret().get_type() == scar::val_type::VAR) {
          std::cerr << "Var(" << statement.get_src_ret().get_reg() << ")";
        }
        std::cerr << ", ";
        if (statement.get_dst().get_type() == scar::val_type::VAR) {
          std::cerr << "Var(" << statement.get_dst().get_reg() << ")";
        } else if (statement.get_dst().get_type() == scar::val_type::CONSTANT) {
          std::cerr << "Constant(" << statement.get_dst().get_value() << ")";
        }
        std::cerr << ")" << std::endl;
      } else if (statement.get_type() == scar::instruction_type::BINARY) {
        std::cerr << binop::to_string(statement.get_binop()) << ", ";
        if (statement.get_src_ret().get_type() == scar::val_type::VAR) {
          std::cerr << "Var(" << statement.get_src_ret().get_reg() << ")";
        } else if (statement.get_src_ret().get_type() ==
                   scar::val_type::CONSTANT) {
          std::cerr << "Constant(" << statement.get_src_ret().get_value()
                    << ")";
        }
        std::cerr << ", ";
        if (statement.get_src2().get_type() == scar::val_type::VAR) {
          std::cerr << "Var(" << statement.get_src2().get_reg() << ")";
        } else if (statement.get_src2().get_type() ==
                   scar::val_type::CONSTANT) {
          std::cerr << "Constant(" << statement.get_src2().get_value() << ")";
        }
        std::cerr << ", ";
        if (statement.get_dst().get_type() == scar::val_type::VAR) {
          std::cerr << "Var(" << statement.get_dst().get_reg() << ")";
        } else if (statement.get_dst().get_type() == scar::val_type::CONSTANT) {
          std::cerr << "Constant(" << statement.get_dst().get_value() << ")";
        }
        std::cerr << ")" << std::endl;
      }
    }
    std::cerr << "\t\t]" << std::endl;
    std::cerr << "\t)," << std::endl;
  }
  std::cerr << ")" << std::endl;
}

void Codegen::gen_scasm() {
  scasm::scasm_program scasm_program{};
  for (auto func : scar.get_functions()) {
    scasm::scasm_function scasm_func{};
    scasm_func.set_name(func.get_identifier().get_value());
    for (auto inst : func.get_instructions()) {
      if (inst.get_type() == scar::instruction_type::RETURN) {
        scasm::scasm_instruction scasm_inst{};
        scasm_inst.set_type(scasm::instruction_type::MOV);
        scasm::scasm_operand scasm_src{};
        if (inst.get_src_ret().get_type() == scar::val_type::CONSTANT) {
          scasm_src.set_type(scasm::operand_type::IMM);
          scasm_src.set_imm(stoi(inst.get_src_ret().get_value()));
        } else if (inst.get_src_ret().get_type() == scar::val_type::VAR) {
          scasm_src.set_type(scasm::operand_type::PSEUDO);
          scasm_src.set_identifier_stack(inst.get_src_ret().get_reg());
        }
        scasm_inst.set_src(scasm_src);
        scasm::scasm_operand scasm_dst{};
        scasm_dst.set_type(scasm::operand_type::REG);
        scasm_dst.set_reg(scasm::register_type::AX);
        scasm_inst.set_dst(scasm_dst);
        scasm_func.add_instruction(scasm_inst);
        scasm::scasm_instruction scasm_inst2{};
        scasm_inst2.set_type(scasm::instruction_type::RET);
        scasm_func.add_instruction(scasm_inst2);
      } else if (inst.get_type() == scar::instruction_type::UNARY) {
        scasm::scasm_instruction scasm_inst{};
        scasm_inst.set_type(scasm::instruction_type::MOV);

        scasm::scasm_operand scasm_src{};
        if (inst.get_src_ret().get_type() == scar::val_type::CONSTANT) {
          scasm_src.set_type(scasm::operand_type::IMM);
          scasm_src.set_imm(stoi(inst.get_src_ret().get_value()));
        } else if (inst.get_src_ret().get_type() == scar::val_type::VAR) {
          scasm_src.set_type(scasm::operand_type::PSEUDO);
          scasm_src.set_identifier_stack(inst.get_src_ret().get_reg());
        }
        scasm_inst.set_src(scasm_src);

        scasm::scasm_operand scasm_dst{};
        if (inst.get_dst().get_type() == scar::val_type::VAR) {
          scasm_dst.set_type(scasm::operand_type::PSEUDO);
          scasm_dst.set_identifier_stack(inst.get_dst().get_reg());
        }
        scasm_inst.set_dst(scasm_dst);

        scasm_func.add_instruction(scasm_inst);

        scasm::scasm_instruction scasm_inst2{};
        scasm_inst2.set_type(scasm::instruction_type::UNARY);
        scasm_inst2.set_unop(inst.get_unop());

        scasm::scasm_operand scasm_dst2{};
        if (inst.get_dst().get_type() == scar::val_type::VAR) {
          scasm_dst2.set_type(scasm::operand_type::PSEUDO);
          scasm_dst2.set_identifier_stack(inst.get_dst().get_reg());
        }
        scasm_inst2.set_dst(scasm_dst2);

        scasm_func.add_instruction(scasm_inst2);
      }
    }
    scasm_program.add_function(scasm_func);
  }

  this->scasm = scasm_program;
}

std::string to_string(scasm::instruction_type type) {
  switch (type) {
  case scasm::instruction_type::UNKNOWN:
    return "UNKNOWN";
  case scasm::instruction_type::MOV:
    return "MOV";
  case scasm::instruction_type::BINARY:
    return "BINARY";
  case scasm::instruction_type::IDIV:
    return "IDIV";
  case scasm::instruction_type::CDQ:
    return "CDQ";
  case scasm::instruction_type::RET:
    return "RET";
  case scasm::instruction_type::UNARY:
    return "UNARY";
  case scasm::instruction_type::ALLOCATE_STACK:
    return "ALLOCATE_STACK";
  }
  return "UNKNOWN";
}

void Codegen::fix_pseudo_registers() {
  int offset = 1;
  for (auto &funcs : scasm.get_functions()) {
    for (auto &inst : funcs.get_instructions()) {
      if (inst.get_src().get_type() == scasm::operand_type::PSEUDO) {
        if (pseduo_registers.find(inst.get_src().get_identifier_stack()) !=
            pseduo_registers.end()) {
          inst.get_src().set_identifier_stack(
              pseduo_registers[inst.get_src().get_identifier_stack()]);
        } else {
          std::string temp = inst.get_src().get_identifier_stack();
          inst.get_src().set_identifier_stack("-" + std::to_string(offset * 4) +
                                              "(%rbp)");
          pseduo_registers[temp] = inst.get_src().get_identifier_stack();
          offset++;
        }
        inst.get_src().set_type(scasm::operand_type::STACK);
      }
      if (inst.get_dst().get_type() == scasm::operand_type::PSEUDO) {
        if (pseduo_registers.find(inst.get_dst().get_identifier_stack()) !=
            pseduo_registers.end()) {
          inst.get_dst().set_identifier_stack(
              pseduo_registers[inst.get_dst().get_identifier_stack()]);
        } else {
          std::string temp = inst.get_dst().get_identifier_stack();
          inst.get_dst().set_identifier_stack("-" + std::to_string(offset * 4) +
                                              "(%rbp)");
          pseduo_registers[temp] = inst.get_dst().get_identifier_stack();
          offset++;
        }
        inst.get_dst().set_type(scasm::operand_type::STACK);
      }
    }
  }
  stack_offset = 4 * (offset - 1);
}

void Codegen::fix_instructions() {
  scasm::scasm_instruction scasm_stack_instr{};
  scasm_stack_instr.set_type(scasm::instruction_type::ALLOCATE_STACK);
  scasm::scasm_operand val{};
  val.set_type(scasm::operand_type::IMM);
  val.set_imm(stack_offset);
  scasm_stack_instr.set_src(val);

  scasm.get_functions()[0].get_instructions().insert(
      scasm.get_functions()[0].get_instructions().begin(), scasm_stack_instr);

  for (auto &funcs : scasm.get_functions()) {
    for (auto it = funcs.get_instructions().begin();
         it != funcs.get_instructions().end(); it++) {
      if ((*it).get_src().get_type() == scasm::operand_type::STACK &&
          (*it).get_dst().get_type() == scasm::operand_type::STACK) {
        scasm::scasm_instruction scasm_inst{};
        scasm_inst.set_type((*it).get_type());
        scasm_inst.set_dst((*it).get_dst());

        // fixing up stack to stack move
        scasm::scasm_operand src{};
        src.set_type(scasm::operand_type::REG);
        src.set_reg(scasm::register_type::R10);
        scasm_inst.set_src(src);

        scasm::scasm_operand dst{};
        dst.set_type(scasm::operand_type::REG);
        dst.set_reg(scasm::register_type::R10);
        (*it).set_dst(dst);
        it = funcs.get_instructions().insert(it + 1, scasm_inst);
        it++;
      }
    }
  }
}

void Codegen::codegen() {
  // ###########################
  gen_scasm();
  fix_pseudo_registers();
  fix_instructions();
  // ###########################
  std::stringstream assembly;

  for (auto funcs : scasm.get_functions()) {
#ifdef __APPLE__
    if (funcs.get_name() == "main") {
      assembly << "\t.globl " << "_main" << "\n";
      assembly << "_main" << ":\n";
    } else {
      assembly << "\t.globl " << funcs.get_name() << "\n";
      assembly << funcs.get_name() << ":\n";
    }
#else
    assembly << "\t.globl " << funcs.get_name() << "\n";
    assembly << funcs.get_name() << ":\n";
#endif
    assembly << "\tpushq %rbp\n";
    assembly << "\tmovq %rsp, %rbp\n";
    for (auto instr : funcs.get_instructions()) {
      if (instr.get_type() == scasm::instruction_type::ALLOCATE_STACK) {
        assembly << "\tsubq $" << instr.get_src().get_imm() << ", %rsp\n";
      } else if (instr.get_type() == scasm::instruction_type::RET) {
        assembly << "\tmovq %rbp, %rsp\n";
        assembly << "\tpopq %rbp\n";
        assembly << "\tret\n";
      } else if (instr.get_type() == scasm::instruction_type::MOV) {
        assembly << "\tmovl ";
        if (instr.get_src().get_type() == scasm::operand_type::IMM) {
          assembly << "$" << instr.get_src().get_imm();
        } else if (instr.get_src().get_type() == scasm::operand_type::STACK) {
          assembly << instr.get_src().get_identifier_stack();
        } else if (instr.get_src().get_type() == scasm::operand_type::REG) {
          if (instr.get_src().get_reg() == scasm::register_type::AX) {
            assembly << "%eax";
          } else if (instr.get_src().get_reg() == scasm::register_type::R10) {
            assembly << "%r10d";
          }
        }
        assembly << ", ";
        if (instr.get_dst().get_type() == scasm::operand_type::STACK) {
          assembly << instr.get_dst().get_identifier_stack();
        } else if (instr.get_dst().get_type() == scasm::operand_type::REG) {
          if (instr.get_dst().get_reg() == scasm::register_type::AX) {
            assembly << "%eax";
          } else if (instr.get_dst().get_reg() == scasm::register_type::R10) {
            assembly << "%r10d";
          }
        }
        assembly << "\n";
      } else if (instr.get_type() == scasm::instruction_type::UNARY) {
        if (instr.get_unop() == scasm::Unop::NEG) {
          assembly << "\tnegl ";
        } else if (instr.get_unop() == scasm::Unop::NOT) {
          assembly << "\tnotl ";
        }
        if (instr.get_dst().get_type() == scasm::operand_type::STACK) {
          assembly << instr.get_dst().get_identifier_stack();
        } else if (instr.get_dst().get_type() == scasm::operand_type::REG) {
          if (instr.get_dst().get_reg() == scasm::register_type::AX) {
            assembly << "%eax";
          } else if (instr.get_dst().get_reg() == scasm::register_type::R10) {
            assembly << "%r10d";
          }
        }
        assembly << "\n";
      }
    }
  }
#ifndef __APPLE__
  assembly << "\t.section    .note.GNU-stack,\"\",@progbits\n";
#endif

  std::ofstream file(file_name);
  if (file.is_open()) {
    file << assembly.str();
    file.close();
  } else {
    std::cerr << "[ERROR]: Unable to open file" << std::endl;
    success = false;
  }
}
} // namespace codegen
} // namespace scarlet
