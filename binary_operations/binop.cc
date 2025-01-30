#include "binop.hh"

namespace scarlet {
namespace binop {

std::string to_string(BINOP binop) {
  switch (binop) {
  case ADD:
    return "ADD";
  case SUB:
    return "SUBTRACT";
  case MUL:
    return "MULTIPLY";
  case DIV:
    return "DIVIDE";
  case MOD:
    return "MODULO";
  default:
    return "UNKNOWN";
  }
}

} // namespace binop
} // namespace scarlet