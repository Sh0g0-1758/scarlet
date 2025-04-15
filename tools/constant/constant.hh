#pragma once

#include <iostream>
#include <string>
#include <tools/macros/macros.hh>

namespace scarlet {
namespace constant {
union Value {
  char c;
  int i;
  long l;
  double d;
  unsigned char uc;
  unsigned int ui;
  unsigned long ul;
};

enum class Type {
  NONE,
  CHAR,
  UCHAR,
  INT,
  UINT,
  LONG,
  ULONG,
  DOUBLE,
  STRING,
  /* ZERO is a special type that can be used to store how many bytes need to be
     zeroed out. The value is stored as an unsigned long */
  ZERO,
};

class Constant {
private:
  Value value{};
  Type type = Type::NONE;
  // Will only be used when the constant is a string
  std::string s;

public:
  Value get_value() { return value; }
  void set_value(Value value) { this->value = value; }
  Type get_type() { return type; }
  void set_type(Type type) { this->type = type; }
  std::string get_string() { return s; }
  void set_string(std::string s) { this->s = s; }
  bool empty() { return type == Type::NONE; }
  void clear() {
    type = Type::NONE;
    value = {};
  }
  std::string typeToString() {
    switch (type) {
    case Type::INT:
      return "int";
    case Type::LONG:
      return "long";
    case Type::DOUBLE:
      return "double";
    case Type::UINT:
      return "unsigned int";
    case Type::ULONG:
      return "unsigned long";
    case Type::CHAR:
      return "char";
    case Type::UCHAR:
      return "unsigned char";
    case Type::STRING:
      return "string";
    case Type::ZERO:
      return "zero";
    case Type::NONE:
      return "";
    }
    UNREACHABLE();
  }
  bool operator<(const Constant &constant) const {
    if (type != constant.type) {
      return type < constant.type;
    }
    switch (type) {
    case Type::INT:
      return value.i < constant.value.i;
    case Type::LONG:
      return value.l < constant.value.l;
    case Type::DOUBLE:
      return value.d < constant.value.d;
    case Type::UINT:
      return value.ui < constant.value.ui;
    case Type::ULONG:
      return value.ul < constant.value.ul;
    case Type::ZERO:
      return value.ul < constant.value.ul;
    case Type::CHAR:
      return value.c < constant.value.c;
    case Type::UCHAR:
      return value.uc < constant.value.uc;
    case Type::STRING:
      return s < constant.s;
    case Type::NONE:
      UNREACHABLE();
    }
    UNREACHABLE();
  }
  bool operator==(const Constant &constant) const {
    if (type != constant.type) {
      return false;
    }
    switch (type) {
    case Type::INT:
      return value.i == constant.value.i;
    case Type::LONG:
      return value.l == constant.value.l;
    case Type::DOUBLE:
      return value.d == constant.value.d;
    case Type::UINT:
      return value.ui == constant.value.ui;
    case Type::ULONG:
      return value.ul == constant.value.ul;
    case Type::CHAR:
      return value.c == constant.value.c;
    case Type::UCHAR:
      return value.uc == constant.value.uc;
    case Type::STRING:
      return s == constant.s;
    case Type::ZERO:
      return value.ul == constant.value.ul;
    case Type::NONE:
      UNREACHABLE();
    }
    UNREACHABLE();
  }
  friend std::ostream &operator<<(std::ostream &os, const Constant &constant) {
    switch (constant.type) {
    case Type::INT:
      os << constant.value.i;
      break;
    case Type::LONG:
      os << constant.value.l;
      break;
    case Type::DOUBLE:
      os << constant.value.d;
      break;
    case Type::UINT:
      os << constant.value.ui;
      break;
    case Type::ULONG:
      os << constant.value.ul;
      break;
    case Type::CHAR:
      os << (int)constant.value.c;
      break;
    case Type::UCHAR:
      os << (int)constant.value.uc;
      break;
    case Type::STRING: {
      std::string escaped{};
      for (char c : constant.s) {
        switch (c) {
        case '\a':
          escaped += "\\007";
          break;
        case '\n':
          escaped += "\\012";
          break;
        case '\f':
          escaped += "\\014";
          break;
        case '\b':
          escaped += "\\010";
          break;
        case '\r':
          escaped += "\\015";
          break;
        case '\t':
          escaped += "\\011";
          break;
        case '\v':
          escaped += "\\013";
          break;
        case '\'':
          escaped += "\\047";
          break;
        case '\"':
          escaped += "\\042";
          break;
        case '\?':
          escaped += "\\077";
          break;
        case '\\':
          escaped += "\\134";
          break;
        default:
          escaped += c;
          break;
        }
      }
      os << "\"" << escaped << "\"";
    } break;
    case Type::ZERO:
      os << "Zero(" << constant.value.ul << ")";
      break;
    case Type::NONE:
      UNREACHABLE();
      break;
    }
    return os;
  }
};
} // namespace constant
} // namespace scarlet
