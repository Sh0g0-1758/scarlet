#include "codegen.hh"

namespace scarlet {
namespace codegen {

#ifdef __APPLE__
#define ARCHPREFIX "_"
#else
#define ARCHPREFIX ""
#endif

#define PRINT_REG(target, type)                                                \
  switch (type) {                                                              \
  case scasm::AssemblyType::DOUBLE:                                            \
  case scasm::AssemblyType::QUAD_WORD:                                         \
    assembly << scasm::to_string(instr->get_##target()->get_reg(),             \
                                 scasm::register_size::QWORD);                 \
    break;                                                                     \
  case scasm::AssemblyType::LONG_WORD:                                         \
    assembly << scasm::to_string(instr->get_##target()->get_reg(),             \
                                 scasm::register_size::LWORD);                 \
    break;                                                                     \
  case scasm::AssemblyType::BYTE:                                              \
    assembly << scasm::to_string(instr->get_##target()->get_reg(),             \
                                 scasm::register_size::BYTE);                  \
    break;                                                                     \
  default:                                                                     \
    break;                                                                     \
  }

#define PRINT_INDEXED(target)                                                  \
  assembly << "(";                                                             \
  assembly << scasm::to_string(instr->get_##target()->get_reg(),               \
                               scasm::register_size::QWORD);                   \
  assembly << ", ";                                                            \
  assembly << scasm::to_string(instr->get_##target()->get_index(),             \
                               scasm::register_size::QWORD);                   \
  assembly << ", " << instr->get_##target()->get_offset() << ")";

#define CODEGEN_SRC(type)                                                      \
  if (instr->get_src()->get_type() == scasm::operand_type::IMM) {              \
    assembly << "$" << instr->get_src()->get_imm();                            \
  } else if (instr->get_src()->get_type() == scasm::operand_type::MEMORY) {    \
    assembly << instr->get_src()->get_offset() << "(";                         \
    assembly << scasm::to_string(instr->get_src()->get_reg(),                  \
                                 scasm::register_size::QWORD);                 \
    assembly << ")";                                                           \
  } else if (instr->get_src()->get_type() == scasm::operand_type::DATA) {      \
    assembly << ARCHPREFIX << instr->get_src()->get_identifier() << "(%rip)";  \
  } else if (instr->get_src()->get_type() == scasm::operand_type::REG) {       \
    PRINT_REG(src, type);                                                      \
  } else if (instr->get_src()->get_type() == scasm::operand_type::INDEXED) {   \
    PRINT_INDEXED(src)                                                         \
  }

#define CODEGEN_DST(type)                                                      \
  if (instr->get_dst()->get_type() == scasm::operand_type::MEMORY) {           \
    assembly << instr->get_dst()->get_offset() << "(";                         \
    assembly << scasm::to_string(instr->get_dst()->get_reg(),                  \
                                 scasm::register_size::QWORD);                 \
    assembly << ")";                                                           \
  } else if (instr->get_dst()->get_type() == scasm::operand_type::DATA) {      \
    assembly << ARCHPREFIX << instr->get_dst()->get_identifier() << "(%rip)";  \
  } else if (instr->get_dst()->get_type() == scasm::operand_type::REG) {       \
    PRINT_REG(dst, type);                                                      \
  }

#define CODEGEN_SRC_DST()                                                      \
  CODEGEN_SRC(instr->get_asm_type());                                          \
  assembly << ", ";                                                            \
  CODEGEN_DST(instr->get_asm_type());

#define POSTFIX_ASM_TYPE(type)                                                 \
  switch (type) {                                                              \
  case scasm::AssemblyType::QUAD_WORD:                                         \
    assembly << "q";                                                           \
    break;                                                                     \
  case scasm::AssemblyType::LONG_WORD:                                         \
    assembly << "l";                                                           \
    break;                                                                     \
  case scasm::AssemblyType::DOUBLE:                                            \
    assembly << "sd";                                                          \
    break;                                                                     \
  case scasm::AssemblyType::BYTE:                                              \
    assembly << "b";                                                           \
    break;                                                                     \
  default:                                                                     \
    break;                                                                     \
  }

void Codegen::asm_gen_func(std::shared_ptr<scasm::scasm_top_level> elem,
                           std::stringstream &assembly) {
  auto funcs = std::static_pointer_cast<scasm::scasm_function>(elem);
  if (funcs->is_global()) {
    assembly << "\t.globl ";
    assembly << ARCHPREFIX << funcs->get_name() << "\n";
  }

  assembly << "\t.text\n";
  assembly << ARCHPREFIX << funcs->get_name() << ":\n";
  assembly << "\tpushq %rbp\n";
  assembly << "\tmovq %rsp, %rbp\n";
  for (auto instr : funcs->get_instructions()) {
    if (instr->get_type() == scasm::instruction_type::CALL) {
      std::string funcName = instr->get_src()->get_identifier();
#ifdef __APPLE__
      assembly << "\tcall " << "_" << funcName << "\n";
#else
      if (backendSymbolTable[funcName].isDefined) {
        assembly << "\tcall " << funcName << "\n";
      } else {
        assembly << "\tcall " << funcName << "@PLT" << "\n";
      }
#endif
    } else if (instr->get_type() == scasm::instruction_type::PUSH) {
      assembly << "\tpush";
      POSTFIX_ASM_TYPE(instr->get_asm_type());
      assembly << " ";
      CODEGEN_SRC(instr->get_asm_type());
      assembly << "\n";
    } else if (instr->get_type() == scasm::instruction_type::POP) {
      assembly << "\tpop";
      POSTFIX_ASM_TYPE(instr->get_asm_type());
      assembly << " ";
      CODEGEN_SRC(instr->get_asm_type());
      assembly << "\n";
    } else if (instr->get_type() == scasm::instruction_type::RET) {
      assembly << "\tmovq %rbp, %rsp\n";
      assembly << "\tpopq %rbp\n";
      assembly << "\tret\n";
    } else if (instr->get_type() == scasm::instruction_type::MOV) {
      assembly << "\tmov";
      POSTFIX_ASM_TYPE(instr->get_asm_type());
      assembly << " ";
      CODEGEN_SRC_DST();
      assembly << "\n";
    } else if (instr->get_type() == scasm::instruction_type::UNARY) {
      assembly << "\t";
      assembly << scasm::to_string(instr->get_unop());
      POSTFIX_ASM_TYPE(instr->get_asm_type());
      assembly << " ";
      CODEGEN_DST(instr->get_asm_type());
      assembly << "\n";
    } else if (instr->get_type() == scasm::instruction_type::CDQ) {
      switch (instr->get_asm_type()) {
      case scasm::AssemblyType::QUAD_WORD:
        assembly << "\tcqo\n";
        break;
      case scasm::AssemblyType::LONG_WORD:
        assembly << "\tcdq\n";
        break;
      default:
        break;
      }
    } else if (instr->get_type() == scasm::instruction_type::IDIV) {
      assembly << "\tidiv";
      POSTFIX_ASM_TYPE(instr->get_asm_type());
      assembly << " ";
      CODEGEN_SRC(instr->get_asm_type());
      assembly << "\n";
    } else if (instr->get_type() == scasm::instruction_type::DIV) {
      assembly << "\tdiv";
      POSTFIX_ASM_TYPE(instr->get_asm_type());
      assembly << " ";
      CODEGEN_SRC(instr->get_asm_type());
      assembly << "\n";
    } else if (instr->get_type() == scasm::instruction_type::BINARY) {
      assembly << "\t";
      if (instr->get_asm_type() == scasm::AssemblyType::DOUBLE) {
        if (instr->get_binop() == scasm::Binop::XOR) {
          assembly << "xorpd";
        } else if (instr->get_binop() == scasm::Binop::MUL) {
          assembly << "mulsd";
        } else {
          assembly << scasm::to_string(instr->get_binop());
          POSTFIX_ASM_TYPE(instr->get_asm_type());
        }
      } else {
        assembly << scasm::to_string(instr->get_binop());
        POSTFIX_ASM_TYPE(instr->get_asm_type());
      }
      assembly << " ";
      if (scasm::isShiftBinop(instr->get_binop())) {
        assembly << scasm::to_string(instr->get_src()->get_reg(),
                                     scasm::register_size::BYTE);
        assembly << ", ";
        CODEGEN_DST(instr->get_asm_type());
      } else {
        CODEGEN_SRC_DST();
      }
      assembly << "\n";
    } else if (instr->get_type() == scasm::instruction_type::JMP) {
#ifdef __APPLE__
      assembly << "\tjmp " << "L" << instr->get_src()->get_identifier() << "\n";
#else
      assembly << "\tjmp " << ".L" << instr->get_src()->get_identifier()
               << "\n";
#endif
    } else if (instr->get_type() == scasm::instruction_type::LABEL) {
#ifdef __APPLE__
      assembly << "L" << instr->get_src()->get_identifier() << ":\n";
#else
      assembly << ".L" << instr->get_src()->get_identifier() << ":\n";
#endif
    } else if (instr->get_type() == scasm::instruction_type::CMP) {
      if (instr->get_asm_type() == scasm::AssemblyType::DOUBLE) {
        assembly << "\tcomi";
      } else {
        assembly << "\tcmp";
      }
      POSTFIX_ASM_TYPE(instr->get_asm_type());
      assembly << " ";
      CODEGEN_SRC_DST();
      assembly << "\n";
    } else if (instr->get_type() == scasm::instruction_type::JMPCC) {
      assembly << "\tj";
      assembly << scasm::to_string(instr->get_src()->get_cond()) << " ";
#ifdef __APPLE__
      assembly << "L" << instr->get_dst()->get_identifier() << "\n";
#else
      assembly << ".L" << instr->get_dst()->get_identifier() << "\n";
#endif
    } else if (instr->get_type() == scasm::instruction_type::SETCC) {
      assembly << "\tset";
      assembly << scasm::to_string(instr->get_src()->get_cond()) << " ";
      CODEGEN_DST(instr->get_asm_type());
      assembly << "\n";
    } else if (instr->get_type() == scasm::instruction_type::MOVSX) {
      assembly << "\tmovs";
      POSTFIX_ASM_TYPE(instr->get_asm_type());
      POSTFIX_ASM_TYPE(instr->get_dst_type());
      assembly << " ";
      CODEGEN_SRC(instr->get_asm_type());
      assembly << ", ";
      CODEGEN_DST(instr->get_dst_type());
      assembly << "\n";
    } else if (instr->get_type() == scasm::instruction_type::MOVZX) {
      assembly << "\tmovz";
      POSTFIX_ASM_TYPE(instr->get_asm_type());
      POSTFIX_ASM_TYPE(instr->get_dst_type());
      assembly << " ";
      CODEGEN_SRC(instr->get_asm_type());
      assembly << ", ";
      CODEGEN_DST(instr->get_dst_type());
      assembly << "\n";
    } else if (instr->get_type() == scasm::instruction_type::CVTSI2SD) {
      assembly << "\tcvtsi2sd";
      POSTFIX_ASM_TYPE(instr->get_asm_type());
      assembly << " ";
      CODEGEN_SRC_DST();
      assembly << "\n";
    } else if (instr->get_type() == scasm::instruction_type::CVTTS2DI) {
      assembly << "\tcvttsd2si";
      POSTFIX_ASM_TYPE(instr->get_asm_type());
      assembly << " ";
      CODEGEN_SRC_DST();
      assembly << "\n";
    } else if (instr->get_type() == scasm::instruction_type::LEA) {
      assembly << "\tlea";
      POSTFIX_ASM_TYPE(instr->get_asm_type());
      assembly << " ";
      CODEGEN_SRC_DST();
      assembly << "\n";
    }
  }
}

void Codegen::asm_gen_static_variable(
    std::shared_ptr<scasm::scasm_top_level> elem, std::stringstream &assembly) {
  /* DATA */
  auto vars = std::static_pointer_cast<scasm::scasm_static_variable>(elem);
  auto init = vars->get_init();
  auto varName = vars->get_name();

  auto symInfo = backendSymbolTable[varName];
  auto varType = symInfo.asmType;
  auto alignement = symInfo.alignment;

  if (vars->is_global()) {
    assembly << "\t.globl ";
    assembly << ARCHPREFIX << varName << "\n";
  }

  bool InDataSection = false;
  if (symInfo.asmType == scasm::AssemblyType::BYTE_ARRAY) {
    InDataSection =
        !(init.size() == 1 and init[0].get_type() == constant::Type::ZERO);
  } else {
    InDataSection = (!init.empty()) or (varType == scasm::AssemblyType::DOUBLE);
  }

  if (InDataSection) {
    assembly << "\t.data\n";
  } else {
    assembly << "\t.bss\n";
  }

#ifdef __APPLE__
  assembly << "\t.balign " + std::to_string(alignement) + '\n';
#else
  assembly << "\t.align " + std::to_string(alignement) + '\n';
#endif
  assembly << ARCHPREFIX << varName << ":\n";

  if (InDataSection) {
    if (varType == scasm::AssemblyType::QUAD_WORD) {
      assembly << "\t.quad ";
      assembly << init[0] << "\n";
    } else if (varType == scasm::AssemblyType::LONG_WORD) {
      assembly << "\t.long ";
      assembly << init[0] << "\n";
    } else if (varType == scasm::AssemblyType::DOUBLE) {
      assembly << "\t.quad ";
      assembly << init[0].get_value().l << "\n";
    } else if (varType == scasm::AssemblyType::BYTE) {
      assembly << "\t.byte ";
      assembly << init[0] << "\n";
    } else if (varType == scasm::AssemblyType::BYTE_ARRAY) {
      for (auto it : init) {
        if (it.get_type() == constant::Type::ZERO) {
          assembly << "\t.zero " << it.get_value().ul << '\n';
        } else if (it.get_type() == constant::Type::INT or
                   it.get_type() == constant::Type::UINT) {
          assembly << "\t.long " << it << '\n';
        } else if (it.get_type() == constant::Type::LONG or
                   it.get_type() == constant::Type::ULONG) {
          assembly << "\t.quad " << it << '\n';
        } else if (it.get_type() == constant::Type::DOUBLE) {
          assembly << "\t.quad " << it.get_value().l << '\n';
        } else if (it.get_type() == constant::Type::CHAR or
                   it.get_type() == constant::Type::UCHAR) {
          assembly << "\t.byte " << it << '\n';
        }
      }
    }
  } else {
    if (varType == scasm::AssemblyType::BYTE_ARRAY) {
      assembly << "\t.zero " + std::to_string(symInfo.size) + '\n';
    }
    assembly << "\t.zero " + std::to_string(alignement) + '\n';
  }
}

void Codegen::asm_gen_static_constant(
    std::shared_ptr<scasm::scasm_top_level> elem, std::stringstream &assembly) {
  auto vars = std::static_pointer_cast<scasm::scasm_static_constant>(elem);
  auto varType = backendSymbolTable[vars->get_name()].asmType;
#ifdef __APPLE__
  if (varType == scasm::AssemblyType::BYTE_ARRAY) {
    assembly << "\t.cstring\n";
  } else {
    assembly << "\t.literal" + std::to_string(vars->get_alignment()) + '\n';
    assembly << "\t.balign " + std::to_string(vars->get_alignment()) + '\n';
  }
#else
  assembly << "\t.section .rodata\n";
  assembly << "\t.align " + std::to_string(vars->get_alignment()) + '\n';
#endif
  assembly << ARCHPREFIX << vars->get_name() << ":\n";
  if (varType == scasm::AssemblyType::QUAD_WORD) {
    assembly << "\t.quad ";
    assembly << vars->get_init() << "\n";
  } else if (varType == scasm::AssemblyType::LONG_WORD) {
    assembly << "\t.long ";
    assembly << vars->get_init() << "\n";
  } else if (varType == scasm::AssemblyType::DOUBLE) {
    assembly << "\t.quad ";
    assembly << vars->get_init().get_value().l << "\n";
  } else if (varType == scasm::AssemblyType::BYTE) {
    assembly << "\t.byte ";
    assembly << vars->get_init() << "\n";
  } else if (varType == scasm::AssemblyType::BYTE_ARRAY) {
    assembly << "\t.asciz ";
    assembly << vars->get_init() << "\n";
  }
#ifdef __APPLE__
  if (vars->get_alignment() == 16)
    assembly << "\t.quad 0\n";
#endif
}

void Codegen::codegen() {
  // ###########################
  gen_scasm();
  allocate_registers();
  fix_pseudo_registers();
  fix_instructions();
  // ###########################
  std::stringstream assembly;

  for (auto elem : scasm.get_elems()) {
    if (elem->get_type() == scasm::scasm_top_level_type::STATIC_VARIABLE) {
      asm_gen_static_variable(elem, assembly);
    } else if (elem->get_type() ==
               scasm::scasm_top_level_type::STATIC_CONSTANT) {
      asm_gen_static_constant(elem, assembly);
    } else if (elem->get_type() == scasm::scasm_top_level_type::FUNCTION) {
      asm_gen_func(elem, assembly);
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
