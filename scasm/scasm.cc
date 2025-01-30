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
  case binop::BINOP::UNKNOWN:
  case binop::BINOP::DIV:
  case binop::BINOP::MOD:
    __builtin_unreachable();
  }
  __builtin_unreachable();
}

} // namespace scasm
} // namespace scarlet
