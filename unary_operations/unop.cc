#include "unop.hh"
namespace scarlet {
namespace unop {

std::string to_string(UNOP unop) {
  switch (unop) {
  case UNOP::INCREMENT:
    return "Increment";
  case UNOP::DECREMENT:
    return "Decrement";
  case UNOP::PINCREMENT:
    return "Post Increment";
  case UNOP::PDECREMENT:
    return "Post Decrement";
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
