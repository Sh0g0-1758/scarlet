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
  LESSTHAN,
  GREATERTHAN,
  LESSTHANEQUAL,
  GREATERTHANEQUAL,
  COMPOUND_SUM,
  COMPOUND_DIFFERENCE,
  COMPOUND_DIVISION,
  COMPOUND_PRODUCT,
  COMPOUND_REMAINDER,
  ASSIGN
};

std::string to_string(BINOP binop);
bool short_circuit(BINOP binop);
bool is_relational(BINOP binop);
} // namespace binop
} // namespace scarlet
