#include <codegen/codegen.hh>

namespace scarlet {
namespace codegen {

void Codegen::pretty_print() {
  std::cout << "Program(" << std::endl;
  for (auto function : scar.get_functions()) {
    std::cout << "\tFunction(" << std::endl;
    std::cout << "\t\tname=\"" << function->get_identifier()->get_value()
              << "\"," << std::endl;
    std::cout << "\t\tbody=[" << std::endl;
    for (auto statement : function->get_instructions()) {
      std::cout << "\t\t\t" << to_string(statement->get_type()) << "(";
      if (statement->get_type() == scar::instruction_type::RETURN) {
        if (statement->get_src1()->get_type() == scar::val_type::CONSTANT) {
          std::cout << "Constant(" << statement->get_src1()->get_value() << ")";
        } else if (statement->get_src1()->get_type() == scar::val_type::VAR) {
          std::cout << "Var(" << statement->get_src1()->get_reg() << ")";
        }
        std::cout << ")" << std::endl;
      } else if (statement->get_type() == scar::instruction_type::UNARY) {
        std::cout << unop::to_string(statement->get_unop()) << ", ";
        if (statement->get_src1()->get_type() == scar::val_type::CONSTANT) {
          std::cout << "Constant(" << statement->get_src1()->get_value() << ")";
        } else if (statement->get_src1()->get_type() == scar::val_type::VAR) {
          std::cout << "Var(" << statement->get_src1()->get_reg() << ")";
        }
        std::cout << ", ";
        if (statement->get_dst()->get_type() == scar::val_type::VAR) {
          std::cout << "Var(" << statement->get_dst()->get_reg() << ")";
        } else if (statement->get_dst()->get_type() ==
                   scar::val_type::CONSTANT) {
          std::cout << "Constant(" << statement->get_dst()->get_value() << ")";
        }
        std::cout << ")" << std::endl;
      } else if (statement->get_type() == scar::instruction_type::BINARY) {
        std::cout << binop::to_string(statement->get_binop()) << ", ";
        if (statement->get_src1()->get_type() == scar::val_type::VAR) {
          std::cout << "Var(" << statement->get_src1()->get_reg() << ")";
        } else if (statement->get_src1()->get_type() ==
                   scar::val_type::CONSTANT) {
          std::cout << "Constant(" << statement->get_src1()->get_value() << ")";
        }
        std::cout << ", ";
        if (statement->get_src2()->get_type() == scar::val_type::VAR) {
          std::cout << "Var(" << statement->get_src2()->get_reg() << ")";
        } else if (statement->get_src2()->get_type() ==
                   scar::val_type::CONSTANT) {
          std::cout << "Constant(" << statement->get_src2()->get_value() << ")";
        }
        std::cout << ", ";
        if (statement->get_dst()->get_type() == scar::val_type::VAR) {
          std::cout << "Var(" << statement->get_dst()->get_reg() << ")";
        } else if (statement->get_dst()->get_type() ==
                   scar::val_type::CONSTANT) {
          std::cout << "Constant(" << statement->get_dst()->get_value() << ")";
        }
        std::cout << ")" << std::endl;
      } else if (statement->get_type() == scar::instruction_type::COPY) {
        if (statement->get_src1()->get_type() == scar::val_type::VAR) {
          std::cout << "Var(" << statement->get_src1()->get_reg() << ")";
        } else if (statement->get_src1()->get_type() ==
                   scar::val_type::CONSTANT) {
          std::cout << "Constant(" << statement->get_src1()->get_value() << ")";
        }
        std::cout << " ,";
        if (statement->get_dst()->get_type() == scar::val_type::VAR) {
          std::cout << "Var(" << statement->get_dst()->get_reg() << ")";
        }
        std::cout << ")" << std::endl;
      } else if (statement->get_type() == scar::instruction_type::JUMP or
                 statement->get_type() == scar::instruction_type::LABEL) {
        std::cout << statement->get_src1()->get_value() << ")" << std::endl;
      } else if (statement->get_type() ==
                     scar::instruction_type::JUMP_IF_ZERO or
                 statement->get_type() ==
                     scar::instruction_type::JUMP_IF_NOT_ZERO) {
        if (statement->get_src1()->get_type() == scar::val_type::VAR) {
          std::cout << "Var(" << statement->get_src1()->get_reg() << ")";
        } else if (statement->get_src1()->get_type() ==
                   scar::val_type::CONSTANT) {
          std::cout << "Constant(" << statement->get_src1()->get_value() << ")";
        }
        std::cout << ", ";
        std::cout << statement->get_dst()->get_value() << ")" << std::endl;
      }
    }
    std::cout << "\t\t]" << std::endl;
    std::cout << "\t)," << std::endl;
  }
  std::cout << ")" << std::endl;
}

} // namespace codegen
} // namespace scarlet
