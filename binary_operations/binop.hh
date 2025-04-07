#pragma once

#include <map>
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
  LOGICAL_LEFT_SHIFT,
  LOGICAL_RIGHT_SHIFT,
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
  COMPOUND_AND,
  COMPOUND_OR,
  COMPOUND_XOR,
  COMPOUND_LEFTSHIFT,
  COMPOUND_RIGHTSHIFT,
  COMPOUND_LOGICAL_LEFTSHIFT,
  COMPOUND_LOGICAL_RIGHTSHIFT,
  ASSIGN,
  TERNARY
};

std::string to_string(BINOP binop);
bool short_circuit(BINOP binop);
bool is_relational(BINOP binop);
bool is_compound(BINOP binop);
binop::BINOP compound_to_base(BINOP binop);
} // namespace binop
} // namespace scarlet
