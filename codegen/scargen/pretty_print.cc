#include <codegen/codegen.hh>

namespace scarlet {
namespace codegen {

void Codegen::pretty_print_function(
    std::shared_ptr<scar::scar_Function_Node> function) {
  std::cout << "\tFunction(" << std::endl;
  std::cout << "\t\tname=\"" << function->get_identifier()->get_value() << "\","
            << std::endl;
  std::cout << "\t\tglobal=" << (function->is_global() ? "True" : "False")
            << "," << std::endl;
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
      std::cout << "\t\t\t" << to_string(callStmt->get_type()) << "(";
      std::cout << "name=\"" << callStmt->get_name()->get_value() << "\", ";
      std::cout << "args=[";
      for (auto arg : callStmt->get_args()) {
        if (arg->get_type() == scar::val_type::VAR) {
          std::cout << "Var(" << arg->get_reg() << "), ";
        } else if (arg->get_type() == scar::val_type::CONSTANT) {
          std::cout << "Constant(" << arg->get_const_val() << "), ";
        }
      }
      std::cout << "], Dst=Var(" << callStmt->get_dst()->get_reg() << "))"
                << std::endl;
      continue;
    }
    std::cout << "\t\t\t" << to_string(statement->get_type()) << "(";
    if (statement->get_type() == scar::instruction_type::RETURN) {
      if (statement->get_src1()->get_type() == scar::val_type::CONSTANT) {
        std::cout << "Constant(" << statement->get_src1()->get_const_val()
                  << ")";
      } else if (statement->get_src1()->get_type() == scar::val_type::VAR) {
        std::cout << "Var(" << statement->get_src1()->get_reg() << ")";
      }
      std::cout << ")" << std::endl;
    } else if (statement->get_type() == scar::instruction_type::UNARY) {
      std::cout << unop::to_string(statement->get_unop()) << ", ";
      if (statement->get_src1()->get_type() == scar::val_type::CONSTANT) {
        std::cout << "Constant(" << statement->get_src1()->get_const_val()
                  << ")";
      } else if (statement->get_src1()->get_type() == scar::val_type::VAR) {
        std::cout << "Var(" << statement->get_src1()->get_reg() << ")";
      }
      std::cout << ", ";
      if (statement->get_dst()->get_type() == scar::val_type::VAR) {
        std::cout << "Var(" << statement->get_dst()->get_reg() << ")";
      } else if (statement->get_dst()->get_type() == scar::val_type::CONSTANT) {
        std::cout << "Constant(" << statement->get_dst()->get_const_val()
                  << ")";
      }
      std::cout << ")" << std::endl;
    } else if (statement->get_type() == scar::instruction_type::BINARY) {
      std::cout << binop::to_string(statement->get_binop()) << ", ";
      if (statement->get_src1()->get_type() == scar::val_type::VAR) {
        std::cout << "Var(" << statement->get_src1()->get_reg() << ")";
      } else if (statement->get_src1()->get_type() ==
                 scar::val_type::CONSTANT) {
        std::cout << "Constant(" << statement->get_src1()->get_const_val()
                  << ")";
      }
      std::cout << ", ";
      if (statement->get_src2()->get_type() == scar::val_type::VAR) {
        std::cout << "Var(" << statement->get_src2()->get_reg() << ")";
      } else if (statement->get_src2()->get_type() ==
                 scar::val_type::CONSTANT) {
        std::cout << "Constant(" << statement->get_src2()->get_const_val()
                  << ")";
      }
      std::cout << ", ";
      if (statement->get_dst()->get_type() == scar::val_type::VAR) {
        std::cout << "Var(" << statement->get_dst()->get_reg() << ")";
      } else if (statement->get_dst()->get_type() == scar::val_type::CONSTANT) {
        std::cout << "Constant(" << statement->get_dst()->get_const_val()
                  << ")";
      }
      std::cout << ")" << std::endl;
    } else if (statement->get_type() == scar::instruction_type::COPY) {
      if (statement->get_src1()->get_type() == scar::val_type::VAR) {
        std::cout << "Var(" << statement->get_src1()->get_reg() << ")";
      } else if (statement->get_src1()->get_type() ==
                 scar::val_type::CONSTANT) {
        std::cout << "Constant(" << statement->get_src1()->get_const_val()
                  << ")";
      }
      std::cout << " ,";
      if (statement->get_dst()->get_type() == scar::val_type::VAR) {
        std::cout << "Var(" << statement->get_dst()->get_reg() << ")";
      }
      std::cout << ")" << std::endl;
    } else if (statement->get_type() == scar::instruction_type::JUMP or
               statement->get_type() == scar::instruction_type::LABEL) {
      std::cout << statement->get_src1()->get_label() << ")" << std::endl;
    } else if (statement->get_type() == scar::instruction_type::JUMP_IF_ZERO or
               statement->get_type() ==
                   scar::instruction_type::JUMP_IF_NOT_ZERO) {
      if (statement->get_src1()->get_type() == scar::val_type::VAR) {
        std::cout << "Var(" << statement->get_src1()->get_reg() << ")";
      } else if (statement->get_src1()->get_type() ==
                 scar::val_type::CONSTANT) {
        std::cout << "Constant(" << statement->get_src1()->get_const_val()
                  << ")";
      }
      std::cout << ", ";
      std::cout << statement->get_dst()->get_label() << ")" << std::endl;
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
  std::cout << "\t\tinit=" << static_variable->get_init() << std::endl;
  std::cout << "\t)," << std::endl;
}

void Codegen::pretty_print() {
  std::cout << "Program(" << std::endl;
  for (auto elem : scar.get_elems()) {
    if (elem->get_type() == scar::topLevelType::FUNCTION) {
      pretty_print_function(
          std::static_pointer_cast<scar::scar_Function_Node>(elem));
    } else {
      pretty_print_static_variable(
          std::static_pointer_cast<scar::scar_StaticVariable_Node>(elem));
    }
  }
  std::cout << ")" << std::endl;
}

} // namespace codegen
} // namespace scarlet
