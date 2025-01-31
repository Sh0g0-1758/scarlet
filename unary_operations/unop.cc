#include "unop.hh"
namespace scarlet {
namespace unop {

std::string to_string(UNOP unop) {
  if (unop == UNOP::COMPLEMENT) {
    return "Complement";
  }
  if (unop == UNOP::NEGATE) {
    return "Negate";
  }
  if (unop == UNOP::NOT) {
    return "Not";
  }
  return "";
}
} // namespace unop
} // namespace scarlet
