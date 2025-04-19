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
  case constant::Type::CHAR:
    return ElemType::CHAR;
  case constant::Type::UCHAR:
    return ElemType::UCHAR;
  // this case will never be reached, so we can safely ignore it
  case constant::Type::STRING:
  case constant::Type::ZERO:
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
  case ElemType::DERIVED:
    return constant::Type::ULONG;
  case ElemType::CHAR:
    return constant::Type::CHAR;
  case ElemType::UCHAR:
    return constant::Type::UCHAR;
  case ElemType::POINTER:
  case ElemType::NONE:
  case ElemType::VOID:
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
  case ElemType::VOID:
    return "void";
  case ElemType::CHAR:
    return "char";
  case ElemType::UCHAR:
    return "unsigned char";
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

int getSizeOfTypeOnArch(ElemType type) {
  switch (type) {
  case ast::ElemType::INT:
  case ast::ElemType::UINT:
    return sizeof(int);
  case ast::ElemType::LONG:
  case ast::ElemType::ULONG:
    return sizeof(long);
  case ast::ElemType::DOUBLE:
    return sizeof(double);
  case ast::ElemType::POINTER:
    return sizeof(unsigned long);
  case ast::ElemType::CHAR:
  case ast::ElemType::UCHAR:
    return sizeof(char);
  case ast::ElemType::VOID:
  case ast::ElemType::DERIVED:
  case ast::ElemType::NONE:
    return 0;
  }
  return 0;
}

// WARNING: Use this function when you want to find the size of the type
//          a pointer points to
unsigned long getSizeOfReferencedTypeOnArch(std::vector<long> derivedType) {
  long sizeOfReferencedType = 1;
  int i = 1;
  while (derivedType[i] > 0) {
    sizeOfReferencedType *= derivedType[i];
    i++;
  }
  sizeOfReferencedType *=
      ast::getSizeOfTypeOnArch((ast::ElemType)derivedType[i]);
  return sizeOfReferencedType;
}

unsigned long getSizeOfArrayTypeOnArch(std::vector<long> derivedType) {
  return derivedType[0] * getSizeOfReferencedTypeOnArch(derivedType);
}

bool is_const_zero(std::shared_ptr<AST_factor_Node> factor) {
  if (factor != nullptr and factor->get_const_node() != nullptr and
      factor->get_const_node()->get_constant().get_value().i == 0 and
      factor->get_const_node()->get_constant().get_type() !=
          constant::Type::DOUBLE)
    return true;
  return false;
}

bool is_ptr_type(ast::ElemType type, std::vector<long> derivedType) {
  if (type == ast::ElemType::DERIVED and
      derivedType[0] == (long)ast::ElemType::POINTER)
    return true;
  return false;
}

bool is_void_ptr(ast::ElemType type, std::vector<long> derivedType) {
  return is_ptr_type(type, derivedType) and
         derivedType[1] == (long)ast::ElemType::VOID;
}

bool is_pointer_to_complete_type(ast::ElemType type,
                                 std::vector<long> derivedType) {
  if (type == ast::ElemType::DERIVED and
      derivedType[0] == (long)ast::ElemType::POINTER) {
    if (derivedType[1] == (long)ast::ElemType::VOID)
      return false;
    return true;
  }
  return true;
}

// Check type specifiers which have void arrays void[3] or void(*[4])[2]
// Makes sure that there are only void pointers. Void arrays are not allowed
// and void as a basic type is not allowed. The check is recursive in nature.
bool validate_type_specifier(ast::ElemType type,
                             std::vector<long> derivedType) {
  if (type == ast::ElemType::DERIVED and derivedType[0] > 0) {
    if (derivedType[1] == (long)ast::ElemType::VOID) {
      return false;
    } else if (derivedType.size() == 2) {
      return true;
    } else {
      derivedType.erase(derivedType.begin());
      return validate_type_specifier(ast::ElemType::DERIVED, derivedType);
    }

  } else if (type == ast::ElemType::DERIVED and
             derivedType[0] == (long)ast::ElemType::POINTER) {
    if (derivedType.size() == 2) {
      return true;
    } else {
      derivedType.erase(derivedType.begin());
      return validate_type_specifier(ast::ElemType::DERIVED, derivedType);
    }
  } else if (type == ast::ElemType::VOID) {
    return false;
  }
  return true;
}

bool is_valid_declarator(ast::ElemType type, std::vector<long> derivedType) {
  if (type == ast::ElemType::VOID or
      (type == ast::ElemType::DERIVED and
       !ast::validate_type_specifier(type, derivedType)))
    return false;
  return true;
}

void unroll_derived_type(std::shared_ptr<ast::AST_declarator_Node> declarator,
                         std::vector<long> &derivedType) {
  if (declarator == nullptr)
    return;

  unroll_derived_type(declarator->get_child(), derivedType);

  if (declarator->is_pointer()) {
    derivedType.push_back((long)ast::ElemType::POINTER);
  }

  if (!declarator->get_arrDim().empty()) {
    for (auto dim : declarator->get_arrDim()) {
      derivedType.push_back(dim);
    }
  }
}

bool is_lvalue(std::shared_ptr<AST_factor_Node> factor) {
  if (factor == nullptr)
    return false;
  if (factor->get_arrIdx().size() != 0)
    return true;
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

bool exp_is_string(std::shared_ptr<AST_exp_Node> exp) {
  if (exp == nullptr)
    return false;
  if (exp->get_factor_node() == nullptr)
    return false;
  if (exp->get_factor_node()->get_const_node() == nullptr)
    return false;
  if (exp->get_factor_node()->get_const_node()->get_constant().get_type() !=
      constant::Type::STRING)
    return false;
  return true;
}

bool factor_is_string(std::shared_ptr<AST_factor_Node> factor) {
  if (factor == nullptr)
    return false;
  if (factor->get_const_node() == nullptr)
    return false;
  if (factor->get_const_node()->get_constant().get_type() !=
      constant::Type::STRING)
    return false;
  return true;
}

std::pair<ElemType, std::vector<long>>
getParentType(ElemType left, ElemType right, std::vector<long> &leftDerivedType,
              std::vector<long> &rightDerivedType,
              std::shared_ptr<AST_exp_Node> exp) {
  // implicit promotion of char to int
  if (left == ElemType::CHAR or left == ElemType::UCHAR)
    left = ElemType::INT;
  if (right == ElemType::CHAR or right == ElemType::UCHAR)
    right = ElemType::INT;

  if (left == ElemType::DERIVED or right == ElemType::DERIVED) {
    if (leftDerivedType == rightDerivedType) {
      return {left, leftDerivedType};
    } else {
      std::shared_ptr<AST_factor_Node> leftFactor{};
      bool isTernary =
          (exp->get_binop_node() != nullptr and
           exp->get_binop_node()->get_op() == binop::BINOP::TERNARY);
      leftFactor = isTernary
                       ? (std::static_pointer_cast<AST_ternary_exp_Node>(exp))
                             ->get_middle()
                             ->get_factor_node()
                       : exp->get_factor_node();
      if (isTernary and (is_void_ptr(left, leftDerivedType) or
                         is_void_ptr(right, rightDerivedType))) {
        return {ElemType::DERIVED,
                {(long)ElemType::POINTER, (long)ElemType::VOID}};
      }
      if (is_const_zero(leftFactor)) {
        return {right, rightDerivedType};
      } else if (is_const_zero(exp->get_right()->get_factor_node())) {
        return {left, leftDerivedType};
      } else {
        return {ElemType::NONE, {}};
      }
    }
  } else {
    if ((left == ElemType::VOID and right != ElemType::VOID) or
        (left != ElemType::VOID and right == ElemType::VOID)) {
      return {ElemType::NONE, {}};
    } else if (left == ast::ElemType::VOID and right == ast::ElemType::VOID) {
      return {ElemType::VOID, {}};
    } else if (left == right)
      return {left, {}};
    else if (left == ElemType::DOUBLE or right == ElemType::DOUBLE)
      return {ElemType::DOUBLE, {}};
    else if (getSizeOfTypeOnArch(left) == getSizeOfTypeOnArch(right)) {
      if (left == ElemType::UINT || left == ElemType::ULONG)
        return {left, {}};
      else
        return {right, {}};
    } else if (getSizeOfTypeOnArch(left) > getSizeOfTypeOnArch(right))
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
    } else if ((is_void_ptr(target, targetDerived) and
                is_ptr_type(src, srcDerived)) or
               (is_ptr_type(target, targetDerived) and
                is_void_ptr(src, srcDerived))) {
      return {target, targetDerived};
    } else {
      return {ElemType::NONE, {}};
    }
  } else {
    if (src == ElemType::VOID) {
      return {ElemType::NONE, {}};
    }
    return {target, targetDerived};
  }
}

#define CASTCONST(Const, ret, t, T)                                            \
  switch (Const.get_type()) {                                                  \
  case constant::Type::INT:                                                    \
    ret.set_value({.t = static_cast<T>(Const.get_value().i)});                 \
    break;                                                                     \
  case constant::Type::LONG:                                                   \
    ret.set_value({.t = static_cast<T>(Const.get_value().l)});                 \
    break;                                                                     \
  case constant::Type::UINT:                                                   \
    ret.set_value({.t = static_cast<T>(Const.get_value().ui)});                \
    break;                                                                     \
  case constant::Type::ULONG:                                                  \
    ret.set_value({.t = static_cast<T>(Const.get_value().ul)});                \
    break;                                                                     \
  case constant::Type::DOUBLE:                                                 \
    ret.set_value({.t = static_cast<T>(Const.get_value().d)});                 \
    break;                                                                     \
  case constant::Type::CHAR:                                                   \
    ret.set_value({.t = static_cast<T>(Const.get_value().c)});                 \
    break;                                                                     \
  case constant::Type::UCHAR:                                                  \
    ret.set_value({.t = static_cast<T>(Const.get_value().uc)});                \
    break;                                                                     \
  case constant::Type::STRING:                                                 \
  case constant::Type::ZERO:                                                   \
  case constant::Type::NONE:                                                   \
    break;                                                                     \
  }

constant::Constant castConstToElemType(constant::Constant Const,
                                       ElemType type) {
  if (constTypeToElemType(Const.get_type()) == type) {
    return Const;
  }

  constant::Constant ret;

  switch (type) {
  case ElemType::INT: {
    ret.set_type(constant::Type::INT);
    CASTCONST(Const, ret, i, int);
  } break;
  case ElemType::LONG: {
    ret.set_type(constant::Type::LONG);
    CASTCONST(Const, ret, l, long);
  } break;
  case ElemType::UINT: {
    ret.set_type(constant::Type::UINT);
    CASTCONST(Const, ret, ui, unsigned int);
  } break;
  case ElemType::ULONG: {
    ret.set_type(constant::Type::ULONG);
    CASTCONST(Const, ret, ul, unsigned long);
  } break;
  case ElemType::DOUBLE: {
    ret.set_type(constant::Type::DOUBLE);
    CASTCONST(Const, ret, d, double);
  } break;
  case ElemType::DERIVED: {
    ret.set_type(constant::Type::ULONG);
    CASTCONST(Const, ret, ul, unsigned long);
  } break;
  case ElemType::CHAR: {
    ret.set_type(constant::Type::CHAR);
    CASTCONST(Const, ret, c, char);
  } break;
  case ElemType::UCHAR: {
    ret.set_type(constant::Type::UCHAR);
    CASTCONST(Const, ret, uc, unsigned char);
  } break;
  case ElemType::POINTER:
  case ElemType::VOID:
    break;
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

std::string get_lvalue_identifier(std::shared_ptr<AST_factor_Node> factor) {
  if (factor == nullptr)
    return "";
  if (factor->get_identifier_node() != nullptr) {
    return factor->get_identifier_node()->get_value();
  } else if (factor->get_child() != nullptr) {
    return get_lvalue_identifier(factor->get_child());
  } else if (factor->get_exp_node() != nullptr) {
    return get_lvalue_identifier(factor->get_exp_node()->get_factor_node());
  }
  return "";
}

bool exp_is_factor(std::shared_ptr<AST_exp_Node> exp) {
  if (exp == nullptr)
    return false;
  if (exp->get_binop_node() != nullptr)
    return false;
  if (exp->get_left() != nullptr)
    return false;
  if (exp->get_right() != nullptr)
    return false;
  if (exp->get_factor_node() == nullptr)
    return false;
  return true;
}

bool is_array(std::shared_ptr<AST_factor_Node> factor) {
  if (factor == nullptr)
    return false;
  if (factor->get_type() != ElemType::DERIVED)
    return false;
  if (factor->get_derived_type().size() == 0)
    return false;
  if (factor->get_derived_type()[0] <= 0)
    return false;
  return true;
}

bool is_array(std::shared_ptr<AST_exp_Node> exp) {
  if (exp == nullptr)
    return false;
  if (exp->get_type() != ElemType::DERIVED)
    return false;
  if (exp->get_derived_type().size() == 0)
    return false;
  if (exp->get_derived_type()[0] <= 0)
    return false;
  return true;
}

} // namespace ast
} // namespace scarlet
