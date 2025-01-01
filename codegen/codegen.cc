#include "codegen.hh"

void Codegen::gen_scar() {
  scar_Program_Node scar_program;
  for (auto it : program.get_functions()) {
    scar_Function_Node scar_function;
    scar_Identifier_Node identifier;
    identifier.set_value(it.get_identifier().get_value());
    scar_function.set_identifier(identifier);
    for (auto inst : it.get_statements()) {
      if (inst.get_type() == "Return") {
        for (auto exp : inst.get_exps()) {
          if (exp.get_unop_nodes().size() > 0) {
            for (int i = exp.get_unop_nodes().size() - 1; i >= 0; i--) {
              scar_Instruction_Node scar_instruction;
              scar_instruction.set_type("Unary");
              if (exp.get_unop_nodes()[i].get_op() == "Negate") {
                scar_instruction.set_op(UNOP::NEGATE);
              } else if (exp.get_unop_nodes()[i].get_op() == "Complement") {
                scar_instruction.set_op(UNOP::COMPLEMENT);
              }

              scar_Val_Node scar_val_src;
              scar_Val_Node scar_val_dst;

              scar_val_dst.set_type("Var");
              scar_val_dst.set_reg_name(get_reg_name());
              scar_instruction.set_dst(scar_val_dst);

              if (i == exp.get_unop_nodes().size() - 1) {
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
            scar_Instruction_Node scar_instruction;
            scar_instruction.set_type("Return");
            scar_Val_Node scar_val_ret;
            scar_val_ret.set_type("Var");
            scar_val_ret.set_reg_name(
                scar_function
                    .get_instructions()
                        [scar_function.get_instructions().size() - 1]
                    .get_dst()
                    .get_reg());
            scar_instruction.set_src_ret(scar_val_ret);
            scar_instruction.set_op(UNOP::UNKNOWN);
            scar_function.add_instruction(scar_instruction);
          } else {
            scar_Instruction_Node scar_instruction;
            scar_instruction.set_type("Return");
            scar_Val_Node scar_val_ret;
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
    for (auto statement : function.get_body()) {
      std::cerr << "\t\t\t" << statement.get_type() << "(";
      if (statement.get_op() == UNOP::UNKNOWN) {
        if (statement.get_src_ret().get_type() == "Constant") {
          std::cerr << "Constant(" << statement.get_src_ret().get_value()
                    << ")";
        } else if (statement.get_src_ret().get_type() == "Var") {
          std::cerr << "Var(" << statement.get_src_ret().get_reg() << ")";
        }
        std::cerr << ")" << std::endl;
      } else {
        if (statement.get_op() == UNOP::COMPLEMENT) {
          std::cerr << "Complement, ";
        } else if (statement.get_op() == UNOP::NEGATE) {
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

void Codegen::codegen() {
  std::string assembly;
  for (auto it : program.get_functions()) {
    assembly += "\t.globl ";
    assembly += it.get_identifier().get_value() + "\n";
    assembly += it.get_identifier().get_value() + ":\n";
    for (auto inst : it.get_statements()) {
      if (inst.get_type() == "Return") {
        assembly += "\tmovl ";
        assembly +=
            "$" + inst.get_exps()[0].get_int_node().get_value() + ", %eax\n";
        assembly += "\tret\n";
      }
    }
  }

  assembly += "\t.section    .note.GNU-stack,\"\",@progbits\n";

  std::ofstream file(file_name);
  if (file.is_open()) {
    file << assembly;
    file.close();
  } else {
    std::cerr << "[ERROR]: Unable to open file" << std::endl;
    success = false;
  }
}
