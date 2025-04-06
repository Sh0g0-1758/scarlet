#include "scar.hh"

namespace scarlet {
namespace scar {

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
  case scar::instruction_type::CALL:
    return "Call";
  case scar::instruction_type::SIGN_EXTEND:
    return "SignExtend";
  case scar::instruction_type::TRUNCATE:
    return "Truncate";
  case scar::instruction_type::ZERO_EXTEND:
    return "ZeroExtend";
  case scar::instruction_type::DOUBLE_TO_INT:
    return "DoubleToInt";
  case scar::instruction_type::DOUBLE_TO_UINT:
    return "DoubleToUint";
  case scar::instruction_type::INT_TO_DOUBLE:
    return "IntToDouble";
  case scar::instruction_type::UINT_TO_DOUBLE:
    return "UintToDouble";
  case scar::instruction_type::LOAD:
    return "Load";
  case scar::instruction_type::STORE:
    return "Store";
  case scar::instruction_type::GET_ADDRESS:
    return "GetAddress";
  }
  UNREACHABLE()
}

std::string to_string(scar::val_type type) {
  switch (type) {
  case scar::val_type::CONSTANT:
    return "Constant";
  case scar::val_type::VAR:
    return "Var";
  case scar::val_type::LABEL:
    return "Label";
  }
  UNREACHABLE()
}

} // namespace scar
} // namespace scarlet
