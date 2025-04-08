#include "common.hh"

namespace scarlet {
namespace parser {

std::pair<bool, int>
parser::is_single_identifier_parentheses(std::vector<token::Token> &tokens) {
  int i = 0;
  int NUM_TOKENS = tokens.size();
  while (i < NUM_TOKENS) {
    if (tokens[i].get_token() == token::TOKEN::OPEN_PARANTHESES) {
      i++;
    } else {
      break;
    }
  }
  if (token::is_constant_or_identifier(tokens[i].get_token())) {
    int num_open_parentheses = i;
    int tmp = i;
    i++;
    while (i < NUM_TOKENS) {
      if (tokens[i].get_token() == token::TOKEN::CLOSE_PARANTHESES) {
        tmp--;
        i++;
      } else {
        break;
      }
    }
    if (tmp == 0) {
      return {true, num_open_parentheses};
    }
  }
  return {false, 0};
}

void parser::parse_abstract_declarator(
    std::vector<token::Token> &tokens,
    std::shared_ptr<ast::AST_declarator_Node> &declarator) {
  if (tokens[0].get_token() == token::TOKEN::OPEN_BRACKET) {
    parse_variable_declarator_suffix(tokens, declarator);
  } else if (tokens[0].get_token() == token::TOKEN::OPEN_PARANTHESES) {
    tokens.erase(tokens.begin());
    MAKE_SHARED(ast::AST_declarator_Node, child);
    parse_abstract_declarator(tokens, child);
    declarator->set_child(std::move(child));
    EXPECT(token::TOKEN::CLOSE_PARANTHESES);
    parse_variable_declarator_suffix(tokens, declarator);
  } else if (tokens[0].get_token() == token::TOKEN::ASTERISK) {
    tokens.erase(tokens.begin());
    declarator->set_pointer(true);
    MAKE_SHARED(ast::AST_declarator_Node, child);
    parse_abstract_declarator(tokens, child);
    declarator->set_child(std::move(child));
  }
}

void parser::parse_factor(std::vector<token::Token> &tokens,
                          std::shared_ptr<ast::AST_factor_Node> &factor) {
  if (token::is_numeric_constant(tokens[0].get_token())) {
    parse_const(tokens, factor);
  } else if (tokens[0].get_token() == token::TOKEN::IDENTIFIER) {
    EXPECT_IDENTIFIER();
    if (tokens[0].get_token() == token::TOKEN::OPEN_PARANTHESES) {
      // TREAT IT AS A FUNCTION CALL
      MAKE_SHARED(ast::AST_factor_function_call_Node, function_call);
      function_call->set_factor_type(ast::FactorType::FUNCTION_CALL);
      function_call->set_identifier_node(std::move(identifier));
      EXPECT(token::TOKEN::OPEN_PARANTHESES);
      if (tokens[0].get_token() != token::TOKEN::CLOSE_PARANTHESES) {
        MAKE_SHARED(ast::AST_exp_Node, exp);
        parse_exp(tokens, exp);
        function_call->add_argument(std::move(exp));
        while (tokens[0].get_token() == token::TOKEN::COMMA) {
          tokens.erase(tokens.begin());
          MAKE_SHARED(ast::AST_exp_Node, exp);
          parse_exp(tokens, exp);
          function_call->add_argument(std::move(exp));
        }
      }
      EXPECT(token::TOKEN::CLOSE_PARANTHESES);
      factor = std::static_pointer_cast<ast::AST_factor_Node>(function_call);
    } else {
      factor->set_identifier_node(std::move(identifier));
    }
  } else if (token::is_unary_op(tokens[0].get_token())) {
    parse_unary_op(tokens, factor);
    MAKE_SHARED(ast::AST_factor_Node, nested_factor);
    parse_factor(tokens, nested_factor);
    factor->set_child(std::move(nested_factor));
  } else if (tokens[0].get_token() == token::TOKEN::OPEN_PARANTHESES) {
    /**
     * Simplification for Single-Identifier Parentheses
     *
     * When encountering an expression with a single identifier or numeric
     * constant wrapped in parentheses, we treat it as a simple identifier or
     * numeric constant rather than a complex expression. This optimization
     * significantly reduces complexity during semantic analysis by avoiding
     * unnecessary nested expression handling.
     *
     * Example:
     *   Input: (((((((((((((((((a)))))))))))))))))
     *   Treats as: a
     *
     *   Input: (((((((((((((((((1)))))))))))))))))
     *   Treats as: 1
     */
    std::pair<bool, int> res = is_single_identifier_parentheses(tokens);
    if (res.first) {
      for (int i = 0; i < res.second; i++) {
        tokens.erase(tokens.begin());
      }
      if (tokens[0].get_token() == token::TOKEN::IDENTIFIER) {
        EXPECT_IDENTIFIER();
        factor->set_identifier_node(std::move(identifier));
      } else {
        parse_const(tokens, factor);
      }
      for (int i = 0; i < res.second; i++) {
        tokens.erase(tokens.begin());
      }
    } else {
      // it can have a nested expression or it could be a cast operation
      tokens.erase(tokens.begin());
      if (!tokens.empty() and token::is_type_specifier(tokens[0].get_token())) {
        PARSE_TYPE(factor, set_cast_type);
        if (!tokens.empty() and
            tokens[0].get_token() != token::TOKEN::CLOSE_PARANTHESES) {
          MAKE_SHARED(ast::AST_declarator_Node, cast_declarator);
          parse_abstract_declarator(tokens, cast_declarator);
          factor->set_cast_declarator(std::move(cast_declarator));
        }
        EXPECT(token::TOKEN::CLOSE_PARANTHESES);
        MAKE_SHARED(ast::AST_factor_Node, nested_factor);
        parse_factor(tokens, nested_factor);
        factor->set_child(std::move(nested_factor));
      } else {
        MAKE_SHARED(ast::AST_exp_Node, exp);
        parse_exp(tokens, exp);
        factor->set_exp_node(std::move(exp));
        EXPECT(token::TOKEN::CLOSE_PARANTHESES);
      }
    }
  } else {
    success = false;
    error_messages.emplace_back("Expected constant, unary operator, semicolon "
                                "or open parantheses but got " +
                                token::to_string(tokens[0].get_token()));
  }
  // NOTE THIS IS A SPECIAL CASE WHERE WE HAVE A POST INCREMENT OR DECREMENT
  if (tokens[0].get_token() == token::TOKEN::INCREMENT_OPERATOR or
      tokens[0].get_token() == token::TOKEN::DECREMENT_OPERATOR) {
    if (factor->get_factor_type() == ast::FactorType::FUNCTION_CALL) {
      success = false;
      error_messages.emplace_back(
          "Expected an lvalue for the increment / decrement operator");
    } else {
      MAKE_SHARED(ast::AST_factor_Node, nested_factor);
      nested_factor->set_identifier_node(factor->get_identifier_node());
      nested_factor->set_unop_node(factor->get_unop_node());
      nested_factor->set_exp_node(factor->get_exp_node());
      nested_factor->set_factor_type(factor->get_factor_type());
      nested_factor->set_cast_type(factor->get_cast_type());
      nested_factor->set_cast_declarator(factor->get_cast_declarator());
      nested_factor->set_child(factor->get_child());
      nested_factor->set_arrIdx(factor->get_arrIdx());

      factor->set_identifier_node(nullptr);
      factor->set_unop_node(nullptr);
      factor->set_exp_node(nullptr);
      factor->set_factor_type(ast::FactorType::BASIC);
      factor->set_cast_type(ast::ElemType::NONE);
      factor->set_cast_declarator(nullptr);
      factor->set_child(std::move(nested_factor));
      factor->set_arrIdx({});

      MAKE_SHARED(ast::AST_unop_Node, unop);
      if (tokens[0].get_token() == token::TOKEN::INCREMENT_OPERATOR) {
        unop->set_op(unop::UNOP::POSTINCREMENT);
      } else {
        unop->set_op(unop::UNOP::POSTDECREMENT);
      }
      tokens.erase(tokens.begin());
      factor->set_unop_node(std::move(unop));
    }
  }
  if (tokens[0].get_token() == token::TOKEN::OPEN_BRACKET) {
    // if (factor->get_const_node()!=nullptr){
    //   success = false;
    //   error_messages.emplace_back(
    //       "Expected an lvalue for the array index operator");
    // } else{
    while (tokens[0].get_token() == token::TOKEN::OPEN_BRACKET) {
      tokens.erase(tokens.begin());
      MAKE_SHARED(ast::AST_exp_Node, exp);
      parse_exp(tokens, exp);
      factor->add_arrIdx(exp);
      EXPECT(token::TOKEN::CLOSE_BRACKET);
    }
    // }
  }
}

void parser::parse_exp(std::vector<token::Token> &tokens,
                       std::shared_ptr<ast::AST_exp_Node> &root_exp, int prec) {
  MAKE_SHARED(ast::AST_factor_Node, factor);
  parse_factor(tokens, factor);
  root_exp->set_factor_node(std::move(factor));
  while (token::is_binary_op(tokens[0].get_token()) and
         token::get_binop_prec(tokens[0].get_token()) >= prec) {
    int new_prec = token::get_binop_prec(tokens[0].get_token()) + 1;
    // Handle right associative operators by reducing the new precedence by 1
    if (token::is_right_associative(tokens[0].get_token()))
      new_prec--;
    MAKE_SHARED(ast::AST_binop_Node, binop);
    parse_binop(tokens, binop);
    // Handle ternary operator
    if (binop->get_op() == binop::BINOP::TERNARY) {
      MAKE_SHARED(ast::AST_exp_Node, middle_exp);
      // reset the precedence
      parse_exp(tokens, middle_exp, 0);

      std::shared_ptr<ast::AST_ternary_exp_Node> root_exp_ternary =
          std::make_shared<ast::AST_ternary_exp_Node>(root_exp);
      root_exp_ternary->set_middle(std::move(middle_exp));
      root_exp = std::static_pointer_cast<ast::AST_exp_Node>(root_exp_ternary);
      EXPECT(token::TOKEN::COLON);
    }
    root_exp->set_binop_node(std::move(binop));
    MAKE_SHARED(ast::AST_exp_Node, rexp);
    parse_exp(tokens, rexp, new_prec);
    root_exp->set_right(std::move(rexp));
    if (token::is_binary_op(tokens[0].get_token()) and
        token::get_binop_prec(tokens[0].get_token()) >= prec) {
      MAKE_SHARED(ast::AST_exp_Node, new_root_exp);
      new_root_exp->set_left(root_exp);
      root_exp = new_root_exp;
    }
  }
}

} // namespace parser
} // namespace scarlet
