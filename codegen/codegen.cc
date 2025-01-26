#include "codegen.hh"
namespace scarlet {
namespace codegen {

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
          if (exp.get_unop_nodes().size() > 0) {
            for (int i = exp.get_unop_nodes().size() - 1; i >= 0; i--) {
              scar::scar_Instruction_Node scar_instruction;
              scar_instruction.set_type("Unary");
              if (exp.get_unop_nodes()[i].get_op() == "Negate") {
                scar_instruction.set_op(unop::UNOP::NEGATE);
              } else if (exp.get_unop_nodes()[i].get_op() == "Complement") {
                scar_instruction.set_op(unop::UNOP::COMPLEMENT);
              }

              scar::scar_Val_Node scar_val_src;
              scar::scar_Val_Node scar_val_dst;

              scar_val_dst.set_type("Var");
              scar_val_dst.set_reg_name(get_reg_name());
              scar_instruction.set_dst(scar_val_dst);

              if (static_cast<long unsigned int>(i) ==
                  exp.get_unop_nodes().size() - 1) {
                scar_val_src.set_type("Constant");
                scar_val_src.set_value(exp.get_int_node().get_value());
                scar_instruction.set_src_ret(scar_val_src);
              } else {
                scar_val_src.set_type("Var");
                scar_val_src.set_reg_name(
                    scar_function
                        .get_instructions()
                            [scar_function.get_instructions().size() - 1]
                        .get_dst()
                        .get_reg());
                scar_instruction.set_src_ret(scar_val_src);
              }

              scar_function.add_instruction(scar_instruction);
            }
            scar::scar_Instruction_Node scar_instruction;
            scar_instruction.set_type("Return");
            scar::scar_Val_Node scar_val_ret;
            scar_val_ret.set_type("Var");
            scar_val_ret.set_reg_name(
                scar_function
                    .get_instructions()
                        [scar_function.get_instructions().size() - 1]
                    .get_dst()
                    .get_reg());
            scar_instruction.set_src_ret(scar_val_ret);
            scar_instruction.set_op(unop::UNOP::UNKNOWN);
            scar_function.add_instruction(scar_instruction);
          } else {
            scar::scar_Instruction_Node scar_instruction;
            scar_instruction.set_type("Return");
            scar::scar_Val_Node scar_val_ret;
            scar_val_ret.set_type("Constant");
            scar_val_ret.set_value(exp.get_int_node().get_value());
            scar_instruction.set_src_ret(scar_val_ret);
            scar_function.add_instruction(scar_instruction);
          }
        }
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
      std::cerr << "\t\t\t" << statement.get_type() << "(";
      if (statement.get_op() == unop::UNOP::UNKNOWN) {
        if (statement.get_src_ret().get_type() == "Constant") {
          std::cerr << "Constant(" << statement.get_src_ret().get_value()
                    << ")";
        } else if (statement.get_src_ret().get_type() == "Var") {
          std::cerr << "Var(" << statement.get_src_ret().get_reg() << ")";
        }
        std::cerr << ")" << std::endl;
      } else {
        if (statement.get_op() == unop::UNOP::COMPLEMENT) {
          std::cerr << "Complement, ";
        } else if (statement.get_op() == unop::UNOP::NEGATE) {
          std::cerr << "Negate, ";
        }
        if (statement.get_src_ret().get_type() == "Constant") {
          std::cerr << "Constant(" << statement.get_src_ret().get_value()
                    << ")";
        } else if (statement.get_src_ret().get_type() == "Var") {
          std::cerr << "Var(" << statement.get_src_ret().get_reg() << ")";
        }
        std::cerr << ", ";
        if (statement.get_dst().get_type() == "Var") {
          std::cerr << "Var(" << statement.get_dst().get_reg() << ")";
        } else if (statement.get_dst().get_type() == "Constant") {
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
  scasm::scasm_program scasm_program;
  for (auto func : scar.get_functions()) {
    scasm::scasm_function scasm_func;
    scasm_func.set_name(func.get_identifier().get_value());
    for (auto inst : func.get_instructions()) {
      if (inst.get_type() == "Return") {
        scasm::scasm_instruction scasm_inst;
        scasm_inst.set_type("Mov");
        scasm::scasm_operand scasm_src;
        if (inst.get_src_ret().get_type() == "Constant") {
          scasm_src.set_type("Imm");
          scasm_src.set_value(inst.get_src_ret().get_value());
        } else if (inst.get_src_ret().get_type() == "Var") {
          scasm_src.set_type("Pseudo");
          scasm_src.set_value(inst.get_src_ret().get_reg());
        }
        scasm_inst.set_src(scasm_src);
        scasm::scasm_operand scasm_dst;
        scasm_dst.set_type("Reg");
        scasm_dst.set_value("AX");
        scasm_inst.set_dst(scasm_dst);
        scasm_func.add_instruction(scasm_inst);
        scasm::scasm_instruction scasm_inst2;
        scasm_inst2.set_type("Ret");
        scasm_func.add_instruction(scasm_inst2);
      } else if (inst.get_type() == "Unary") {
        scasm::scasm_instruction scasm_inst;
        scasm_inst.set_type("Mov");

        scasm::scasm_operand scasm_src;
        if (inst.get_src_ret().get_type() == "Constant") {
          scasm_src.set_type("Imm");
          scasm_src.set_value(inst.get_src_ret().get_value());
        } else if (inst.get_src_ret().get_type() == "Var") {
          scasm_src.set_type("Pseudo");
          scasm_src.set_value(inst.get_src_ret().get_reg());
        }
        scasm_inst.set_src(scasm_src);

        scasm::scasm_operand scasm_dst;
        if (inst.get_dst().get_type() == "Var") {
          scasm_dst.set_type("Pseudo");
          scasm_dst.set_value(inst.get_dst().get_reg());
        }
        scasm_inst.set_dst(scasm_dst);

        scasm_func.add_instruction(scasm_inst);

        scasm::scasm_instruction scasm_inst2;
        scasm_inst2.set_type("Unary");
        scasm_inst2.set_op(inst.get_op());

        scasm::scasm_operand scasm_dst2;
        if (inst.get_dst().get_type() == "Var") {
          scasm_dst2.set_type("Pseudo");
          scasm_dst2.set_value(inst.get_dst().get_reg());
        }
        scasm_inst2.set_dst(scasm_dst2);

        scasm_func.add_instruction(scasm_inst2);
      }
    }
    scasm_program.add_function(scasm_func);
  }

  this->scasm = scasm_program;
}

void Codegen::fix_pseudo_registers() {
  int offset = 1;
  for (auto &funcs : scasm.get_functions()) {
    for (auto &inst : funcs.get_instructions()) {
      if (inst.get_src().get_type() == "Pseudo") {
        if (pseduo_registers.find(inst.get_src().get_value()) !=
            pseduo_registers.end()) {
          inst.get_src().set_value(
              pseduo_registers[inst.get_src().get_value()]);
        } else {
          std::string temp = inst.get_src().get_value();
          inst.get_src().set_value("-" + std::to_string(offset * 4) + "(%rbp)");
          pseduo_registers[temp] = inst.get_src().get_value();
          offset++;
        }
        inst.get_src().set_type("Stack");
      }
      if (inst.get_dst().get_type() == "Pseudo") {
        if (pseduo_registers.find(inst.get_dst().get_value()) !=
            pseduo_registers.end()) {
          inst.get_dst().set_value(
              pseduo_registers[inst.get_dst().get_value()]);
        } else {
          std::string temp = inst.get_dst().get_value();
          inst.get_dst().set_value("-" + std::to_string(offset * 4) + "(%rbp)");
          pseduo_registers[temp] = inst.get_dst().get_value();
          offset++;
        }
        inst.get_dst().set_type("Stack");
      }
    }
  }
  stack_offset = 4 * (offset - 1);
}

void Codegen::fix_instructions() {
  scasm::scasm_instruction scasm_stack_instr;
  scasm_stack_instr.set_type("AllocateStack");
  scasm::scasm_operand val;
  val.set_type("Imm");
  val.set_value(std::to_string(stack_offset));
  scasm_stack_instr.set_src(val);

  scasm.get_functions()[0].get_instructions().insert(
      scasm.get_functions()[0].get_instructions().begin(), scasm_stack_instr);

  for (auto &funcs : scasm.get_functions()) {
    for (auto it = funcs.get_instructions().begin();
         it != funcs.get_instructions().end(); it++) {
      if ((*it).get_src().get_type() == "Stack" &&
          (*it).get_dst().get_type() == "Stack") {
        scasm::scasm_instruction scasm_inst;
        scasm_inst.set_type((*it).get_type());
        scasm_inst.set_dst((*it).get_dst());

        // fixing up stack to stack move
        scasm::scasm_operand src;
        src.set_type("Reg");
        src.set_value("R10");
        scasm_inst.set_src(src);

        scasm::scasm_operand dst;
        dst.set_type("Reg");
        dst.set_value("R10");
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
      if (instr.get_type() == "AllocateStack") {
        assembly << "\tsubq $" << instr.get_src().get_value() << ", %rsp\n";
      } else if (instr.get_type() == "Ret") {
        assembly << "\tmovq %rbp, %rsp\n";
        assembly << "\tpopq %rbp\n";
        assembly << "\tret\n";
      } else if (instr.get_type() == "Mov") {
        assembly << "\tmovl ";
        if (instr.get_src().get_type() == "Imm") {
          assembly << "$" << instr.get_src().get_value();
        } else if (instr.get_src().get_type() == "Stack") {
          assembly << instr.get_src().get_value();
        } else if (instr.get_src().get_type() == "Reg") {
          if (instr.get_src().get_value() == "AX") {
            assembly << "%eax";
          } else if (instr.get_src().get_value() == "R10") {
            assembly << "%r10d";
          }
        }
        assembly << ", ";
        if (instr.get_dst().get_type() == "Stack") {
          assembly << instr.get_dst().get_value();
        } else if (instr.get_dst().get_type() == "Reg") {
          if (instr.get_dst().get_value() == "AX") {
            assembly << "%eax";
          } else if (instr.get_dst().get_value() == "R10") {
            assembly << "%r10d";
          }
        }
        assembly << "\n";
      } else if (instr.get_type() == "Unary") {
        if (instr.get_op() == unop::UNOP::NEGATE) {
          assembly << "\tnegl ";
        } else if (instr.get_op() == unop::UNOP::COMPLEMENT) {
          assembly << "\tnotl ";
        }
        if (instr.get_dst().get_type() == "Stack") {
          assembly << instr.get_dst().get_value();
        } else if (instr.get_dst().get_type() == "Reg") {
          if (instr.get_dst().get_value() == "AX") {
            assembly << "%eax";
          } else if (instr.get_dst().get_value() == "R10") {
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
