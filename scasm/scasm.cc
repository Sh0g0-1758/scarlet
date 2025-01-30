#include "scasm.hh"

namespace scarlet {
namespace scasm {

Unop scar_unop_to_scasm_unop(unop::UNOP unop) {
  switch (unop) {
  case unop::UNOP::NEGATE:
    return Unop::NEG;
  case unop::UNOP::COMPLEMENT:
    return Unop::NOT;
  case unop::UNOP::UNKNOWN:
    __builtin_unreachable();
  }
  __builtin_unreachable();
}

} // namespace scasm
} // namespace scarlet
