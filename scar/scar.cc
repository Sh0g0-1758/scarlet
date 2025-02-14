#include "scar.hh"

namespace scarlet {
namespace scar {

#define UNREACHABLE()                                                          \
  std::cerr << "Unreachable code reached in " << __FILE__ << " at line "       \
            << __LINE__ << std::endl;                                          \
  __builtin_unreachable();

std::string to_string(scar::instruction_type type) {
  switch (type) {
  case scar::instruction_type::RETURN:
    return "Return";
  case scar::instruction_type::UNARY:
    return "Unary";
  case scar::instruction_type::BINARY:
    return "Binary";
  case scar::instruction_type::COPY:
    return "Copy";
  case scar::instruction_type::JUMP:
    return "Jump";
  case scar::instruction_type::JUMP_IF_ZERO:
    return "JumpIfZero";
  case scar::instruction_type::JUMP_IF_NOT_ZERO:
    return "JumpIfNotZero";
  case scar::instruction_type::LABEL:
    return "Label";
  }
  UNREACHABLE()
}

std::string to_string(scar::val_type type) {
  switch (type) {
  case scar::val_type::CONSTANT:
    return "Constant";
  case scar::val_type::VAR:
    return "Var";
  case scar::val_type::UNKNOWN:
    return "";
  }
  UNREACHABLE()
}

} // namespace scar
} // namespace scarlet