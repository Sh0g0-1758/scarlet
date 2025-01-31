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
  AAND,
  AOR,
  XOR,
  LEFT_SHIFT,
  RIGHT_SHIFT,
  LAND,
  LOR,
  EQUAL,
  NOTEQUAL,
  LESSER,
  GREATER,
  LESSEREQUAL,
  GREATEREQUAL
};

std::string to_string(BINOP binop);
} // namespace binop
} // namespace scarlet
