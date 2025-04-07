#include "scasm.hh"

namespace scarlet {
namespace scasm {

Unop scar_unop_to_scasm_unop(unop::UNOP unop) {
  switch (unop) {
  case unop::UNOP::NEGATE:
    return Unop::NEG;
  case unop::UNOP::COMPLEMENT:
    return Unop::ANOT;
  case unop::UNOP::NOT:
    return Unop::LNOT;
  // INCREMENT AND DECREMENT ARE HANDLED IN SCAR AS A BINOP
  case unop::UNOP::PREINCREMENT:
  case unop::UNOP::PREDECREMENT:
  case unop::UNOP::POSTINCREMENT:
  case unop::UNOP::POSTDECREMENT:
  // TODO: Handle ADDROF and DEREFERENCE
  case unop::UNOP::ADDROF:
  case unop::UNOP::DEREFERENCE:
  case unop::UNOP::UNKNOWN:
    UNREACHABLE()
  }
  UNREACHABLE()
}

Binop scar_binop_to_scasm_binop(binop::BINOP binop) {
  switch (binop) {
  case binop::BINOP::ADD:
    return Binop::ADD;
  case binop::BINOP::SUB:
    return Binop::SUB;
  case binop::BINOP::MUL:
    return Binop::MUL;
  case binop::BINOP::AAND:
    return Binop::AAND;
  case binop::BINOP::AOR:
    return Binop::AOR;
  case binop::BINOP::XOR:
    return Binop::XOR;
  case binop::BINOP::LEFT_SHIFT:
    return Binop::LEFT_SHIFT;
  case binop::BINOP::RIGHT_SHIFT:
    return Binop::RIGHT_SHIFT;
  case binop::BINOP::LOGICAL_LEFT_SHIFT:
    return Binop::LOGICAL_LEFT_SHIFT;
  case binop::BINOP::LOGICAL_RIGHT_SHIFT:
    return Binop::LOGICAL_RIGHT_SHIFT;
  case binop::BINOP::LAND:
    return Binop::LAND;
  case binop::BINOP::LOR:
    return Binop::LOR;
  case binop::BINOP::EQUAL:
    return Binop::EQUAL;
  case binop::BINOP::NOTEQUAL:
    return Binop::NOTEQUAL;
  case binop::BINOP::LESSTHAN:
    return Binop::LESSTHAN;
  case binop::BINOP::GREATERTHAN:
    return Binop::GREATERTHAN;
  case binop::BINOP::LESSTHANEQUAL:
    return Binop::LESSTHANEQUAL;
  case binop::BINOP::GREATERTHANEQUAL:
    return Binop::GREATERTHANEQUAL;
  case binop::BINOP::ASSIGN:
    return Binop::ASSIGN;
  /* This case will only be called when its a double division */
  case binop::BINOP::DIV:
    return Binop::DIV_DOUBLE;
  case binop::BINOP::COMPOUND_DIFFERENCE:
  case binop::BINOP::COMPOUND_DIVISION:
  case binop::BINOP::COMPOUND_PRODUCT:
  case binop::BINOP::COMPOUND_REMAINDER:
  case binop::BINOP::COMPOUND_SUM:
  case binop::BINOP::COMPOUND_AND:
  case binop::BINOP::COMPOUND_OR:
  case binop::BINOP::COMPOUND_XOR:
  case binop::BINOP::COMPOUND_LEFTSHIFT:
  case binop::BINOP::COMPOUND_RIGHTSHIFT:
  case binop::BINOP::MOD:
  case binop::BINOP::UNKNOWN:
  case binop::BINOP::TERNARY:
    UNREACHABLE()
  }
  UNREACHABLE()
}

// If small is set, return the 8-bit version of the register
// else return the 32-bit version of the register
std::string to_string(register_type reg, register_size size) {
  switch (reg) {
  case register_type::AX:
    switch (size) {
    case register_size::BYTE:
      return "%al";
    case register_size::LWORD:
      return "%eax";
    case register_size::QWORD:
      return "%rax";
    }
    break;

  case register_type::DX:
    switch (size) {
    case register_size::BYTE:
      return "%dl";
    case register_size::LWORD:
      return "%edx";
    case register_size::QWORD:
      return "%rdx";
    }
    break;

  case register_type::CX:
    switch (size) {
    case register_size::BYTE:
      return "%cl";
    case register_size::LWORD:
      return "%ecx";
    case register_size::QWORD:
      return "%rcx";
    }
    break;

  case register_type::DI:
    switch (size) {
    case register_size::BYTE:
      return "%dil";
    case register_size::LWORD:
      return "%edi";
    case register_size::QWORD:
      return "%rdi";
    }
    break;

  case register_type::SI:
    switch (size) {
    case register_size::BYTE:
      return "%sil";
    case register_size::LWORD:
      return "%esi";
    case register_size::QWORD:
      return "%rsi";
    }
    break;

  case register_type::R8:
    switch (size) {
    case register_size::BYTE:
      return "%r8b";
    case register_size::LWORD:
      return "%r8d";
    case register_size::QWORD:
      return "%r8";
    }
    break;

  case register_type::R9:
    switch (size) {
    case register_size::BYTE:
      return "%r9b";
    case register_size::LWORD:
      return "%r9d";
    case register_size::QWORD:
      return "%r9";
    }
    break;

  case register_type::R10:
    switch (size) {
    case register_size::BYTE:
      return "%r10b";
    case register_size::LWORD:
      return "%r10d";
    case register_size::QWORD:
      return "%r10";
    }
    break;

  case register_type::R11:
    switch (size) {
    case register_size::BYTE:
      return "%r11b";
    case register_size::LWORD:
      return "%r11d";
    case register_size::QWORD:
      return "%r11";
    }
    break;

  case register_type::BP:
    return "%rbp";
  case register_type::SP:
    return "%rsp";
  case register_type::CL:
    return "%cl";
  case register_type::XMM0:
    return "%xmm0";
  case register_type::XMM1:
    return "%xmm1";
  case register_type::XMM2:
    return "%xmm2";
  case register_type::XMM3:
    return "%xmm3";
  case register_type::XMM4:
    return "%xmm4";
  case register_type::XMM5:
    return "%xmm5";
  case register_type::XMM6:
    return "%xmm6";
  case register_type::XMM7:
    return "%xmm7";
  case register_type::XMM14:
    return "%xmm14";
  case register_type::XMM15:
    return "%xmm15";
  case register_type::UNKNOWN:
    UNREACHABLE();
  }
  UNREACHABLE();
}

std::string to_string(cond_code code) {
  switch (code) {
  case cond_code::E:
    return "e";
  case cond_code::NE:
    return "ne";
  case cond_code::G:
    return "g";
  case cond_code::GE:
    return "ge";
  case cond_code::L:
    return "l";
  case cond_code::LE:
    return "le";
  case cond_code::A:
    return "a";
  case cond_code::AE:
    return "ae";
  case cond_code::B:
    return "b";
  case cond_code::BE:
    return "be";
  case cond_code::UNKNOWN:
    UNREACHABLE()
  }
  UNREACHABLE()
}

std::string to_string(Unop unop) {
  switch (unop) {
  case Unop::NEG:
    return "neg";
  case Unop::ANOT:
    return "not";
  case Unop::SHR:
    return "shr";
  case Unop::UNKNOWN:
  case Unop::LNOT:
    UNREACHABLE()
  }
  UNREACHABLE()
}

std::string to_string(Binop binop) {
  switch (binop) {
  case Binop::ADD:
    return "add";
  case Binop::SUB:
    return "sub";
  case Binop::MUL:
    return "imul";
  case Binop::AAND:
    return "and";
  case Binop::AOR:
    return "or";
  case Binop::XOR:
    return "xor";
  case Binop::LEFT_SHIFT:
    return "sal";
  case Binop::RIGHT_SHIFT:
    return "sar";
  case Binop::LOGICAL_LEFT_SHIFT:
    return "shl";
  case Binop::LOGICAL_RIGHT_SHIFT:
    return "shr";
  case Binop::DIV_DOUBLE:
    return "div";
  case Binop::UNKNOWN:
  // All relational operators are handled by the cmpl instruction using register
  // flags and results are interpreted from the flags using setcc instruction.
  // Assign instruction in turn is handled by the mov instruction.
  // So, these cases should never be reached.
  case Binop::ASSIGN:
  case Binop::LAND:
  case Binop::LOR:
  case Binop::EQUAL:
  case Binop::NOTEQUAL:
  case Binop::LESSTHAN:
  case Binop::GREATERTHAN:
  case Binop::LESSTHANEQUAL:
  case Binop::GREATERTHANEQUAL:
    UNREACHABLE()
  }
  UNREACHABLE()
}

bool RegIsXMM(register_type reg) {
  switch (reg) {
  case register_type::XMM0:
  case register_type::XMM1:
  case register_type::XMM2:
  case register_type::XMM3:
  case register_type::XMM4:
  case register_type::XMM5:
  case register_type::XMM6:
  case register_type::XMM7:
  case register_type::XMM14:
  case register_type::XMM15:
    return true;
  default:
    return false;
  }
}

} // namespace scasm
} // namespace scarlet
