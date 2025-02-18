#include "parser.hh"
namespace scarlet {
namespace parser {

#define MAKE_SHARED(a, b) std::shared_ptr<a> b = std::make_shared<a>()
#define UNREACHABLE()                                                          \
  std::cout << "Unreachable code reached in " << __FILE__ << " at line "       \
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
  MAKE_SHARED(ast::AST_Block_Node, block);
  parse_block(tokens, block);
  function->set_block(block);
  return function;
}

void parser::parse_block(std::vector<token::Token> &tokens,
                         std::shared_ptr<ast::AST_Block_Node> block) {
  EXPECT(token::TOKEN::OPEN_BRACE);
  while (!tokens.empty() and success and
         tokens[0].get_token() != token::TOKEN::CLOSE_BRACE) {
    parse_block_item(tokens, block);
  }
  EXPECT(token::TOKEN::CLOSE_BRACE);
}

void parser::parse_block_item(std::vector<token::Token> &tokens,
                              std::shared_ptr<ast::AST_Block_Node> block) {
  // We have a variable declaration / defintion
  if (tokens[0].get_token() == token::TOKEN::INT) {
    parse_declaration(tokens, block);
  } else {
    // we have a return statement, an expression, an if-else block or a null
    // statement
    parse_statement(tokens, block);
  }
}

void parser::parse_declaration(std::vector<token::Token> &tokens,
                               std::shared_ptr<ast::AST_Block_Node> block) {
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
  block->add_blockItem(std::move(block_item));
}

void parser::parse_statement(std::vector<token::Token> &tokens,
                             std::shared_ptr<ast::AST_Block_Node> block) {
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
    block->add_blockItem(std::move(block_item));
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
    block->add_blockItem(std::move(block_item));

    // statement
    parse_statement(tokens, block);

    // mark the end of this if statement
    MAKE_SHARED(ast::AST_Block_Item_Node, block_item2);
    block_item2->set_type(ast::BlockItemType::STATEMENT);
    MAKE_SHARED(ast::AST_Statement_Node, statement2);
    statement2->set_type(ast::statementType::_IF_END);
    block_item2->set_statement(std::move(statement2));
    block->add_blockItem(std::move(block_item2));

    // Then we optionally check for the else statement
    if (tokens[0].get_token() == token::TOKEN::ELSE) {
      tokens.erase(tokens.begin());
      statement->set_type(ast::statementType::IFELSE);
      // the statement following else is being treated as another block item
      parse_statement(tokens, block);

      // mark the end of this else statement
      MAKE_SHARED(ast::AST_Block_Item_Node, block_item3);
      block_item3->set_type(ast::BlockItemType::STATEMENT);
      MAKE_SHARED(ast::AST_Statement_Node, statement3);
      statement3->set_type(ast::statementType::_IFELSE_END);
      block_item3->set_statement(std::move(statement3));
      block->add_blockItem(std::move(block_item3));
    }
  } else if (tokens[0].get_token() == token::TOKEN::SEMICOLON) {
    // ignore the empty statement
    tokens.erase(tokens.begin());
    return;
  } else if (tokens[0].get_token() == token::TOKEN::GOTO) {
    tokens.erase(tokens.begin());
    statement->set_type(ast::statementType::GOTO);
    EXPECT_IDENTIFIER();
    statement->set_labels({std::move(identifier), nullptr});
    EXPECT(token::TOKEN::SEMICOLON);
    block_item->set_statement(std::move(statement));
    block->add_blockItem(std::move(block_item));
  } else if (tokens.size() >= 2 &&
             tokens[0].get_token() == token::TOKEN::IDENTIFIER &&
             tokens[1].get_token() == token::TOKEN::COLON) {
    statement->set_type(ast::statementType::LABEL);
    EXPECT_IDENTIFIER();
    statement->set_labels({std::move(identifier), nullptr});
    EXPECT(token::TOKEN::COLON);
    block_item->set_statement(std::move(statement));
    block->add_blockItem(std::move(block_item));
  } else if (tokens[0].get_token() == token::TOKEN::OPEN_BRACE) {
    MAKE_SHARED(ast::AST_Block_Node, block_node);
    parse_block(tokens, block_node);
    statement->set_type(ast::statementType::BLOCK);
    statement->set_block(std::move(block_node));
    block_item->set_statement(std::move(statement));
    block->add_blockItem(std::move(block_item));
  } else if (tokens[0].get_token() == token::TOKEN::WHILE) {
    statement->set_type(ast::statementType::WHILE);
    MAKE_SHARED(ast::AST_identifier_Node, start_label);
    MAKE_SHARED(ast::AST_identifier_Node, end_label);
    start_label->set_identifier(get_loop_start_label());
    end_label->set_identifier(get_loop_end_label());
    statement->set_labels({std::move(start_label), end_label});
    tokens.erase(tokens.begin());
    EXPECT(token::TOKEN::OPEN_PARANTHESES);
    MAKE_SHARED(ast::AST_exp_Node, exp);
    parse_exp(tokens, exp);
    statement->set_exp(std::move(exp));
    EXPECT(token::TOKEN::CLOSE_PARANTHESES);
    block_item->set_statement(std::move(statement));
    block->add_blockItem(std::move(block_item));
    parse_statement(tokens, block);

    // make the end label
    MAKE_SHARED(ast::AST_Block_Item_Node, block_item2);
    block_item2->set_type(ast::BlockItemType::STATEMENT);
    MAKE_SHARED(ast::AST_Statement_Node, statement2);
    statement2->set_type(ast::statementType::LABEL);
    statement2->set_labels({std::move(end_label), nullptr});
    block_item2->set_statement(std::move(statement2));
    block->add_blockItem(std::move(block_item2));

    remove_loop_start_label();
    remove_loop_end_label();
  } else if (tokens[0].get_token() == token::TOKEN::CONTINUE) {
    statement->set_type(ast::statementType::CONTINUE);
    MAKE_SHARED(ast::AST_identifier_Node, label);
    auto [lbl, res] = get_prev_loop_start_label();
    if (!res) {
      tokens.erase(tokens.begin());
      error_messages.emplace_back(
          "Continue statement found outside of a loop construct");
      success = false;
    } else {
      label->set_identifier(lbl);
      statement->set_labels({std::move(label), nullptr});
      tokens.erase(tokens.begin());
      EXPECT(token::TOKEN::SEMICOLON);
      block_item->set_statement(std::move(statement));
      block->add_blockItem(std::move(block_item));
    }
  } else if (tokens[0].get_token() == token::TOKEN::BREAK) {
    statement->set_type(ast::statementType::BREAK);
    MAKE_SHARED(ast::AST_identifier_Node, label);
    auto [lbl, res] = get_prev_loop_end_label();
    if (!res) {
      tokens.erase(tokens.begin());
      error_messages.emplace_back(
          "Break statement found outside of a loop construct");
      success = false;
    } else {
      label->set_identifier(lbl);
      statement->set_labels({std::move(label), nullptr});
      tokens.erase(tokens.begin());
      EXPECT(token::TOKEN::SEMICOLON);
      block_item->set_statement(std::move(statement));
      block->add_blockItem(std::move(block_item));
    }
  } else if (tokens[0].get_token() == token::TOKEN::DO) {
    tokens.erase(tokens.begin());
    MAKE_SHARED(ast::AST_identifier_Node, start_label);
    MAKE_SHARED(ast::AST_identifier_Node, end_label);
    start_label->set_identifier(get_loop_start_label());
    end_label->set_identifier(get_loop_end_label());

    // make the start label
    MAKE_SHARED(ast::AST_Block_Item_Node, block_item2);
    block_item2->set_type(ast::BlockItemType::STATEMENT);
    MAKE_SHARED(ast::AST_Statement_Node, statement2);
    statement2->set_type(ast::statementType::LABEL);
    statement2->set_labels({start_label, nullptr});
    block_item2->set_statement(std::move(statement2));
    block->add_blockItem(std::move(block_item2));

    statement->set_labels({std::move(start_label), std::move(end_label)});

    parse_statement(tokens, block);

    EXPECT(token::TOKEN::WHILE);
    EXPECT(token::TOKEN::OPEN_PARANTHESES);
    MAKE_SHARED(ast::AST_exp_Node, exp);
    parse_exp(tokens, exp);
    EXPECT(token::TOKEN::CLOSE_PARANTHESES);
    EXPECT(token::TOKEN::SEMICOLON);
    statement->set_exp(std::move(exp));
    statement->set_type(ast::statementType::DO_WHILE);
    block_item->set_statement(std::move(statement));
    block->add_blockItem(std::move(block_item));

    remove_loop_start_label();
    remove_loop_end_label();
  } else if (tokens[0].get_token() == token::TOKEN::FOR) {
    // For statement has additional constructs, so we use a
    // child class AST_For_Statement_Node to represent it.
    // We will use the power of down and upcasting to handle this.
    // for ( <for-init> ; <exp> ; <exp> ) <statement>
    tokens.erase(tokens.begin());
    EXPECT(token::TOKEN::OPEN_PARANTHESES);

    MAKE_SHARED(ast::AST_For_Statement_Node, for_statement);
    for_statement->set_type(ast::statementType::FOR);
    if (tokens[0].get_token() != token::TOKEN::SEMICOLON)
      parse_for_init(tokens, for_statement);

    EXPECT(token::TOKEN::SEMICOLON);

    if (tokens[0].get_token() != token::TOKEN::SEMICOLON) {
      MAKE_SHARED(ast::AST_exp_Node, exp);
      parse_exp(tokens, exp);
      for_statement->set_exp(std::move(exp));
    } else {
      for_statement->set_exp(nullptr);
    }

    EXPECT(token::TOKEN::SEMICOLON);

    if (tokens[0].get_token() != token::TOKEN::CLOSE_PARANTHESES) {
      MAKE_SHARED(ast::AST_exp_Node, exp2);
      parse_exp(tokens, exp2);
      for_statement->set_exp2(std::move(exp2));
    } else {
      for_statement->set_exp2(nullptr);
    }

    EXPECT(token::TOKEN::CLOSE_PARANTHESES);

    MAKE_SHARED(ast::AST_identifier_Node, start_label);
    MAKE_SHARED(ast::AST_identifier_Node, end_label);
    start_label->set_identifier(get_loop_start_label());
    end_label->set_identifier(get_loop_end_label());

    for_statement->set_labels({std::move(start_label), end_label});

    statement =
        std::static_pointer_cast<ast::AST_Statement_Node>(for_statement);
    block_item->set_statement(std::move(statement));
    block->add_blockItem(std::move(block_item));

    parse_statement(tokens, block);

    // make the end label
    MAKE_SHARED(ast::AST_Block_Item_Node, block_item2);
    block_item2->set_type(ast::BlockItemType::STATEMENT);
    MAKE_SHARED(ast::AST_Statement_Node, statement2);
    statement2->set_type(ast::statementType::LABEL);
    statement2->set_labels({std::move(end_label), nullptr});
    block_item2->set_statement(std::move(statement2));
    block->add_blockItem(std::move(block_item2));

    remove_loop_start_label();
    remove_loop_end_label();
  } else {
    statement->set_type(ast::statementType::EXP);
    MAKE_SHARED(ast::AST_exp_Node, exp);
    parse_exp(tokens, exp);
    statement->set_exp(std::move(exp));
    EXPECT(token::TOKEN::SEMICOLON);
    block_item->set_statement(std::move(statement));
    block->add_blockItem(std::move(block_item));
  }
}

void parser::parse_for_init(
    std::vector<token::Token> &tokens,
    std::shared_ptr<ast::AST_For_Statement_Node> forstmt) {
  MAKE_SHARED(ast::AST_For_Init_Node, for_init);
  if (tokens[0].get_token() == token::TOKEN::INT) {
    tokens.erase(tokens.begin());
    EXPECT_IDENTIFIER();
    MAKE_SHARED(ast::AST_Declaration_Node, declaration);
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
    }
    for_init->set_declaration(std::move(declaration));
  } else {
    MAKE_SHARED(ast::AST_exp_Node, exp);
    parse_exp(tokens, exp);
    for_init->set_exp(std::move(exp));
  }
  forstmt->set_for_init(std::move(for_init));
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
      tokens.erase(tokens.begin());
      MAKE_SHARED(ast::AST_exp_Node, exp);
      parse_exp(tokens, exp);
      factor->set_exp_node(std::move(exp));
      EXPECT(token::TOKEN::CLOSE_PARANTHESES);
    }
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

void parser::analyze_exp(
    std::shared_ptr<ast::AST_exp_Node> exp,
    std::map<std::pair<std::string, int>, std::string> &symbol_table,
    int indx) {
  if (exp == nullptr)
    return;
  analyze_exp(exp->get_left(), symbol_table, indx);
  // here we check that the factor of the expresssion is not an undeclared
  // variable
  if (exp->get_factor_node() != nullptr and
      exp->get_factor_node()->get_identifier_node() != nullptr) {
    std::string var_name =
        exp->get_factor_node()->get_identifier_node()->get_value();
    int level = indx;
    bool found = false;
    while (level >= 0) {
      if (symbol_table.find({var_name, level}) != symbol_table.end()) {
        exp->get_factor_node()->get_identifier_node()->set_identifier(
            symbol_table[{var_name, level}]);
        found = true;
        break;
      }
      level--;
    }
    if (!found) {
      success = false;
      error_messages.emplace_back("Variable " + var_name + " not declared");
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
    analyze_exp(exp->get_factor_node()->get_exp_node(), symbol_table, indx);
  // now we recursively check the right side of the expression
  if (exp->get_right() != nullptr)
    analyze_exp(exp->get_right(), symbol_table, indx);
  // and a recursive check for the middle expression -> special case(ternary
  // operator)
  if (exp->get_middle() != nullptr)
    analyze_exp(exp->get_middle(), symbol_table, indx);
}

// NOTE: symbol table here is a map from {variable_name, block_indx} ->
// temporary_variable_name(used as scar registers later)
void parser::analyze_block(
    std::shared_ptr<ast::AST_Block_Node> block,
    std::map<std::pair<std::string, int>, std::string> &symbol_table,
    int indx) {
  if (block == nullptr)
    return;
  for (auto blockItem : block->get_blockItems()) {
    if (blockItem->get_type() == ast::BlockItemType::DECLARATION) {
      std::string var_name =
          blockItem->get_declaration()->get_identifier()->get_value();
      if (symbol_table.find({var_name, indx}) != symbol_table.end()) {
        // the symbol has been declared twice which is illegal
        success = false;
        error_messages.emplace_back("Variable " + var_name +
                                    " already declared");
      } else {
        symbol_table[{var_name, indx}] = get_temp_name(var_name);
        blockItem->get_declaration()->get_identifier()->set_identifier(
            symbol_table[{var_name, indx}]);
        if (blockItem->get_declaration()->get_exp() != nullptr)
          analyze_exp(blockItem->get_declaration()->get_exp(), symbol_table,
                      indx);
      }
    } else if (blockItem->get_type() == ast::BlockItemType::STATEMENT) {
      // every variable that is used in any of the statement should already
      // have been declared.

      // We need to check that the labels used in goto statements are
      // actually declared and we also need to make sure that there are no
      // duplicate labels
      if (blockItem->get_statement()->get_type() == ast::statementType::GOTO) {
        std::string label =
            blockItem->get_statement()->get_labels().first->get_value();
        if (goto_labels.find(label) == goto_labels.end()) {
          goto_labels[label] = false;
        }
        continue;
      }

      if (blockItem->get_statement()->get_type() == ast::statementType::LABEL) {
        std::string label =
            blockItem->get_statement()->get_labels().first->get_value();
        if (goto_labels.find(label) != goto_labels.end()) {
          if (goto_labels[label] == false) {
            goto_labels[label] = true;
          } else {
            success = false;
            error_messages.emplace_back("Label " + label + " already declared");
          }
        } else {
          goto_labels[label] = true;
        }
        continue;
      }
      // downcast for loop statement
      if (blockItem->get_statement()->get_type() == ast::statementType::FOR) {
        auto forstmt = std::static_pointer_cast<ast::AST_For_Statement_Node>(
            blockItem->get_statement());
        std::map<std::pair<std::string, int>, std::string> proxy_symbol_table(
            symbol_table);
        if (forstmt->get_for_init()->get_declaration() != nullptr) {
          analyze_exp(forstmt->get_for_init()->get_declaration()->get_exp(),
                      proxy_symbol_table, indx);
        }
        if (forstmt->get_exps() != nullptr) {
          analyze_exp(forstmt->get_exps(), proxy_symbol_table, indx);
        }
        if (forstmt->get_exp2() != nullptr) {
          analyze_exp(forstmt->get_exp2(), proxy_symbol_table, indx);
        }
        continue;
      }
      if (blockItem->get_statement()->get_exps() != nullptr) {
        analyze_exp(blockItem->get_statement()->get_exps(), symbol_table, indx);
      }
      if (blockItem->get_statement()->get_block() != nullptr) {
        std::map<std::pair<std::string, int>, std::string> proxy_symbol_table(
            symbol_table);
        analyze_block(blockItem->get_statement()->get_block(),
                      proxy_symbol_table, indx + 1);
      }
    }
  }
}

void parser::semantic_analysis() {
  // variable resolution
  for (auto funcs : program.get_functions()) {
    std::map<std::pair<std::string, int>, std::string> symbol_table;
    analyze_block(funcs->get_block(), symbol_table, 0);
  }
  // Check that all varaibles are declared
  for (auto label : goto_labels) {
    if (label.second == false) {
      success = false;
      error_messages.emplace_back("Label " + label.first +
                                  " used but not declared");
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
  case ast::statementType::GOTO:
    return "Goto";
  case ast::statementType::LABEL:
    return "Label";
  case ast::statementType::BLOCK:
    return "Block";
  case ast::statementType::CONTINUE:
    return "Continue";
  case ast::statementType::BREAK:
    return "Break";
  case ast::statementType::WHILE:
    return "While";
  case ast::statementType::DO_WHILE:
    return "DoWhile";
  case ast::statementType::FOR:
    return "For";
  case ast::statementType::UNKNOWN:
    UNREACHABLE()
  }
  UNREACHABLE()
}

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
    std::cout << factor->get_identifier_node()->get_AST_name() << "("
              << factor->get_identifier_node()->get_value() << ")";
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
    if (exp->get_middle() != nullptr) {
      pretty_print_exp(exp->get_middle());
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

void parser::pretty_print_block(std::shared_ptr<ast::AST_Block_Node> block) {
  if (block == nullptr)
    return;
  std::cout << "\t\t\t" << "Block=(" << std::endl;
  for (auto blockItem : block->get_blockItems()) {
    std::cout << "\t\t\t\t" << to_string(blockItem->get_type()) << "("
              << std::endl;
    if (blockItem->get_type() == ast::BlockItemType::DECLARATION) {
      std::cout << "\t\t\t\t\tidentifier=\""
                << blockItem->get_declaration()->get_identifier()->get_value()
                << "\"," << std::endl;
      if (blockItem->get_declaration()->get_exp() != nullptr) {
        std::cout << "\t\t\t\t\texp=(" << std::endl;
        pretty_print_exp(blockItem->get_declaration()->get_exp());
        std::cout << "\t\t\t\t\t)," << std::endl;
      }
    } else {
      std::cout << "\t\t\t\t\ttype="
                << to_string(blockItem->get_statement()->get_type()) << ","
                << std::endl;
      if (blockItem->get_statement()->get_type() == ast::statementType::FOR) {
        std::shared_ptr<ast::AST_For_Statement_Node> forstmt =
            std::static_pointer_cast<ast::AST_For_Statement_Node>(
                blockItem->get_statement());
        std::cout << "\t\t\t\t\tlabels=("
                  << forstmt->get_labels().first->get_value() << ","
                  << forstmt->get_labels().second->get_value() << "),"
                  << std::endl;
        std::cout << "\t\t\t\t\tfor_init=(" << std::endl;
        if (forstmt->get_for_init() != nullptr) {
          if (forstmt->get_for_init()->get_declaration() != nullptr) {
            std::cout << "\t\t\t\t\t\tDeclaration=(" << std::endl;
            std::cout << "\t\t\t\t\t\t\tidentifier=\""
                      << forstmt->get_for_init()
                             ->get_declaration()
                             ->get_identifier()
                             ->get_value()
                      << "\"," << std::endl;
            if (forstmt->get_for_init()->get_declaration()->get_exp() !=
                nullptr) {
              std::cout << "\t\t\t\t\t\t\texp=(" << std::endl;
              pretty_print_exp(
                  forstmt->get_for_init()->get_declaration()->get_exp());
              std::cout << "\t\t\t\t\t\t\t)," << std::endl;
            }
            std::cout << "\t\t\t\t\t\t)," << std::endl;
          } else {
            std::cout << "\t\t\t\t\t\texp=(" << std::endl;
            pretty_print_exp(forstmt->get_for_init()->get_exp());
            std::cout << "\t\t\t\t\t\t)," << std::endl;
          }
        }
        std::cout << "\t\t\t\t\t)," << std::endl;

        std::cout << "\t\t\t\t\tcondition=(" << std::endl;
        if (forstmt->get_exps() != nullptr)
          pretty_print_exp(forstmt->get_exps());
        std::cout << "\t\t\t\t\t)," << std::endl;

        std::cout << "\t\t\t\t\tpost=(" << std::endl;
        if (forstmt->get_exp2() != nullptr)
          pretty_print_exp(forstmt->get_exp2());
        std::cout << "\t\t\t\t\t)," << std::endl;

        continue;
      }
      if (blockItem->get_statement()->get_block() != nullptr)
        pretty_print_block(blockItem->get_statement()->get_block());
      else {
        if (blockItem->get_statement()->get_exps() != nullptr) {
          std::cout << "\t\t\t\t\texp=(" << std::endl;
          pretty_print_exp(blockItem->get_statement()->get_exps());
          std::cout << "\t\t\t\t\t)," << std::endl;
        }
        if (blockItem->get_statement()->get_labels().first != nullptr) {
          std::cout
              << "\t\t\t\t\tlabels=("
              << blockItem->get_statement()->get_labels().first->get_value();
          if (blockItem->get_statement()->get_labels().second != nullptr) {
            std::cout << ",";
            std::cout
                << blockItem->get_statement()->get_labels().second->get_value()
                << ")" << std::endl;
          } else {
            std::cout << ")" << std::endl;
          }
        }
      }
    }
    std::cout << "\t\t\t\t)," << std::endl;
  }
  std::cout << "\t\t\t)" << std::endl;
}

void parser::pretty_print() {
  std::cout << "Program(" << std::endl;
  for (auto function : program.get_functions()) {
    std::cout << "\tFunction(" << std::endl;
    std::cout << "\t\tname=\"" << function->get_identifier()->get_value()
              << "\"," << std::endl;
    std::cout << "\t\tbody=[" << std::endl;
    pretty_print_block(function->get_block());
    std::cout << "\t\t]" << std::endl;
    std::cout << "\t)," << std::endl;
  }
  std::cout << ")" << std::endl;
}

} // namespace parser
} // namespace scarlet
