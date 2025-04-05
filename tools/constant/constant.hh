#pragma once

#include <iostream>
#include <tools/macros/macros.hh>

namespace scarlet {
namespace constant {
union Value {
  int i;
  long l;
  double d;
  unsigned int ui;
  unsigned long ul;
};

enum class Type {
  NONE,
  INT,
  LONG,
  DOUBLE,
  UINT,
  ULONG,
  /* ZERO is a special type that can be used to store how many bytes need to be
     zeroed out. The value is stored as an unsigned long */
  ZERO,
};

class Constant {
private:
  Value value{};
  Type type = Type::NONE;

public:
  Value get_value() { return value; }
  void set_value(Value value) { this->value = value; }
  Type get_type() { return type; }
  void set_type(Type type) { this->type = type; }
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
    case Type::ZERO:
      os << constant.value.ul;
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
