#include <parser/common.hh>

namespace scarlet {
namespace parser {

void parser::pretty_print_factor(std::shared_ptr<ast::AST_factor_Node> factor) {
  if (!factor->get_unop_nodes().empty()) {
    std::cout << "Unop( ";
    for (auto unop : factor->get_unop_nodes()) {
      std::cout << unop::to_string(unop->get_op()) << ", ";
    }
  }
  if (factor->get_exp_node() != nullptr) {
    pretty_print_exp(factor->get_exp_node());
  } else if (factor->get_int_node() != nullptr) {
    std::cout << factor->get_int_node()->get_AST_name() << "("
              << factor->get_int_node()->get_value() << ")";
    if (!factor->get_unop_nodes().empty()) {
      std::cout << ")";
    }
  } else {
    if (factor->get_type() == ast::FactorType::FUNCTION_CALL) {
      auto funcCall =
          std::static_pointer_cast<ast::AST_factor_function_call_Node>(factor);
      std::cout << funcCall->get_AST_name() << "("
                << funcCall->get_identifier_node()->get_value() << "), "
                << "Arguments: " << std::endl;
      for (auto arg : funcCall->get_arguments()) {
        pretty_print_exp(arg);
      }
    } else {
      std::cout << factor->get_identifier_node()->get_AST_name() << "("
                << factor->get_identifier_node()->get_value() << ")";
    }
    if (!factor->get_unop_nodes().empty()) {
      std::cout << ")";
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
