#include <codegen/codegen.hh>

namespace scarlet {
namespace codegen {

void Codegen::pretty_print_type(ast::ElemType type,
                                std::vector<long> derivedType) {
  std::cout << "<<";
  if (type != ast::ElemType::DERIVED) {
    std::cout << ast::to_string(type);
  } else {
    for (auto i : derivedType) {
      if (i == (long)ast::ElemType::POINTER) {
        std::cout << "*->";
      } else if (i > 0) {
        std::cout << "[" << i << "]->";
      } else {
        std::cout << ast::to_string(static_cast<ast::ElemType>(i));
      }
    }
  }
  std::cout << ">>";
}

#define PRINT_VAR_CONST(scarValNode)                                           \
  if (scarValNode->get_type() == scar::val_type::CONSTANT) {                   \
    std::cout << "<<" << scarValNode->get_const_val().typeToString() << ">>"   \
              << "Constant(" << scarValNode->get_const_val() << ")";           \
  } else if (scarValNode->get_type() == scar::val_type::VAR) {                 \
    pretty_print_type(                                                         \
        globalSymbolTable[scarValNode->get_reg()].typeDef[0],                  \
        globalSymbolTable[scarValNode->get_reg()].derivedTypeMap[0]);          \
    std::cout << "Var(" << scarValNode->get_reg() << ")";                      \
  }

void Codegen::pretty_print_function(
    std::shared_ptr<scar::scar_Function_Node> function) {
  std::cout << "\tFunction(" << std::endl;
  std::cout << "\t\tname=\"" << function->get_identifier()->get_value() << "\","
            << std::endl;
  std::cout << "\t\tglobal=" << (function->is_global() ? "True" : "False")
            << "," << std::endl;
  std::cout << "\t\treturn_type=";
  pretty_print_type(
      globalSymbolTable[function->get_identifier()->get_value()].typeDef[0],
      globalSymbolTable[function->get_identifier()->get_value()]
          .derivedTypeMap[0]);
  std::cout << "," << std::endl;
  std::cout << "\t\tparams=[";
  std::stringstream ss;
  for (auto param : function->get_params()) {
    ss << "Var(" << param->get_value() << "), ";
  }
  std::cout << ss.str().substr(0, ss.str().size() - 2);
  std::cout << "]," << std::endl;
  std::cout << "\t\tbody=[" << std::endl;
  for (auto statement : function->get_instructions()) {
    if (statement->get_type() == scar::instruction_type::CALL) {
      auto callStmt =
          std::static_pointer_cast<scar::scar_FunctionCall_Instruction_Node>(
              statement);
      std::cout << "\t\t\t" << scar::to_string(callStmt->get_type()) << "(";
      std::cout << "name=\"" << callStmt->get_name()->get_value() << "\", ";
      std::cout << "args=[";
      for (auto arg : callStmt->get_args()) {
        PRINT_VAR_CONST(arg);
        std::cout << ", ";
      }
      if (callStmt->get_dst() != nullptr) {
        std::cout << "], Dst=Var(" << callStmt->get_dst()->get_reg() << "))"
                  << std::endl;
      } else {
        std::cout << "], Dst=None)" << std::endl;
      }
      continue;
    }
    std::cout << "\t\t\t" << scar::to_string(statement->get_type()) << "(";
    if (statement->get_type() == scar::instruction_type::RETURN) {
      if (statement->get_src1() != nullptr) {
        PRINT_VAR_CONST(statement->get_src1());
      }
      std::cout << ")" << std::endl;

    } else if (statement->get_type() == scar::instruction_type::UNARY) {
      std::cout << unop::to_string(statement->get_unop()) << ", ";
      PRINT_VAR_CONST(statement->get_src1());
      std::cout << ", ";
      PRINT_VAR_CONST(statement->get_dst());
      std::cout << ")" << std::endl;
    } else if (statement->get_type() == scar::instruction_type::BINARY) {
      std::cout << binop::to_string(statement->get_binop()) << ", ";
      PRINT_VAR_CONST(statement->get_src1());
      std::cout << ", ";
      PRINT_VAR_CONST(statement->get_src2());
      std::cout << ", ";
      PRINT_VAR_CONST(statement->get_dst());
      std::cout << ")" << std::endl;
    } else if (statement->get_type() == scar::instruction_type::ADD_PTR) {
      PRINT_VAR_CONST(statement->get_src1());
      std::cout << ", ";
      PRINT_VAR_CONST(statement->get_src2());
      std::cout << ", ";
      PRINT_VAR_CONST(statement->get_dst());
      std::cout << ", ";
      std::cout << "scale=" << statement->get_offset();
      std::cout << ")" << std::endl;
    } else if (statement->get_type() ==
               scar::instruction_type::COPY_TO_OFFSET) {
      PRINT_VAR_CONST(statement->get_src1());
      std::cout << ", ";
      PRINT_VAR_CONST(statement->get_dst());
      std::cout << ", ";
      std::cout << "offset=" << statement->get_offset();
      std::cout << ")" << std::endl;
    } else if (statement->get_type() == scar::instruction_type::LOAD or
               statement->get_type() == scar::instruction_type::STORE or
               statement->get_type() == scar::instruction_type::GET_ADDRESS) {
      PRINT_VAR_CONST(statement->get_src1());
      std::cout << ", ";
      PRINT_VAR_CONST(statement->get_dst());
      std::cout << ")" << std::endl;
    } else if (statement->get_type() == scar::instruction_type::COPY) {
      PRINT_VAR_CONST(statement->get_src1());
      std::cout << " ,";
      PRINT_VAR_CONST(statement->get_dst());
      std::cout << ")" << std::endl;
    } else if (statement->get_type() == scar::instruction_type::JUMP or
               statement->get_type() == scar::instruction_type::LABEL) {
      std::cout << statement->get_src1()->get_label() << ")" << std::endl;
    } else if (statement->get_type() == scar::instruction_type::JUMP_IF_ZERO or
               statement->get_type() ==
                   scar::instruction_type::JUMP_IF_NOT_ZERO) {
      PRINT_VAR_CONST(statement->get_src1());
      std::cout << ", ";
      std::cout << statement->get_dst()->get_label() << ")" << std::endl;
    } else if (statement->get_type() == scar::instruction_type::SIGN_EXTEND or
               statement->get_type() == scar::instruction_type::TRUNCATE or
               statement->get_type() == scar::instruction_type::ZERO_EXTEND or
               statement->get_type() == scar::instruction_type::DOUBLE_TO_INT or
               statement->get_type() == scar::instruction_type::INT_TO_DOUBLE or
               statement->get_type() ==
                   scar::instruction_type::DOUBLE_TO_UINT or
               statement->get_type() ==
                   scar::instruction_type::UINT_TO_DOUBLE) {
      PRINT_VAR_CONST(statement->get_src1());
      std::cout << ", ";
      PRINT_VAR_CONST(statement->get_dst());
      std::cout << ")" << std::endl;
    } else if (statement->get_type() == scar::instruction_type::ADD_PTR) {
      PRINT_VAR_CONST(statement->get_src1());
      std::cout << ", ";
      PRINT_VAR_CONST(statement->get_src2());
      std::cout << ", ";
      PRINT_VAR_CONST(statement->get_dst());
      std::cout << ", ";
      std::cout << statement->get_offset();

      std::cout << ")" << std::endl;
    }
  }
  std::cout << "\t\t]" << std::endl;
  std::cout << "\t)," << std::endl;
}

void Codegen::pretty_print_static_variable(
    std::shared_ptr<scar::scar_StaticVariable_Node> static_variable) {
  std::cout << "\tStaticVariable(" << std::endl;
  std::cout << "\t\tname=\"" << static_variable->get_identifier()->get_value()
            << "\"," << std::endl;
  std::cout << "\t\tglobal="
            << (static_variable->is_global() ? "True" : "False") << ","
            << std::endl;
  std::cout << "\t\tinit=[";
  for (int i = 0; i < (int)static_variable->get_init().size() - 1; i++) {
    std::cout << static_variable->get_init()[i] << ", ";
  }
  std::cout << static_variable->get_init().back() << "]," << std::endl;
  std::cout << "\t\ttype=";
  pretty_print_type(
      globalSymbolTable[static_variable->get_identifier()->get_value()]
          .typeDef[0],
      globalSymbolTable[static_variable->get_identifier()->get_value()]
          .derivedTypeMap[0]);
  std::cout << std::endl;
}

void Codegen::pretty_print_static_constant(
    std::shared_ptr<scar::scar_StaticConstant_Node> static_constant) {
  std::cout << "\tStaticConstant(" << std::endl;
  std::cout << "\t\tname=\"" << static_constant->get_identifier()->get_value()
            << "\"," << std::endl;
  std::cout << "\t\tglobal="
            << (static_constant->is_global() ? "True" : "False") << ","
            << std::endl;
  std::cout << "\t\tinit=";
  if (static_constant->get_init().get_type() == constant::Type::STRING) {
    std::cout << "String(" << static_constant->get_init().get_string() << ")";
  } else {
    std::cout << "Constant(" << static_constant->get_init() << ")";
  }
  std::cout << "\t)" << std::endl;
}

void Codegen::pretty_print() {
  std::cout << "Program(" << std::endl;
  for (auto elem : scar.get_elems()) {
    if (elem->get_type() == scar::topLevelType::FUNCTION) {
      pretty_print_function(
          std::static_pointer_cast<scar::scar_Function_Node>(elem));
    } else if (elem->get_type() == scar::topLevelType::STATIC_VARIABLE) {
      pretty_print_static_variable(
          std::static_pointer_cast<scar::scar_StaticVariable_Node>(elem));
    } else if (elem->get_type() == scar::topLevelType::STATIC_CONSTANT) {
      pretty_print_static_constant(
          std::static_pointer_cast<scar::scar_StaticConstant_Node>(elem));
    }
  }
  std::cout << ")" << std::endl;
}

} // namespace codegen
} // namespace scarlet
