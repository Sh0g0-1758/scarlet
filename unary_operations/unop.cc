#include "unop.hh"

namespace Scarlet {
namespace Unop {
std::string to_string_unop(UNOP unop) {
  if (unop == UNOP::COMPLEMENT) {
    return "Complement";
  }
  if (unop == UNOP::NEGATE) {
    return "Negate";
  }
  return "";
}
}
}