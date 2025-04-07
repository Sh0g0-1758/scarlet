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
  case LESSTHAN:
    return "LESSTHAN";
  case GREATERTHAN:
    return "GREATERTHAN";
  case LESSTHANEQUAL:
    return "LESSTHANEQUAL";
  case GREATERTHANEQUAL:
    return "GREATERTHANEQUAL";
  case ASSIGN:
    return "ASSIGN";
  case COMPOUND_DIFFERENCE:
    return "COMPOUND_DIFFERENCE";
  case COMPOUND_SUM:
    return "COMPOUND_SUM";
  case COMPOUND_DIVISION:
    return "COMPOUND_DIVISION";
  case COMPOUND_PRODUCT:
    return "COMPOUND_PRODUCT";
  case COMPOUND_REMAINDER:
    return "COMPOUND_REMAINDER";
  case COMPOUND_AND:
    return "COMPOUND_AND";
  case COMPOUND_OR:
    return "COMPOUND_OR";
  case COMPOUND_XOR:
    return "COMPOUND_XOR";
  case COMPOUND_LEFTSHIFT:
    return "COMPOUND_LEFTSHIFT";
  case COMPOUND_RIGHTSHIFT:
    return "COMPOUND_RIGHTSHIFT";
  case TERNARY:
    return "TERNARY";
  default:
    return "UNKNOWN";
  }
}

bool short_circuit(BINOP binop) {
  return binop == LAND or binop == LOR or binop == TERNARY;
}

bool is_relational(BINOP binop) {
  return binop == EQUAL or binop == NOTEQUAL or binop == LESSTHAN or
         binop == GREATERTHAN or binop == LESSTHANEQUAL or
         binop == GREATERTHANEQUAL;
}
bool is_compound(BINOP bin_op) {
  if (bin_op == BINOP::COMPOUND_DIFFERENCE or
      bin_op == BINOP::COMPOUND_DIVISION or bin_op == BINOP::COMPOUND_PRODUCT or
      bin_op == BINOP::COMPOUND_REMAINDER or bin_op == BINOP::COMPOUND_SUM or
      bin_op == BINOP::COMPOUND_AND or bin_op == BINOP::COMPOUND_OR or
      bin_op == BINOP::COMPOUND_LEFTSHIFT or
      bin_op == BINOP::COMPOUND_RIGHTSHIFT or bin_op == BINOP::COMPOUND_XOR) {
    return true;
  }
  return false;
}

binop::BINOP compound_to_base(BINOP binop) {
  if (binop == BINOP::COMPOUND_DIFFERENCE) {
    return BINOP::SUB;
  } else if (binop == BINOP::COMPOUND_DIVISION) {
    return BINOP::DIV;
  } else if (binop == BINOP::COMPOUND_PRODUCT) {
    return BINOP::MUL;
  } else if (binop == BINOP::COMPOUND_REMAINDER) {
    return BINOP::MOD;
  } else if (binop == BINOP::COMPOUND_SUM) {
    return BINOP::ADD;
  } else if (binop == BINOP::COMPOUND_AND) {
    return BINOP::AAND;
  } else if (binop == BINOP::COMPOUND_OR) {
    return BINOP::AOR;
  } else if (binop == BINOP::COMPOUND_XOR) {
    return BINOP::XOR;
  } else if (binop == BINOP::COMPOUND_LEFTSHIFT) {
    return BINOP::LEFT_SHIFT;
  } else if (binop == BINOP::COMPOUND_RIGHTSHIFT) {
    return BINOP::RIGHT_SHIFT;
  }
  return binop;
}
} // namespace binop
} // namespace scarlet
