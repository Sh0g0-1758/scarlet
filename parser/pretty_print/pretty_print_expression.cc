#include <parser/common.hh>

namespace scarlet {
namespace parser {

void parser::pretty_print_factor(std::shared_ptr<ast::AST_factor_Node> factor) {
  if (factor == nullptr)
    return;

  if (factor->get_type() == ast::ElemType::DERIVED) {
    std::cout << "<<";
    for (auto it : factor->get_derived_type()) {
      if (it > 0) {
        std::cout << "[" << it << "]->";
      } else if (it == -2) {
        std::cout << "*->";
      } else {
        std::cout << ast::to_string(static_cast<ast::ElemType>(it));
      }
    }
    std::cout << ">>";
  } else {
    std::cout << "<<" << ast::to_string(factor->get_type()) << ">>";
  }

  if (factor->get_unop_node() != nullptr) {
    std::cout << "Unop(" << unop::to_string(factor->get_unop_node()->get_op())
              << "(";
    pretty_print_factor(factor->get_child());
    std::cout << "))";
  } else if (factor->get_cast_type() != ast::ElemType::NONE) {
    std::cout << "Cast(";
    pretty_print_factor(factor->get_child());
    std::cout << ")";
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
    if (factor->get_arrIdx().size() > 0) {
      for (auto arrIdx : factor->get_arrIdx()) {
        std::cout << " [";
        pretty_print_exp(arrIdx);
        std::cout << "]";
      }
    }
  }
}

void parser::pretty_print_exp(std::shared_ptr<ast::AST_exp_Node> exp) {
  if (exp == nullptr)
    return;
  if (exp->get_binop_node() != nullptr and
      exp->get_binop_node()->get_op() != binop::BINOP::UNKNOWN) {
    pretty_print_exp(exp->get_left());
    std::cout << "\t\t\t";
    if (exp->get_type() == ast::ElemType::DERIVED) {
      std::cout << "<<";
      for (auto it : exp->get_derived_type()) {
        if (it > 0) {
          std::cout << "[" << it << "]->";
        } else if (it == -2) {
          std::cout << "*->";
        } else {
          std::cout << ast::to_string(static_cast<ast::ElemType>(it));
        }
      }
      std::cout << ">>";
    } else {
      std::cout << "<<" << ast::to_string(exp->get_type()) << ">>";
    }
    std::cout << "Binop(" << binop::to_string(exp->get_binop_node()->get_op())
              << " ,";
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
    std::cout << "\t\t\t";
    pretty_print_factor(exp->get_factor_node());
    std::cout << std::endl;
  }
}

} // namespace parser
} // namespace scarlet
