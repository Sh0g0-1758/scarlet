#include "parser.hh"
namespace scarlet {
namespace parser {

#define MAKE_SHARED(a, b) std::shared_ptr<a> b = std::make_shared<a>()

#define EXPECT(tok)                                                            \
  if (!success) {                                                              \
    return;                                                                    \
  }                                                                            \
  if (tokens.empty()) {                                                        \
    eof_error(tok);                                                            \
    return;                                                                    \
  }                                                                            \
  expect(tokens[0].get_token(), tok);                                          \
  if (!success) {                                                              \
    return;                                                                    \
  }                                                                            \
  tokens.erase(tokens.begin());

#define EXPECT_FUNC(tok)                                                       \
  if (!success) {                                                              \
    return function;                                                           \
  }                                                                            \
  if (tokens.empty()) {                                                        \
    eof_error(tok);                                                            \
    return function;                                                           \
  }                                                                            \
  expect(tokens[0].get_token(), tok);                                          \
  if (!success) {                                                              \
    return function;                                                           \
  }                                                                            \
  tokens.erase(tokens.begin());

#define EXPECT_INT(tok)                                                        \
  if (!success) {                                                              \
    return;                                                                    \
  }                                                                            \
  if (tokens.empty()) {                                                        \
    eof_error(tok);                                                            \
    return;                                                                    \
  }                                                                            \
  expect(tokens[0].get_token(), tok);                                          \
  if (!success) {                                                              \
    return;                                                                    \
  }                                                                            \
  MAKE_SHARED(ast::AST_int_Node, int_node);                                    \
  int_node->set_value(tokens[0].get_value().value());                          \
  factor->set_int_node(std::move(int_node));                                   \
  tokens.erase(tokens.begin());

#define EXPECT_IDENTIFIER(tok)                                                 \
  if (!success) {                                                              \
    return;                                                                    \
  }                                                                            \
  if (tokens.empty()) {                                                        \
    eof_error(tok);                                                            \
    return;                                                                    \
  }                                                                            \
  expect(tokens[0].get_token(), tok);                                          \
  if (!success) {                                                              \
    return;                                                                    \
  }                                                                            \
  ast::AST_identifier_Node identifier;                                         \
  identifier.set_identifier(tokens[0].get_value().value());                    \
  function->set_identifier(identifier);                                        \
  tokens.erase(tokens.begin());

void parser::parse_program(std::vector<token::Token> tokens) {
  ast::AST_Program_Node program;
  while (!tokens.empty() and success) {
    program.add_function(parse_function(tokens));
  }
  this->program = program;
}

std::shared_ptr<ast::AST_Function_Node>
parser::parse_function(std::vector<token::Token> &tokens) {
  MAKE_SHARED(ast::AST_Function_Node, function);
  EXPECT_FUNC(token::TOKEN::INT);
  parse_identifier(tokens, function);
  EXPECT_FUNC(token::TOKEN::OPEN_PARANTHESES);
  EXPECT_FUNC(token::TOKEN::VOID);
  EXPECT_FUNC(token::TOKEN::CLOSE_PARANTHESES);
  EXPECT_FUNC(token::TOKEN::OPEN_BRACE);
  parse_statement(tokens, function);
  EXPECT_FUNC(token::TOKEN::CLOSE_BRACE);
  return function;
}

void parser::parse_statement(std::vector<token::Token> &tokens,
                             std::shared_ptr<ast::AST_Function_Node> function) {

  std::shared_ptr<ast::AST_Statement_Node> statement =
      std::make_shared<ast::AST_Statement_Node>("Return");
  EXPECT(token::TOKEN::RETURN);
  MAKE_SHARED(ast::AST_exp_Node, exp);
  parse_exp(tokens, exp);
  statement->add_exp(std::move(exp));
  EXPECT(token::TOKEN::SEMICOLON);
  function->add_statement(std::move(statement));
}

void parser::parse_factor(std::vector<token::Token> &tokens,
                          std::shared_ptr<ast::AST_factor_Node> factor) {
  if (tokens[0].get_token() == token::TOKEN::CONSTANT) {
    parse_int(tokens, factor);
  } else if (tokens[0].get_token() == token::TOKEN::TILDE or
             tokens[0].get_token() == token::TOKEN::HYPHEN or
             tokens[0].get_token() == token::TOKEN::NOT) {
    parse_unary_op(tokens, factor);
    parse_factor(tokens, factor);
  } else if (tokens[0].get_token() == token::TOKEN::OPEN_PARANTHESES) {
    tokens.erase(tokens.begin());
    MAKE_SHARED(ast::AST_exp_Node, exp);
    parse_exp(tokens, exp);
    factor->set_exp_node(std::move(exp));
    EXPECT(token::TOKEN::CLOSE_PARANTHESES);
  } else {
    success = false;
    error_messages.emplace_back(
        "Expected constant, unary operator or open parantheses but got " +
        to_string(tokens[0].get_token()));
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
    MAKE_SHARED(ast::AST_binop_Node, binop);
    parse_binop(tokens, binop);
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

void parser::parse_binop(std::vector<token::Token> &tokens,
                         std::shared_ptr<ast::AST_binop_Node> binop) {
  if (tokens[0].get_token() == token::TOKEN::PLUS) {
    binop->set_op(binop::BINOP::ADD);
    tokens.erase(tokens.begin());
  } else if (tokens[0].get_token() == token::TOKEN::PERCENT_SIGN) {
    binop->set_op(binop::BINOP::MOD);
    tokens.erase(tokens.begin());
  } else if (tokens[0].get_token() == token::TOKEN::FORWARD_SLASH) {
    binop->set_op(binop::BINOP::DIV);
    tokens.erase(tokens.begin());
  } else if (tokens[0].get_token() == token::TOKEN::ASTERISK) {
    binop->set_op(binop::BINOP::MUL);
    tokens.erase(tokens.begin());
  } else if (tokens[0].get_token() == token::TOKEN::HYPHEN) {
    binop->set_op(binop::BINOP::SUB);
    tokens.erase(tokens.begin());
  } else if (tokens[0].get_token() == token::TOKEN::AAND) {
    binop->set_op(binop::BINOP::AAND);
    tokens.erase(tokens.begin());
  } else if (tokens[0].get_token() == token::TOKEN::AOR) {
    binop->set_op(binop::BINOP::AOR);
    tokens.erase(tokens.begin());
  } else if (tokens[0].get_token() == token::TOKEN::XOR) {
    binop->set_op(binop::BINOP::XOR);
    tokens.erase(tokens.begin());
  } else if (tokens[0].get_token() == token::TOKEN::LEFT_SHIFT) {
    binop->set_op(binop::BINOP::LEFT_SHIFT);
    tokens.erase(tokens.begin());
  } else if (tokens[0].get_token() == token::TOKEN::RIGHT_SHIFT) {
    binop->set_op(binop::BINOP::RIGHT_SHIFT);
    tokens.erase(tokens.begin());
  } else if (tokens[0].get_token() == token::TOKEN::LAND) {
    binop->set_op(binop::BINOP::LAND);
    tokens.erase(tokens.begin());
  } else if (tokens[0].get_token() == token::TOKEN::LOR) {
    binop->set_op(binop::BINOP::LOR);
    tokens.erase(tokens.begin());
  } else if (tokens[0].get_token() == token::TOKEN::EQUAL) {
    binop->set_op(binop::BINOP::EQUAL);
    tokens.erase(tokens.begin());
  } else if (tokens[0].get_token() == token::TOKEN::NOTEQUAL) {
    binop->set_op(binop::BINOP::NOTEQUAL);
    tokens.erase(tokens.begin());
  } else if (tokens[0].get_token() == token::TOKEN::LESSTHAN) {
    binop->set_op(binop::BINOP::LESSTHAN);
    tokens.erase(tokens.begin());
  } else if (tokens[0].get_token() == token::TOKEN::GREATERTHAN) {
    binop->set_op(binop::BINOP::GREATERTHAN);
    tokens.erase(tokens.begin());
  } else if (tokens[0].get_token() == token::TOKEN::LESSTHANEQUAL) {
    binop->set_op(binop::BINOP::LESSTHANEQUAL);
    tokens.erase(tokens.begin());
  } else if (tokens[0].get_token() == token::TOKEN::GREATERTHANEQUAL) {
    binop->set_op(binop::BINOP::GREATERTHANEQUAL);
    tokens.erase(tokens.begin());
  } else {
    success = false;
    error_messages.emplace_back("Expected binary operator but got " +
                                to_string(tokens[0].get_token()));
  }
}

void parser::parse_unary_op(std::vector<token::Token> &tokens,
                            std::shared_ptr<ast::AST_factor_Node> factor) {
  if (tokens[0].get_token() == token::TOKEN::TILDE) {
    MAKE_SHARED(ast::AST_unop_Node, unop);
    unop->set_op(unop::UNOP::COMPLEMENT);
    factor->set_unop_node(std::move(unop));
    tokens.erase(tokens.begin());
  } else if (tokens[0].get_token() == token::TOKEN::HYPHEN) {
    MAKE_SHARED(ast::AST_unop_Node, unop);
    unop->set_op(unop::UNOP::NEGATE);
    factor->set_unop_node(std::move(unop));
    tokens.erase(tokens.begin());
  } else if (tokens[0].get_token() == token::TOKEN::NOT) {
    MAKE_SHARED(ast::AST_unop_Node, unop);
    unop->set_op(unop::UNOP::NOT);
    factor->set_unop_node(std::move(unop));
    tokens.erase(tokens.begin());
  } else {
    success = false;
    error_messages.emplace_back("Expected unary operator but got " +
                                to_string(tokens[0].get_token()));
  }
}

void parser::parse_identifier(
    std::vector<token::Token> &tokens,
    std::shared_ptr<ast::AST_Function_Node> function) {
  EXPECT_IDENTIFIER(token::TOKEN::IDENTIFIER);
}

void parser::parse_int(std::vector<token::Token> &tokens,
                       std::shared_ptr<ast::AST_factor_Node> factor) {
  EXPECT_INT(token::TOKEN::CONSTANT);
}

void parser::expect(token::TOKEN actual_token, token::TOKEN expected_token) {
  if (actual_token != expected_token) {
    success = false;
    error_messages.emplace_back("Expected token " + to_string(expected_token) +
                                " but got " + to_string(actual_token));
  }
}

void parser::display_errors() {
  for (auto error : error_messages) {
    std::cerr << error << std::endl;
  }
}

void parser::eof_error(token::Token token) {
  success = false;
  error_messages.emplace_back("Expected " + to_string(token.get_token()) +
                              " but got end of file");
}

void parser::pretty_print_factor(std::shared_ptr<ast::AST_factor_Node> factor) {
  if (!factor->get_unop_nodes().empty()) {
    std::cerr << "Unop( ";
    for (auto unop : factor->get_unop_nodes()) {
      std::cerr << unop::to_string(unop->get_op()) << ", ";
    }
  }
  if (factor->get_exp_node() != nullptr) {
    pretty_print_exp(factor->get_exp_node());
  } else {
    std::cerr << factor->get_int_node()->get_AST_name() << "("
              << factor->get_int_node()->get_value() << ")";
    if (!factor->get_unop_nodes().empty()) {
      std::cerr << ")";
    }
  }
}

void parser::pretty_print_exp(std::shared_ptr<ast::AST_exp_Node> exp) {
  if (exp == nullptr)
    return;
  pretty_print_exp(exp->get_left());
  if (exp->get_binop_node() != nullptr and
      exp->get_binop_node()->get_op() != binop::BINOP::UNKNOWN) {
    std::cerr << "\t\t\t\tBinop("
              << binop::to_string(exp->get_binop_node()->get_op()) << " ,";
    if (exp->get_left() == nullptr) {
      pretty_print_factor(exp->get_factor_node());
    } else {
      std::cerr << "Earlier, ";
    }
    pretty_print_exp(exp->get_right());
    std::cerr << ")" << std::endl;
  } else {
    std::cerr << "\t\t\t\t";
    pretty_print_factor(exp->get_factor_node());
    std::cerr << std::endl;
  }
}

void parser::pretty_print() {
  std::cerr << "Program(" << std::endl;
  for (auto function : program.get_functions()) {
    std::cerr << "\tFunction(" << std::endl;
    std::cerr << "\t\tname=\"" << function->get_identifier().get_value()
              << "\"," << std::endl;
    std::cerr << "\t\tbody=[" << std::endl;
    for (auto statement : function->get_statements()) {
      std::cerr << "\t\t\t" << statement->get_type() << "(" << std::endl;
      for (std::shared_ptr<ast::AST_exp_Node> exp : statement->get_exps()) {
        pretty_print_exp(exp);
      }
      std::cerr << "\t\t\t)," << std::endl;
    }
    std::cerr << "\t\t]" << std::endl;
    std::cerr << "\t)," << std::endl;
  }
  std::cerr << ")" << std::endl;
}

} // namespace parser
} // namespace scarlet
