#pragma once

namespace scarlet {
namespace ast { 
enum class ElemType {
  NONE = 0,
  DERIVED = -1,
  POINTER = -2,
  INT = -3,
  LONG = -4,
  ULONG = -5,
  UINT = -6,
  DOUBLE = -7,
  CHAR = -8,
  UCHAR = -9,
  VOID = -10,
  STRUCT = -11,
};
}
}