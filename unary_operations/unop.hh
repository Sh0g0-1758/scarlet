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
  POSTDECREMENT
};
std::string to_string(UNOP unop);
} // namespace unop
} // namespace scarlet
