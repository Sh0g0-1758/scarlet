#include "ast.hh"

namespace scarlet {
namespace ast {
ElemType constTypeToElemType(constant::Type t) {
  switch (t) {
  case constant::Type::INT:
    return ElemType::INT;
  case constant::Type::LONG:
    return ElemType::LONG;
  case constant::Type::UINT:
    return ElemType::UINT;
  case constant::Type::ULONG:
    return ElemType::ULONG;
  case constant::Type::DOUBLE:
    return ElemType::DOUBLE;
  case constant::Type::NONE:
    return ElemType::NONE;
  }
  return ElemType::NONE;
}

constant::Type elemTypeToConstType(ElemType t) {
  switch (t) {
  case ElemType::INT:
    return constant::Type::INT;
  case ElemType::LONG:
    return constant::Type::LONG;
  case ElemType::UINT:
    return constant::Type::UINT;
  case ElemType::ULONG:
    return constant::Type::ULONG;
  case ElemType::DOUBLE:
    return constant::Type::DOUBLE;
  // TODO: FIXME, maybe when elemType is derived/pointer, constant type should
  // be something different ?
  case ElemType::DERIVED:
  case ElemType::POINTER:
  case ElemType::NONE:
    return constant::Type::NONE;
  }
  return constant::Type::NONE;
}

std::string to_string(ElemType type) {
  switch (type) {
  case ElemType::INT:
    return "int";
  case ElemType::LONG:
    return "long";
  case ElemType::UINT:
    return "unsigned int";
  case ElemType::ULONG:
    return "unsigned long";
  case ElemType::DOUBLE:
    return "double";
  case ElemType::DERIVED:
    return "derived";
  case ElemType::POINTER:
    return "pointer";
  case ElemType::NONE:
    return "";
  }
  UNREACHABLE();
}

std::string to_string(SpecifierType type) {
  switch (type) {
  case SpecifierType::STATIC:
    return "static";
  case SpecifierType::EXTERN:
    return "extern";
  case SpecifierType::NONE:
    return "none";
  }
  UNREACHABLE();
}

int getSizeType(ElemType type) {
  if (type == ElemType::INT || type == ElemType::UINT)
    return 4;
  else if (type == ElemType::LONG || type == ElemType::ULONG)
    return 8;
  else if (type == ElemType::DOUBLE)
    return 8;
  else
    return -1;
}

bool is_const_zero(std::shared_ptr<AST_factor_Node> factor) {
  if (factor != nullptr and factor->get_const_node() != nullptr and
      factor->get_const_node()->get_constant().get_value().i == 0 and
      factor->get_const_node()->get_constant().get_type() !=
          constant::Type::DOUBLE)
    return true;
  return false;
}

bool is_lvalue(std::shared_ptr<AST_factor_Node> factor) {
  if (factor == nullptr)
    return false;
  if (factor->get_identifier_node() != nullptr) {
    if (factor->get_factor_type() == FactorType::FUNCTION_CALL) {
      return false;
    }
    return true;
  }
  if (factor->get_const_node() != nullptr)
    return false;
  if (factor->get_unop_node() != nullptr)
    return factor->get_unop_node()->get_op() == unop::UNOP::DEREFERENCE;
  if (factor->get_exp_node() != nullptr and
      factor->get_exp_node()->get_binop_node() != nullptr)
    return false;
  if (factor->get_cast_type() != ElemType::NONE)
    return false;
  return is_lvalue(factor->get_child()) or
         (factor->get_exp_node() != nullptr
              ? is_lvalue(factor->get_exp_node()->get_factor_node())
              : false);
}

std::pair<ElemType, std::vector<long>>
getParentType(ElemType left, ElemType right, std::vector<long> &leftDerivedType,
              std::vector<long> &rightDerivedType,
              std::shared_ptr<AST_exp_Node> exp) {
  if (left == ElemType::DERIVED or right == ElemType::DERIVED) {
    if (leftDerivedType == rightDerivedType) {
      return {left, leftDerivedType};
    } else {
      std::shared_ptr<AST_factor_Node> leftFactor{};
      bool isTernary =
          (exp->get_binop_node() != nullptr and
           exp->get_binop_node()->get_op() == binop::BINOP::TERNARY);
      if (isTernary)
        leftFactor = (std::static_pointer_cast<AST_ternary_exp_Node>(exp))
                         ->get_middle()
                         ->get_factor_node();
      else
        leftFactor = exp->get_factor_node();
      if (is_const_zero(leftFactor)) {
        return {right, rightDerivedType};
      } else if (is_const_zero(exp->get_right()->get_factor_node())) {
        return {left, leftDerivedType};
      } else {
        return {ElemType::NONE, {}};
      }
    }
  } else {
    if (left == right)
      return {left, {}};
    else if (left == ElemType::DOUBLE or right == ElemType::DOUBLE)
      return {ElemType::DOUBLE, {}};
    else if (getSizeType(left) == getSizeType(right)) {
      if (left == ElemType::UINT || left == ElemType::ULONG)
        return {left, {}};
      else
        return {right, {}};
    } else if (getSizeType(left) > getSizeType(right))
      return {left, {}};
    else
      return {right, {}};
  }
}

std::pair<ElemType, std::vector<long>>
getAssignType(ElemType target, std::vector<long> targetDerived, ElemType src,
              std::vector<long> srcDerived,
              std::shared_ptr<AST_exp_Node> srcExp) {
  if (target == ElemType::DERIVED or src == ElemType::DERIVED) {
    if (targetDerived == srcDerived) {
      return {target, targetDerived};
    } else if (is_const_zero(srcExp->get_factor_node())) {
      return {target, targetDerived};
    } else {
      return {ElemType::NONE, {}};
    }
  } else {
    return {target, targetDerived};
  }
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

constant::Constant castConstToElemType(constant::Constant c, ElemType type) {
  if (constTypeToElemType(c.get_type()) == type) {
    return c;
  }

  constant::Constant ret;

  switch (type) {
  case ElemType::INT: {
    ret.set_type(constant::Type::INT);
    CASTCONST(c, ret, i, int);
  } break;
  case ElemType::LONG: {
    ret.set_type(constant::Type::LONG);
    CASTCONST(c, ret, l, long);
  } break;
  case ElemType::UINT: {
    ret.set_type(constant::Type::UINT);
    CASTCONST(c, ret, ui, unsigned int);
  } break;
  case ElemType::ULONG: {
    ret.set_type(constant::Type::ULONG);
    CASTCONST(c, ret, ul, unsigned long);
  } break;
  case ElemType::DOUBLE: {
    ret.set_type(constant::Type::DOUBLE);
    CASTCONST(c, ret, d, double);
  } break;
  case ElemType::DERIVED: {
    ret.set_type(constant::Type::ULONG);
    CASTCONST(c, ret, ul, unsigned long);
  } break;
  case ElemType::POINTER:
  case ElemType::NONE:
    UNREACHABLE();
  }

  return ret;
}

bool isComplexType(ElemType type) {
  if (type == ElemType::DERIVED || type == ElemType::POINTER)
    return true;
  return false;
}
} // namespace ast
} // namespace scarlet
