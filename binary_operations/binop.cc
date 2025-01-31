#include "binop.hh"

namespace scarlet {
namespace binop {

std::string to_string(BINOP binop) {
  switch (binop) {
  case ADD:
    return "ADD";
  case SUB:
    return "SUBTRACT";
  case MUL:
    return "MULTIPLY";
  case DIV:
    return "DIVIDE";
  case MOD:
    return "MODULO";
  case AAND:
    return "AAND";
  case AOR:
    return "AOR";
  case XOR:
    return "XOR";
  case LEFT_SHIFT:
    return "LEFT_SHIFT";
  case RIGHT_SHIFT:
    return "RIGHT_SHIFT";
  case LAND:
    return "LAND";
  case LOR:
    return "LOR";
  case EQUAL:
    return "EQUAL";
  case NOTEQUAL:
    return "NOTEQUAL";
  case LESSER:
    return "LESSER";
  case GREATER:
    return "GREATER";
  case LESSEREQUAL:
    return "LESSEREQUAL";
  case GREATEREQUAL:
    return "GREATEREQUAL";
  default:
    return "UNKNOWN";
  }
}

} // namespace binop
} // namespace scarlet