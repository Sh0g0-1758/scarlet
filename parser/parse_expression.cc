#include "common.hh"

namespace scarlet {
namespace parser {

#define PARSE_TYPE(decl, func)                                                 \
  if (!tokens.empty() and tokens[0].get_token() == token::TOKEN::INT) {        \
    tokens.erase(tokens.begin());                                              \
    if (!tokens.empty() and tokens[0].get_token() == token::TOKEN::LONG) {     \
      tokens.erase(tokens.begin());                                            \
      decl->func(ast::ElemType::LONG);                                         \
    } else {                                                                   \
      decl->func(ast::ElemType::INT);                                          \
    }                                                                          \
  } else {                                                                     \
    EXPECT(token::TOKEN::LONG);                                                \
    decl->func(ast::ElemType::LONG);                                           \
    if (!tokens.empty() and (tokens[0].get_token() == token::TOKEN::INT or     \
                             tokens[0].get_token() == token::TOKEN::LONG)) {   \
      tokens.erase(tokens.begin());                                            \
    }                                                                          \
  }

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
  if (tokens[i].get_token() == token::TOKEN::IDENTIFIER) {
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

void parser::parse_factor(std::vector<token::Token> &tokens,
                          std::shared_ptr<ast::AST_factor_Node> &factor) {
  if (tokens[0].get_token() == token::TOKEN::INT_CONSTANT or
      tokens[0].get_token() == token::TOKEN::LONG_CONSTANT) {
    parse_const(tokens, factor);
  } else if (tokens[0].get_token() == token::TOKEN::IDENTIFIER) {
    EXPECT_IDENTIFIER();
    if (tokens[0].get_token() == token::TOKEN::OPEN_PARANTHESES) {
      // TREAT IT AS A FUNCTION CALL
      MAKE_SHARED(ast::AST_factor_function_call_Node, function_call);

      for (auto unop : factor->get_unop_nodes()) {
        function_call->add_unop_node(std::move(unop));
      }

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
    parse_factor(tokens, factor);
  } else if (tokens[0].get_token() == token::TOKEN::OPEN_PARANTHESES) {
    /**
     * Simplification for Single-Identifier Parentheses
     *
     * When encountering an expression with a single identifier wrapped in
     * parentheses, e.g., (identifier), we treat it as a simple identifier
     * rather than a complex expression. This optimization significantly reduces
     * complexity during semantic analysis by avoiding unnecessary nested
     * expression handling.
     *
     * Example:
     *   Input: (((((((((((((((((a)))))))))))))))))
     *   Treats as: a
     */
    std::pair<bool, int> res = is_single_identifier_parentheses(tokens);
    if (res.first) {
      for (int i = 0; i < res.second; i++) {
        tokens.erase(tokens.begin());
      }
      EXPECT_IDENTIFIER();
      factor->set_identifier_node(std::move(identifier));
      for (int i = 0; i < res.second; i++) {
        tokens.erase(tokens.begin());
      }
    } else {
      // it can have a nested expression or it could be a cast operation
      tokens.erase(tokens.begin());
      if (!tokens.empty() and token::is_type_specifier(tokens[0].get_token())) {
        PARSE_TYPE(factor, add_cast_type);
        EXPECT(token::TOKEN::CLOSE_PARANTHESES);
        parse_factor(tokens, factor);
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
  // IF THIS BRANCH IS CALLED THEN WE CAN BE SURE THAT WE ARE DEALING WITH A
  // POST INCREMENT OR DECREMENT
  if (tokens[0].get_token() == token::TOKEN::INCREMENT_OPERATOR or
      tokens[0].get_token() == token::TOKEN::DECREMENT_OPERATOR) {
    MAKE_SHARED(ast::AST_unop_Node, unop);
    if (tokens[0].get_token() == token::TOKEN::INCREMENT_OPERATOR) {
      unop->set_op(unop::UNOP::POSTINCREMENT);
    } else {
      unop->set_op(unop::UNOP::POSTDECREMENT);
    }
    tokens.erase(tokens.begin());
    factor->add_unop_node(std::move(unop));
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
