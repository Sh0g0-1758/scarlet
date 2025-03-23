#include <parser/common.hh>

namespace scarlet {
namespace parser {

void parser::pretty_print_factor(std::shared_ptr<ast::AST_factor_Node> factor) {
  if (factor == nullptr)
    return;
  if (factor->get_unop_node() != nullptr) {
    std::cout << "Unop(" << unop::to_string(factor->get_unop_node()->get_op())
              << "(";
    pretty_print_factor(factor->get_child());
    std::cout << "))";
  } else if (factor->get_cast_type() != ast::ElemType::NONE) {
    std::cout << "Cast(" << to_string(factor->get_cast_type()) << "(";
    pretty_print_factor(factor->get_child());
    std::cout << "))";
  } else if (factor->get_exp_node() != nullptr) {
    pretty_print_exp(factor->get_exp_node());
  } else if (factor->get_const_node() != nullptr) {
    std::cout << factor->get_const_node()->get_AST_name() << "("
              << factor->get_const_node()->get_constant() << ")";
  } else if (factor->get_identifier_node() != nullptr) {
    if (factor->get_factor_type() == ast::FactorType::FUNCTION_CALL) {
      auto funcCall =
          std::static_pointer_cast<ast::AST_factor_function_call_Node>(factor);
      std::cout << funcCall->get_AST_name() << "("
                << funcCall->get_identifier_node()->get_value() << "), "
                << "Arguments: {";
      for (auto arg : funcCall->get_arguments()) {
        pretty_print_exp(arg);
      }
      std::cout << "}" << std::endl;
    } else {
      std::cout << factor->get_identifier_node()->get_AST_name() << "("
                << factor->get_identifier_node()->get_value() << ")";
    }
  }
}

void parser::pretty_print_exp(std::shared_ptr<ast::AST_exp_Node> exp) {
  if (exp == nullptr)
    return;
  pretty_print_exp(exp->get_left());
  if (exp->get_binop_node() != nullptr and
      exp->get_binop_node()->get_op() != binop::BINOP::UNKNOWN) {
    std::cout << "\t\t\t\t\tBinop("
              << binop::to_string(exp->get_binop_node()->get_op()) << " ,";
    if (exp->get_left() == nullptr) {
      pretty_print_factor(exp->get_factor_node());
    } else {
      std::cout << "Earlier, ";
    }
    if (exp->get_binop_node()->get_op() == binop::BINOP::TERNARY) {
      auto ternary = std::static_pointer_cast<ast::AST_ternary_exp_Node>(exp);
      pretty_print_exp(ternary->get_middle());
      std::cout << ", ";
    }
    pretty_print_exp(exp->get_right());
    std::cout << ")" << std::endl;
  } else {
    std::cout << "\t\t\t\t\t";
    pretty_print_factor(exp->get_factor_node());
    std::cout << std::endl;
  }
}

} // namespace parser
} // namespace scarlet
