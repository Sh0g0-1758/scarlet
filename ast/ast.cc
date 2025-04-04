#include "ast.hh"

namespace scarlet {
namespace ast {
ElemType constTypeToElemType(constant::Type t) {
  switch (t) {
  case constant::Type::INT:
    return ast::ElemType::INT;
  case constant::Type::LONG:
    return ast::ElemType::LONG;
  case constant::Type::UINT:
    return ast::ElemType::UINT;
  case constant::Type::ULONG:
    return ast::ElemType::ULONG;
  case constant::Type::DOUBLE:
    return ast::ElemType::DOUBLE;
  case constant::Type::NONE:
    return ast::ElemType::NONE;
  }
  return ast::ElemType::NONE;
}

constant::Type elemTypeToConstType(ast::ElemType t) {
  switch (t) {
  case ast::ElemType::INT:
    return constant::Type::INT;
  case ast::ElemType::LONG:
    return constant::Type::LONG;
  case ast::ElemType::UINT:
    return constant::Type::UINT;
  case ast::ElemType::ULONG:
    return constant::Type::ULONG;
  case ast::ElemType::DOUBLE:
    return constant::Type::DOUBLE;
  // TODO: FIXME, maybe when elemType is derived/pointer, constant type should
  // be something different ?
  case ast::ElemType::DERIVED:
  case ast::ElemType::POINTER:
  case ast::ElemType::NONE:
    return constant::Type::NONE;
  }
  return constant::Type::NONE;
}

std::string to_string(ast::ElemType type) {
  switch (type) {
  case ast::ElemType::INT:
    return "int";
  case ast::ElemType::LONG:
    return "long";
  case ast::ElemType::UINT:
    return "unsigned int";
  case ast::ElemType::ULONG:
    return "unsigned long";
  case ast::ElemType::DOUBLE:
    return "double";
  case ast::ElemType::DERIVED:
    return "derived";
  case ast::ElemType::POINTER:
    return "pointer";
  case ast::ElemType::NONE:
    return "";
  }
  UNREACHABLE();
}

std::string to_string(ast::SpecifierType type) {
  switch (type) {
  case ast::SpecifierType::STATIC:
    return "static";
  case ast::SpecifierType::EXTERN:
    return "extern";
  case ast::SpecifierType::NONE:
    return "none";
  }
  UNREACHABLE();
}

int getSizeType(ast::ElemType type) {
  if (type == ast::ElemType::INT || type == ast::ElemType::UINT)
    return 4;
  else if (type == ast::ElemType::LONG || type == ast::ElemType::ULONG)
    return 8;
  else if (type == ast::ElemType::DOUBLE)
    return 8;
  else
    return -1;
}

ast::ElemType getParentType(ast::ElemType left, ast::ElemType right) {
  if (left == right)
    return left;
  else if (left == ast::ElemType::DOUBLE or right == ast::ElemType::DOUBLE)
    return ast::ElemType::DOUBLE;
  else if (getSizeType(left) == getSizeType(right)) {
    if (left == ast::ElemType::UINT || left == ast::ElemType::ULONG)
      return left;
    else
      return right;
  } else if (getSizeType(left) > getSizeType(right))
    return left;
  else
    return right;
}

#define CASTCONST(c, ret, t, T)                                                \
  switch (c.get_type()) {                                                      \
  case constant::Type::INT:                                                    \
    ret.set_value({.t = static_cast<T>(c.get_value().i)});                     \
    break;                                                                     \
  case constant::Type::LONG:                                                   \
    ret.set_value({.t = static_cast<T>(c.get_value().l)});                     \
    break;                                                                     \
  case constant::Type::UINT:                                                   \
    ret.set_value({.t = static_cast<T>(c.get_value().ui)});                    \
    break;                                                                     \
  case constant::Type::ULONG:                                                  \
    ret.set_value({.t = static_cast<T>(c.get_value().ul)});                    \
    break;                                                                     \
  case constant::Type::DOUBLE:                                                 \
    ret.set_value({.t = static_cast<T>(c.get_value().d)});                     \
    break;                                                                     \
  case constant::Type::NONE:                                                   \
    break;                                                                     \
  }

constant::Constant castConstToVal(constant::Constant c, ast::ElemType type) {
  if (ast::constTypeToElemType(c.get_type()) == type) {
    return c;
  }

  constant::Constant ret;

  switch (type) {
  case ast::ElemType::INT: {
    ret.set_type(constant::Type::INT);
    CASTCONST(c, ret, i, int);
  } break;
  case ast::ElemType::LONG: {
    ret.set_type(constant::Type::LONG);
    CASTCONST(c, ret, l, long);
  } break;
  case ast::ElemType::UINT: {
    ret.set_type(constant::Type::UINT);
    CASTCONST(c, ret, ui, unsigned int);
  } break;
  case ast::ElemType::ULONG: {
    ret.set_type(constant::Type::ULONG);
    CASTCONST(c, ret, ul, unsigned long);
  } break;
  case ast::ElemType::DOUBLE: {
    ret.set_type(constant::Type::DOUBLE);
    CASTCONST(c, ret, d, double);
  } break;
  // TODO: FIXME
  case ast::ElemType::DERIVED:
  case ast::ElemType::POINTER:
  case ast::ElemType::NONE:
    UNREACHABLE();
  }

  return ret;
}
} // namespace ast
} // namespace scarlet
