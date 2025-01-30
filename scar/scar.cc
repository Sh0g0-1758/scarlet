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
  }
  __builtin_unreachable();
}

std::string to_string(scar::val_type type) {
  switch (type) {
  case scar::val_type::CONSTANT:
    return "Constant";
  case scar::val_type::VAR:
    return "Var";
  }
  __builtin_unreachable();
}

} // namespace scar
} // namespace scarlet