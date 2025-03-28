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
  default:
    return ast::ElemType::NONE;
  }
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
  default:
    return constant::Type::NONE;
  }
}
} // namespace ast
} // namespace scarlet
