#include "scar.hh"

namespace scarlet {
namespace scar {

std::string to_string(scar::scar_instruction_type type) {
  switch (type) {
  case scar::scar_instruction_type::RETURN:
    return "Return";
  case scar::scar_instruction_type::UNARY:
    return "Unary";
  case scar::scar_instruction_type::BINARY:
    return "Binary";
  }
  __builtin_unreachable();
}

std::string to_string(scar::scar_val_type type) {
  switch (type) {
  case scar::scar_val_type::CONSTANT:
    return "Constant";
  case scar::scar_val_type::VAR:
    return "Var";
  }
  __builtin_unreachable();
}

} // namespace scar
} // namespace scarlet