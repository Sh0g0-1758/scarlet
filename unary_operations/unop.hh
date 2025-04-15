#pragma once

#include <string>
namespace scarlet {
namespace unop {
enum class UNOP {
  UNKNOWN,
  COMPLEMENT,
  NEGATE,
  NOT,
  PREINCREMENT,
  PREDECREMENT,
  POSTINCREMENT,
  POSTDECREMENT,
  DEREFERENCE,
  ADDROF,
  DECAY,
  SIZEOF
};
std::string to_string(UNOP unop);
bool is_incr_decr(UNOP unop);
} // namespace unop
} // namespace scarlet
