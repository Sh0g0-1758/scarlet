#include "scasm.hh"

namespace scarlet {
namespace scasm {

Unop scar_unop_to_scasm_unop(unop::UNOP unop) {
  switch (unop) {
  case unop::UNOP::NEGATE:
    return Unop::NEG;
  case unop::UNOP::COMPLEMENT:
    return Unop::NOT;
  case unop::UNOP::UNKNOWN:
    __builtin_unreachable();
  }
  __builtin_unreachable();
}

Binop scar_binop_to_scasm_binop(binop::BINOP binop) {
  switch (binop) {
  case binop::BINOP::ADD:
    return Binop::ADD;
  case binop::BINOP::SUB:
    return Binop::SUB;
  case binop::BINOP::MUL:
    return Binop::MUL;
  case binop::BINOP::AND:
    return Binop::AND;
  case binop::BINOP::OR:
    return Binop::OR;
  case binop::BINOP::XOR:
    return Binop::XOR;
  case binop::BINOP::LEFT_SHIFT:
    return Binop::LEFT_SHIFT;
  case binop::BINOP::RIGHT_SHIFT:
    return Binop::RIGHT_SHIFT;
  case binop::BINOP::UNKNOWN:
  case binop::BINOP::DIV:
  case binop::BINOP::MOD:
    __builtin_unreachable();
  }
  __builtin_unreachable();
}

std::string to_string(register_type reg) {
  switch (reg) {
  case register_type::AX:
    return "%eax";
  case register_type::DX:
    return "%edx";
  case register_type::CX:
    return "%ecx";
  case register_type::CL:
    return "%cl";
  case register_type::R10:
    return "%r10d";
  case register_type::R11:
    return "%r11d";
  case register_type::UNKNOWN:
    __builtin_unreachable();
  }
  __builtin_unreachable();
}

std::string to_string(Unop unop) {
  switch (unop) {
  case Unop::NEG:
    return "negl";
  case Unop::NOT:
    return "notl";
  case Unop::UNKNOWN:
    __builtin_unreachable();
  }
  __builtin_unreachable();
}

std::string to_string(Binop binop) {
  switch (binop) {
  case Binop::ADD:
    return "addl";
  case Binop::SUB:
    return "subl";
  case Binop::MUL:
    return "imull";
  case Binop::AND:
    return "andl";
  case Binop::OR:
    return "orl";
  case Binop::XOR:
    return "xorl";
  case Binop::LEFT_SHIFT:
    return "sall";
  case Binop::RIGHT_SHIFT:
    return "sarl";
  case Binop::UNKNOWN:
    __builtin_unreachable();
  }
  __builtin_unreachable();
}

} // namespace scasm
} // namespace scarlet
