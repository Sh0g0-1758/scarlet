#include "parser.hh"
namespace scarlet {
namespace parser {

#define MAKE_SHARED(a, b) std::shared_ptr<a> b = std::make_shared<a>()
#define UNREACHABLE()                                                          \
  std::cerr << "Unreachable code reached in " << __FILE__ << " at line "       \
            << __LINE__ << std::endl;                                          \
  __builtin_unreachable();

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

#define EXPECT_IDENTIFIER()                                                    \
  if (!success) {                                                              \
    return;                                                                    \
  }                                                                            \
  if (tokens.empty()) {                                                        \
    eof_error(token::TOKEN::IDENTIFIER);                                       \
    return;                                                                    \
  }                                                                            \
  expect(tokens[0].get_token(), token::TOKEN::IDENTIFIER);                     \
  if (!success) {                                                              \
    return;                                                                    \
  }                                                                            \
  MAKE_SHARED(ast::AST_identifier_Node, identifier);                           \
  identifier->set_identifier(tokens[0].get_value().value());                   \
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
  // parse all the block items in the current scope
  while (!tokens.empty() and success and
         tokens[0].get_token() != token::TOKEN::CLOSE_BRACE) {
    parse_block_item(tokens, function);
  }
  EXPECT_FUNC(token::TOKEN::CLOSE_BRACE);
  return function;
}

void parser::parse_declaration(
    std::vector<token::Token> &tokens,
    std::shared_ptr<ast::AST_Function_Node> function) {
  MAKE_SHARED(ast::AST_Block_Item_Node, block_item);
  block_item->set_type(ast::BlockItemType::DECLARATION);
  MAKE_SHARED(ast::AST_Declaration_Node, declaration);
  EXPECT(token::TOKEN::INT);
  EXPECT_IDENTIFIER();
  declaration->set_identifier(std::move(identifier));
  if (tokens[0].get_token() == token::TOKEN::SEMICOLON) {
    // the variable just have a declaration
    tokens.erase(tokens.begin());
  } else {
    // the variable has a definition as well
    EXPECT(token::TOKEN::ASSIGNMENT);
    MAKE_SHARED(ast::AST_exp_Node, exp);
    parse_exp(tokens, exp);
    declaration->set_exp(std::move(exp));
    EXPECT(token::TOKEN::SEMICOLON);
  }
  block_item->set_declaration(std::move(declaration));
  function->add_blockItem(std::move(block_item));
}

void parser::parse_statement(std::vector<token::Token> &tokens,
                             std::shared_ptr<ast::AST_Function_Node> function) {
  // NOTE THESE CHECKS ARE NECESSARY BECAUSE THE FUNCTION CAN BE CALLED
  // RECURSIVELY
  if (!success)
    return;
  if (tokens.empty()) {
    eof_error(token::TOKEN::SEMICOLON);
    return;
  }
  // ===============================
  MAKE_SHARED(ast::AST_Block_Item_Node, block_item);
  block_item->set_type(ast::BlockItemType::STATEMENT);
  MAKE_SHARED(ast::AST_Statement_Node, statement);

  if (tokens[0].get_token() == token::TOKEN::RETURN) {
    tokens.erase(tokens.begin());
    statement->set_type(ast::statementType::RETURN);

    MAKE_SHARED(ast::AST_exp_Node, exp);
    parse_exp(tokens, exp);
    statement->set_exp(std::move(exp));

    EXPECT(token::TOKEN::SEMICOLON);

    block_item->set_statement(std::move(statement));
    function->add_blockItem(std::move(block_item));
  } else if (tokens[0].get_token() == token::TOKEN::IF) {
    // We first check for the if (exp) statement
    tokens.erase(tokens.begin());

    EXPECT(token::TOKEN::OPEN_PARANTHESES);

    // exp
    MAKE_SHARED(ast::AST_exp_Node, exp);
    parse_exp(tokens, exp);
    statement->set_type(ast::statementType::IF);
    statement->set_exp(std::move(exp));

    EXPECT(token::TOKEN::CLOSE_PARANTHESES);

    // Don't move it as we modify it later in case it has an else statement
    block_item->set_statement(statement);
    function->add_blockItem(std::move(block_item));

    // statement
    parse_statement(tokens, function);

    // mark the end of this if statement
    MAKE_SHARED(ast::AST_Block_Item_Node, block_item2);
    block_item2->set_type(ast::BlockItemType::STATEMENT);
    MAKE_SHARED(ast::AST_Statement_Node, statement2);
    statement2->set_type(ast::statementType::_IF_END);
    block_item2->set_statement(std::move(statement2));
    function->add_blockItem(std::move(block_item2));

    // Then we optionally check for the else statement
    if (tokens[0].get_token() == token::TOKEN::ELSE) {
      tokens.erase(tokens.begin());
      statement->set_type(ast::statementType::IFELSE);
      // the statement following else is being treated as another block item
      parse_statement(tokens, function);

      // mark the end of this else statement
      MAKE_SHARED(ast::AST_Block_Item_Node, block_item3);
      block_item3->set_type(ast::BlockItemType::STATEMENT);
      MAKE_SHARED(ast::AST_Statement_Node, statement3);
      statement3->set_type(ast::statementType::_IFELSE_END);
      block_item3->set_statement(std::move(statement3));
      function->add_blockItem(std::move(block_item3));
    }
  } else if (tokens[0].get_token() == token::TOKEN::SEMICOLON) {
    // ignore the empty statement
    tokens.erase(tokens.begin());
    return;
  } else {
    statement->set_type(ast::statementType::EXP);
    MAKE_SHARED(ast::AST_exp_Node, exp);
    parse_exp(tokens, exp);
    statement->set_exp(std::move(exp));
    EXPECT(token::TOKEN::SEMICOLON);
    block_item->set_statement(std::move(statement));
    function->add_blockItem(std::move(block_item));
  }
}

void parser::parse_block_item(
    std::vector<token::Token> &tokens,
    std::shared_ptr<ast::AST_Function_Node> function) {
  // We have a variable declaration / defintion
  if (tokens[0].get_token() == token::TOKEN::INT) {
    parse_declaration(tokens, function);
  } else {
    // we have a return statement, an expression, an if-else block or a null
    // statement
    parse_statement(tokens, function);
  }
}

void parser::parse_factor(std::vector<token::Token> &tokens,
                          std::shared_ptr<ast::AST_factor_Node> factor) {
  if (tokens[0].get_token() == token::TOKEN::CONSTANT) {
    parse_int(tokens, factor);
  } else if (tokens[0].get_token() == token::TOKEN::IDENTIFIER) {
    EXPECT_IDENTIFIER();
    factor->set_identifier_node(std::move(identifier));
  } else if (token::is_unary_op(tokens[0].get_token())) {
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
    error_messages.emplace_back("Expected constant, unary operator, semicolon "
                                "or open parantheses but got " +
                                to_string(tokens[0].get_token()));
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
      root_exp->set_middle(std::move(middle_exp));
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

void parser::parse_binop(std::vector<token::Token> &tokens,
                         std::shared_ptr<ast::AST_binop_Node> binop) {
  switch (tokens[0].get_token()) {
  case token::TOKEN::PLUS:
    binop->set_op(binop::BINOP::ADD);
    break;
  case token::TOKEN::PERCENT_SIGN:
    binop->set_op(binop::BINOP::MOD);
    break;
  case token::TOKEN::FORWARD_SLASH:
    binop->set_op(binop::BINOP::DIV);
    break;
  case token::TOKEN::ASTERISK:
    binop->set_op(binop::BINOP::MUL);
    break;
  case token::TOKEN::HYPHEN:
    binop->set_op(binop::BINOP::SUB);
    break;
  case token::TOKEN::AAND:
    binop->set_op(binop::BINOP::AAND);
    break;
  case token::TOKEN::AOR:
    binop->set_op(binop::BINOP::AOR);
    break;
  case token::TOKEN::XOR:
    binop->set_op(binop::BINOP::XOR);
    break;
  case token::TOKEN::LEFT_SHIFT:
    binop->set_op(binop::BINOP::LEFT_SHIFT);
    break;
  case token::TOKEN::RIGHT_SHIFT:
    binop->set_op(binop::BINOP::RIGHT_SHIFT);
    break;
  case token::TOKEN::LAND:
    binop->set_op(binop::BINOP::LAND);
    break;
  case token::TOKEN::LOR:
    binop->set_op(binop::BINOP::LOR);
    break;
  case token::TOKEN::EQUAL:
    binop->set_op(binop::BINOP::EQUAL);
    break;
  case token::TOKEN::NOTEQUAL:
    binop->set_op(binop::BINOP::NOTEQUAL);
    break;
  case token::TOKEN::LESSTHAN:
    binop->set_op(binop::BINOP::LESSTHAN);
    break;
  case token::TOKEN::GREATERTHAN:
    binop->set_op(binop::BINOP::GREATERTHAN);
    break;
  case token::TOKEN::LESSTHANEQUAL:
    binop->set_op(binop::BINOP::LESSTHANEQUAL);
    break;
  case token::TOKEN::GREATERTHANEQUAL:
    binop->set_op(binop::BINOP::GREATERTHANEQUAL);
    break;
  case token::TOKEN::ASSIGNMENT:
    binop->set_op(binop::BINOP::ASSIGN);
    break;
  case token::TOKEN::COMPOUND_DIFFERENCE:
    binop->set_op(binop::BINOP::COMPOUND_DIFFERENCE);
    break;
  case token::TOKEN::COMPOUND_DIVISION:
    binop->set_op(binop::BINOP::COMPOUND_DIVISION);
    break;
  case token::TOKEN::COMPOUND_PRODUCT:
    binop->set_op(binop::BINOP::COMPOUND_PRODUCT);
    break;
  case token::TOKEN::COMPOUND_REMAINDER:
    binop->set_op(binop::BINOP::COMPOUND_REMAINDER);
    break;
  case token::TOKEN::COMPOUND_SUM:
    binop->set_op(binop::BINOP::COMPOUND_SUM);
    break;
  case token::TOKEN::COMPOUND_AND:
    binop->set_op(binop::BINOP::COMPOUND_AND);
    break;
  case token::TOKEN::COMPOUND_OR:
    binop->set_op(binop::BINOP::COMPOUND_OR);
    break;
  case token::TOKEN::COMPOUND_XOR:
    binop->set_op(binop::BINOP::COMPOUND_XOR);
    break;
  case token::TOKEN::COMPOUND_LEFTSHIFT:
    binop->set_op(binop::BINOP::COMPOUND_LEFTSHIFT);
    break;
  case token::TOKEN::COMPOUND_RIGHTSHIFT:
    binop->set_op(binop::BINOP::COMPOUND_RIGHTSHIFT);
    break;
  case token::TOKEN::QUESTION_MARK:
    binop->set_op(binop::BINOP::TERNARY);
    break;
  default:
    success = false;
    error_messages.emplace_back("Expected binary operator but got " +
                                to_string(tokens[0].get_token()));
    break;
  }
  tokens.erase(tokens.begin());
}

void parser::parse_unary_op(std::vector<token::Token> &tokens,
                            std::shared_ptr<ast::AST_factor_Node> factor) {
  MAKE_SHARED(ast::AST_unop_Node, unop);
  switch (tokens[0].get_token()) {
  case token::TOKEN::TILDE:
    unop->set_op(unop::UNOP::COMPLEMENT);
    break;
  case token::TOKEN::HYPHEN:
    unop->set_op(unop::UNOP::NEGATE);
    break;
  case token::TOKEN::NOT:
    unop->set_op(unop::UNOP::NOT);
    break;
  case token::TOKEN::INCREMENT_OPERATOR:
    unop->set_op(unop::UNOP::PREINCREMENT);
    break;
  case token::TOKEN::DECREMENT_OPERATOR:
    unop->set_op(unop::UNOP::PREDECREMENT);
    break;
  default:
    UNREACHABLE()
  }
  factor->add_unop_node(std::move(unop));
  tokens.erase(tokens.begin());
}

void parser::parse_identifier(
    std::vector<token::Token> &tokens,
    std::shared_ptr<ast::AST_Function_Node> function) {
  EXPECT_IDENTIFIER();
  function->set_identifier(std::move(identifier));
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

void parser::analyze_exp(std::shared_ptr<ast::AST_exp_Node> exp) {
  if (exp == nullptr)
    return;
  analyze_exp(exp->get_left());
  // here we check that the factor of the expresssion is not an undeclared
  // variable
  if (exp->get_factor_node() != nullptr and
      exp->get_factor_node()->get_identifier_node() != nullptr) {
    std::string var_name =
        exp->get_factor_node()->get_identifier_node()->get_value();
    if (symbol_table.find(var_name) == symbol_table.end()) {
      success = false;
      error_messages.emplace_back("Variable " + var_name + " not declared");
    } else {
      exp->get_factor_node()->get_identifier_node()->set_identifier(
          symbol_table[var_name]);
    }
  }

  // now we check that if the exp is of type assignment, then factor is an
  // identifier
  if (exp->get_binop_node() != nullptr and
      (exp->get_binop_node()->get_op() == binop::BINOP::ASSIGN or
       binop::is_compound(exp->get_binop_node()->get_op()))) {
    // ERROR CONDITION: (no factor node) or (factor node is a constant, not a
    // variable) or (factor node is a variable but has unary operators) Here we
    // exploit the benefit of short circuiting power of the logical operator
    // this means that as we proceed, we are ensured that the earlier checks
    // must not be satisfied. Note that an identifier with unops makes it an
    // rvalue.
    if (exp->get_factor_node() == nullptr or
        exp->get_factor_node()->get_identifier_node() == nullptr or
        exp->get_factor_node()->get_unop_nodes().size() > 0) {
      success = false;
      error_messages.emplace_back("Expected a modifiable lvalue on the left "
                                  "side of the assignment operator");
    }
  }
  // SEMANTIC ANALYSIS FOR INCREMENT AND DECREMENT OPERATOR
  if (exp->get_factor_node() != nullptr) {
    auto check_factor = exp->get_factor_node();
    bool has_i_d = false;
    bool has_multiple_i_d = false;
    // CHECK WHETHER INCREMENT OR DECREMENT OPERATOR IS PRESENT
    for (auto it : check_factor->get_unop_nodes()) {
      if (it->get_op() == unop::UNOP::PREINCREMENT or
          it->get_op() == unop::UNOP::PREDECREMENT or
          it->get_op() == unop::UNOP::POSTINCREMENT or
          it->get_op() == unop::UNOP::POSTDECREMENT) {
        if (has_i_d) {
          has_multiple_i_d = true;
          break;
        }
        has_i_d = true;
      }
    }
    if (has_multiple_i_d) {
      success = false;
      error_messages.emplace_back(
          "Expected an lvalue for the increment / decrement operator");
    } else if (has_i_d) {
      // THE INCREMENT AND DECREMENT OPERATOR MUST BE APPLIED TO AN LVALUE
      // THAT MEANS IT SHOULD BE THE LAST UNOP IN THE UNOPS VECTOR
      if (check_factor->get_unop_nodes().back()->get_op() !=
              unop::UNOP::PREINCREMENT and
          check_factor->get_unop_nodes().back()->get_op() !=
              unop::UNOP::PREDECREMENT and
          check_factor->get_unop_nodes().back()->get_op() !=
              unop::UNOP::POSTINCREMENT and
          check_factor->get_unop_nodes().back()->get_op() !=
              unop::UNOP::POSTDECREMENT) {
        success = false;
        error_messages.emplace_back(
            "Expected an lvalue for the increment / decrement operator");
      } else {
        // NOW THERE ARE ONLY TWO VALID CASES
        // CASE 1: FACTOR HAS AN IDENTIFIER
        // CASE 2: FACTOR HAS A DEEPLY NESTED EXPRESSION WHICH THEN
        //         CONTAINS THE IDENTIFIER (eg. ++(((((((a))))))) )
        //         or ( (((((((a)))))))++ )
        if (check_factor->get_identifier_node() != nullptr) {
          // EARLIER CHECKS ENSURE THAT THE IDENTIFIER IS ALREADY DECLARED
        } else if (check_factor->get_exp_node() != nullptr) {
          // NOW WE RECURSIVELY CHECK THAT THE EXPRESSION IS A SIMPLE IDENTIFIER
          // AND NOT A COMPLEX EXPRESSION
          auto check_exp = check_factor->get_exp_node();
          while (check_exp != nullptr) {
            // ENSURE THAT BINOP FOR CHECK_EXP IS NULL
            // ENSURE THAT LEFT FOR CHECK_EXP IS NULL
            if (check_exp->get_binop_node() != nullptr or
                check_exp->get_left() != nullptr) {
              success = false;
              error_messages.emplace_back(
                  "Expected an lvalue for the increment / decrement operator");
              break;
            }
            // ENSURE THAT THERE ARE NO UNOPS AS WELL
            // WE ARE BASICALLY GUARANTEED THAT FACTOR IS NOT NULL
            if (check_exp->get_factor_node()->get_unop_nodes().size() > 0) {
              success = false;
              error_messages.emplace_back(
                  "Expected an lvalue for the increment / decrement operator");
              break;
            }
            // NOW WE CHECK THAT THE FACTOR IS AN IDENTIFIER
            if (exp->get_factor_node()->get_exp_node() == nullptr) {
              if (exp->get_factor_node()->get_identifier_node() == nullptr) {
                success = false;
                error_messages.emplace_back("Expected an lvalue for the "
                                            "increment / decrement operator");
                break;
              }
            }
            check_exp = check_exp->get_factor_node()->get_exp_node();
          }
        } else {
          success = false;
          error_messages.emplace_back(
              "Expected an lvalue for the increment / decrement operator");
        }
      }
    }
  }
  // since the factor can have its own exp as well, we recursively check that
  if (exp->get_factor_node() != nullptr)
    analyze_exp(exp->get_factor_node()->get_exp_node());
  // now we recursively check the right side of the expression
  analyze_exp(exp->get_right());
  // and a recursive check for the middle expression -> special case(ternary
  // operator)
  analyze_exp(exp->get_middle());
}

void parser::semantic_analysis() {
  // variable resolution
  for (auto funcs : program.get_functions()) {
    for (auto blockItem : funcs->get_blockItems()) {
      if (blockItem->get_type() == ast::BlockItemType::DECLARATION) {
        std::string var_name =
            blockItem->get_declaration()->get_identifier()->get_value();
        if (symbol_table.find(var_name) != symbol_table.end()) {
          // the symbol has been declared twice which is illegal
          success = false;
          error_messages.emplace_back("Variable " + var_name +
                                      " already declared");
        } else {
          symbol_table[var_name] = get_temp_name(var_name);
          blockItem->get_declaration()->get_identifier()->set_identifier(
              symbol_table[var_name]);
          if (blockItem->get_declaration()->get_exp() != nullptr) {
            analyze_exp(blockItem->get_declaration()->get_exp());
          }
        }
      } else if (blockItem->get_type() == ast::BlockItemType::STATEMENT) {
        // every variable that is used in any of the statement should already
        // have been declared.
        analyze_exp(blockItem->get_statement()->get_exps());
      }
    }
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

std::string to_string(ast::BlockItemType type) {
  switch (type) {
  case ast::BlockItemType::STATEMENT:
    return "Statement";
  case ast::BlockItemType::DECLARATION:
    return "Declaration";
  case ast::BlockItemType::UNKNOWN:
    UNREACHABLE()
  }
  UNREACHABLE()
}

std::string to_string(ast::statementType type) {
  switch (type) {
  case ast::statementType::RETURN:
    return "Return";
  case ast::statementType::EXP:
    return "Exp";
  case ast::statementType::IF:
    return "If";
  case ast::statementType::IFELSE:
    return "IfElse";
  case ast::statementType::_IF_END:
    return "_If_End";
  case ast::statementType::_IFELSE_END:
    return "_IfElse_End";
  case ast::statementType::UNKNOWN:
    UNREACHABLE()
  }
  UNREACHABLE()
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
  } else if (factor->get_int_node() != nullptr) {
    std::cerr << factor->get_int_node()->get_AST_name() << "("
              << factor->get_int_node()->get_value() << ")";
    if (!factor->get_unop_nodes().empty()) {
      std::cerr << ")";
    }
  } else {
    std::cerr << factor->get_identifier_node()->get_AST_name() << "("
              << factor->get_identifier_node()->get_value() << ")";
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
    std::cerr << "\t\t\t\t\tBinop("
              << binop::to_string(exp->get_binop_node()->get_op()) << " ,";
    if (exp->get_left() == nullptr) {
      pretty_print_factor(exp->get_factor_node());
    } else {
      std::cerr << "Earlier, ";
    }
    if (exp->get_middle() != nullptr) {
      pretty_print_exp(exp->get_middle());
      std::cerr << ", ";
    }
    pretty_print_exp(exp->get_right());
    std::cerr << ")" << std::endl;
  } else {
    std::cerr << "\t\t\t\t\t";
    pretty_print_factor(exp->get_factor_node());
    std::cerr << std::endl;
  }
}

void parser::pretty_print() {
  std::cerr << "Program(" << std::endl;
  for (auto function : program.get_functions()) {
    std::cerr << "\tFunction(" << std::endl;
    std::cerr << "\t\tname=\"" << function->get_identifier()->get_value()
              << "\"," << std::endl;
    std::cerr << "\t\tbody=[" << std::endl;
    for (auto blockItem : function->get_blockItems()) {
      std::cerr << "\t\t\t" << to_string(blockItem->get_type()) << "("
                << std::endl;
      if (blockItem->get_type() == ast::BlockItemType::DECLARATION) {
        std::cerr << "\t\t\t\tidentifier=\""
                  << blockItem->get_declaration()->get_identifier()->get_value()
                  << "\"," << std::endl;
        if (blockItem->get_declaration()->get_exp() != nullptr) {
          std::cerr << "\t\t\t\texp=(" << std::endl;
          pretty_print_exp(blockItem->get_declaration()->get_exp());
          std::cerr << "\t\t\t\t)," << std::endl;
        }
      } else {
        std::cerr << "\t\t\t\ttype="
                  << to_string(blockItem->get_statement()->get_type()) << ","
                  << std::endl;
        std::cerr << "\t\t\t\texp=(" << std::endl;
        pretty_print_exp(blockItem->get_statement()->get_exps());
        std::cerr << "\t\t\t\t)," << std::endl;
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
