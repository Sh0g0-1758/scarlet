#include "unop.hh"
namespace scarlet {
namespace unop {

std::string to_string(UNOP unop) {
  switch (unop) {
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
} // namespace unop
} // namespace scarlet
