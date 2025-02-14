#pragma once

#include <string>
namespace scarlet {
namespace unop {
// NOTE: INCREMENT and DECREMENT are pre operations while PINCREMENT and
// PDECREMENT are post operations
enum class UNOP {
  UNKNOWN,
  COMPLEMENT,
  NEGATE,
  NOT,
  PREINCREMENT,
  PREDECREMENT,
  POSTINCREMENT,
  POSTDECREMENT
};
std::string to_string(UNOP unop);
} // namespace unop
} // namespace scarlet
