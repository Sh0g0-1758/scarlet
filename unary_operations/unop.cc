#include "unop.hh"
namespace scarlet {
namespace unop {

std::string to_string(UNOP unop) {
  switch (unop) {
  case UNOP::DEREFERENCE:
    return "Dereference";
  case UNOP::ADDROF:
    return "AddressOf";
  case UNOP::PREINCREMENT:
    return "PreIncrement";
  case UNOP::PREDECREMENT:
    return "PreDecrement";
  case UNOP::POSTINCREMENT:
    return "PostIncrement";
  case UNOP::POSTDECREMENT:
    return "PostDecrement";
  case UNOP::COMPLEMENT:
    return "Complement";
  case UNOP::NEGATE:
    return "Negate";
  case UNOP::NOT:
    return "Not";
  case UNOP::UNKNOWN:
    return "";
  }
  return "";
}

bool is_incr_decr(UNOP unop) {
  return unop == UNOP::PREINCREMENT or unop == UNOP::PREDECREMENT or
         unop == UNOP::POSTINCREMENT or unop == UNOP::POSTDECREMENT;
}

} // namespace unop
} // namespace scarlet
