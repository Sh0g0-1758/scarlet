#include "scasm.hh"

namespace scarlet {
namespace scasm {

#define UNREACHABLE()                                                          \
  std::cerr << "Unreachable code reached in " << __FILE__ << " at line "       \
            << __LINE__ << std::endl;                                          \
  __builtin_unreachable();

Unop scar_unop_to_scasm_unop(unop::UNOP unop) {
  switch (unop) {
  case unop::UNOP::NEGATE:
    return Unop::NEG;
  case unop::UNOP::COMPLEMENT:
    return Unop::ANOT;
  case unop::UNOP::NOT:
    return Unop::LNOT;
    // INCREMENT AND DECREMENT ARE HANDLED IN SCAR AS A BINOP
  case unop::UNOP::INCREMENT:
  case unop::UNOP::DECREMENT:
  case unop::UNOP::PINCREMENT:
  case unop::UNOP::PDECREMENT:
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
  case binop::BINOP::DIV:
  case binop::BINOP::MOD:
  case binop::BINOP::UNKNOWN:
  case binop::BINOP::TERNARY:
    UNREACHABLE()
  }
  UNREACHABLE()
}

// If small is set, return the 8-bit version of the register
// else return the 32-bit version of the register
std::string to_string(register_type reg, bool small) {
  switch (reg) {
  case register_type::AX:
    if (small)
      return "%al";
    return "%eax";
  case register_type::DX:
    if (small)
      return "%dl";
    return "%edx";
  case register_type::CX:
    if (small)
      return "%cl";
    return "%ecx";
  case register_type::CL:
    return "%cl";
  case register_type::R10:
    if (small)
      return "%r10b";
    return "%r10d";
  case register_type::R11:
    if (small)
      return "%r11b";
    return "%r11d";
  case register_type::UNKNOWN:
    UNREACHABLE()
  }
  UNREACHABLE()
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
  case cond_code::UNKNOWN:
    UNREACHABLE()
  }
  UNREACHABLE()
}

std::string to_string(Unop unop) {
  switch (unop) {
  case Unop::NEG:
    return "negl";
  case Unop::ANOT:
    return "notl";
  case Unop::UNKNOWN:
  case Unop::LNOT:
    UNREACHABLE()
  }
  UNREACHABLE()
}

std::string to_string(Binop binop) {
  switch (binop) {
  case Binop::ADD:
    return "addl";
  case Binop::SUB:
    return "subl";
  case Binop::MUL:
    return "imull";
  case Binop::AAND:
    return "andl";
  case Binop::AOR:
    return "orl";
  case Binop::XOR:
    return "xorl";
  case Binop::LEFT_SHIFT:
    return "sall";
  case Binop::RIGHT_SHIFT:
    return "sarl";
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

} // namespace scasm
} // namespace scarlet
