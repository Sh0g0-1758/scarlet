#pragma once

#include <string>

namespace scarlet {
namespace binop {
enum BINOP {
  UNKNOWN,
  ADD,
  SUB,
  MUL,
  DIV,
  MOD,
  AND,
  OR,
  XOR,
  LEFT_SHIFT,
  RIGHT_SHIFT
};

std::string to_string(BINOP binop);
} // namespace binop
} // namespace scarlet
