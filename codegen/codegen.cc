#include "codegen.hh"

namespace scarlet {
namespace codegen {

#define CODEGEN_SRC_DST()                                                      \
  if (instr->get_src()->get_type() == scasm::operand_type::IMM) {              \
    assembly << "$" << instr->get_src()->get_imm();                            \
  } else if (instr->get_src()->get_type() == scasm::operand_type::STACK) {     \
    assembly << instr->get_src()->get_identifier_stack();                      \
  } else if (instr->get_src()->get_type() == scasm::operand_type::DATA) {      \
    assembly << instr->get_src()->get_identifier_stack() << "(%rip)";          \
  } else if (instr->get_src()->get_type() == scasm::operand_type::REG) {       \
    assembly << scasm::to_string(instr->get_src()->get_reg(),                  \
                                 scasm::register_size::DWORD);                 \
  }                                                                            \
  assembly << ", ";                                                            \
  if (instr->get_dst()->get_type() == scasm::operand_type::STACK) {            \
    assembly << instr->get_dst()->get_identifier_stack();                      \
  } else if (instr->get_dst()->get_type() == scasm::operand_type::DATA) {      \
    assembly << instr->get_dst()->get_identifier_stack() << "(%rip)";          \
  } else if (instr->get_dst()->get_type() == scasm::operand_type::REG) {       \
    assembly << scasm::to_string(instr->get_dst()->get_reg(),                  \
                                 scasm::register_size::DWORD);                 \
  }

void Codegen::codegen() {
  // ###########################
  gen_scasm();
  fix_pseudo_registers();
  fix_instructions();
  // ###########################
  std::stringstream assembly;

  for (auto elem : scasm.get_elems()) {
    if (elem->get_type() == scasm::scasm_top_level_type::STATIC_VARIABLE) {
      auto vars = std::static_pointer_cast<scasm::scasm_static_variable>(elem);
      if (vars->is_global()) {
        assembly << "\t.globl ";
#ifdef __APPLE__
        assembly << "_" << vars->get_name() << "\n";
#else
        assembly << vars->get_name() << "\n";
#endif
      }
      if (vars->get_init() == 0) {
        assembly << "\t.bss\n";
      } else {
        assembly << "\t.data\n";
      }
#ifdef __APPLE__
      assembly << "\t.balign 4\n";
      assembly << "_" << vars->get_name() << ":\n";
#else
      assembly << "\t.align 4\n";
      assembly << vars->get_name() << ":\n";
#endif
      if (vars->get_init() != 0) {
        assembly << "\t.long " << vars->get_init() << "\n";
      } else {
        assembly << "\t.zero 4\n";
      }
      continue;
    } // STATIC_VARIABLE

    auto funcs = std::static_pointer_cast<scasm::scasm_function>(elem);
    if (funcs->is_global()) {
      assembly << "\t.globl ";
#ifdef __APPLE__
      assembly << "_" << funcs->get_name() << "\n";
#else
      assembly << funcs->get_name() << "\n";
#endif
    }

    assembly << "\t.text\n";
#ifdef __APPLE__
    assembly << "_" << funcs->get_name() << ":\n";
#else
    assembly << funcs->get_name() << ":\n";
#endif
    assembly << "\tpushq %rbp\n";
    assembly << "\tmovq %rsp, %rbp\n";
    for (auto instr : funcs->get_instructions()) {
      if (instr->get_type() == scasm::instruction_type::ALLOCATE_STACK) {
        assembly << "\tsubq $" << instr->get_src()->get_imm() << ", %rsp\n";
      } else if (instr->get_type() ==
                 scasm::instruction_type::DEALLOCATE_STACK) {
        assembly << "\taddq $" << instr->get_src()->get_imm() << ", %rsp\n";
      } else if (instr->get_type() == scasm::instruction_type::CALL) {
        std::string funcName = instr->get_src()->get_identifier_stack();
#ifdef __APPLE__
        assembly << "\tcall " << "_" << funcName << "\n";
#else
        if (globalSymbolTable[funcName].def == symbolTable::defType::TRUE) {
          assembly << "\tcall " << funcName << "\n";
        } else {
          assembly << "\tcall " << funcName << "@PLT" << "\n";
        }
#endif
      } else if (instr->get_type() == scasm::instruction_type::PUSH) {
        assembly << "\tpushq ";
        if (instr->get_src()->get_type() == scasm::operand_type::STACK) {
          assembly << instr->get_src()->get_identifier_stack();
        } else if (instr->get_src()->get_type() == scasm::operand_type::REG) {
          assembly << scasm::to_string(instr->get_src()->get_reg(),
                                       scasm::register_size::QWORD);
        } else if (instr->get_src()->get_type() == scasm::operand_type::IMM) {
          assembly << "$" << instr->get_src()->get_imm();
        } else if (instr->get_src()->get_type() == scasm::operand_type::DATA) {
          assembly << instr->get_src()->get_identifier_stack() << "(%rip)";
        }
        assembly << "\n";
      } else if (instr->get_type() == scasm::instruction_type::RET) {
        assembly << "\tmovq %rbp, %rsp\n";
        assembly << "\tpopq %rbp\n";
        assembly << "\tret\n";
      } else if (instr->get_type() == scasm::instruction_type::MOV) {
        assembly << "\tmovl ";
        CODEGEN_SRC_DST();
        assembly << "\n";
      } else if (instr->get_type() == scasm::instruction_type::UNARY) {
        assembly << "\t";
        assembly << scasm::to_string(instr->get_unop()) << " ";
        if (instr->get_dst()->get_type() == scasm::operand_type::STACK) {
          assembly << instr->get_dst()->get_identifier_stack();
        } else if (instr->get_dst()->get_type() == scasm::operand_type::REG) {
          assembly << scasm::to_string(instr->get_src()->get_reg(),
                                       scasm::register_size::DWORD);
        } else if (instr->get_dst()->get_type() == scasm::operand_type::DATA) {
          assembly << instr->get_dst()->get_identifier_stack() << "(%rip)";
        }
        assembly << "\n";
      } else if (instr->get_type() == scasm::instruction_type::CDQ) {
        assembly << "\tcdq\n";
      } else if (instr->get_type() == scasm::instruction_type::IDIV) {
        assembly << "\tidivl ";
        if (instr->get_src()->get_type() == scasm::operand_type::STACK) {
          assembly << instr->get_src()->get_identifier_stack();
        } else if (instr->get_src()->get_type() == scasm::operand_type::REG) {
          assembly << scasm::to_string(instr->get_src()->get_reg(),
                                       scasm::register_size::DWORD);
        } else if (instr->get_src()->get_type() == scasm::operand_type::DATA) {
          assembly << instr->get_src()->get_identifier_stack() << "(%rip)";
        }
        assembly << "\n";
      } else if (instr->get_type() == scasm::instruction_type::BINARY) {
        assembly << "\t";
        assembly << scasm::to_string(instr->get_binop()) << " ";
        CODEGEN_SRC_DST();
        assembly << "\n";
      } else if (instr->get_type() == scasm::instruction_type::JMP) {
#ifdef __APPLE__
        assembly << "\tjmp " << "L" << instr->get_src()->get_identifier_stack()
                 << "\n";
#else
        assembly << "\tjmp " << ".L" << instr->get_src()->get_identifier_stack()
                 << "\n";
#endif
      } else if (instr->get_type() == scasm::instruction_type::LABEL) {
#ifdef __APPLE__
        assembly << "L" << instr->get_src()->get_identifier_stack() << ":\n";
#else
        assembly << ".L" << instr->get_src()->get_identifier_stack() << ":\n";
#endif
      } else if (instr->get_type() == scasm::instruction_type::CMP) {
        assembly << "\tcmpl ";
        CODEGEN_SRC_DST();
        assembly << "\n";
      } else if (instr->get_type() == scasm::instruction_type::JMPCC) {
        assembly << "\tj";
        assembly << scasm::to_string(instr->get_src()->get_cond()) << " ";
#ifdef __APPLE__
        assembly << "L" << instr->get_dst()->get_identifier_stack() << "\n";
#else
        assembly << ".L" << instr->get_dst()->get_identifier_stack() << "\n";
#endif
      } else if (instr->get_type() == scasm::instruction_type::SETCC) {
        assembly << "\tset";
        assembly << scasm::to_string(instr->get_src()->get_cond()) << " ";
        if (instr->get_dst()->get_type() == scasm::operand_type::STACK) {
          assembly << instr->get_dst()->get_identifier_stack();
        } else if (instr->get_dst()->get_type() == scasm::operand_type::REG) {
          assembly << scasm::to_string(instr->get_dst()->get_reg(),
                                       scasm::register_size::BYTE);
        } else if (instr->get_dst()->get_type() == scasm::operand_type::DATA) {
          assembly << instr->get_dst()->get_identifier_stack() << "(%rip)";
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
