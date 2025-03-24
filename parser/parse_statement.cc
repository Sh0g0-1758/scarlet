#include "common.hh"

namespace scarlet {
namespace parser {

void parser::parse_statement(
    std::vector<token::Token> &tokens,
    std::shared_ptr<ast::AST_Statement_Node> &statement) {
  // NOTE THESE CHECKS ARE NECESSARY BECAUSE THE FUNCTION CAN BE CALLED
  // RECURSIVELY
  if (!success)
    return;
  if (tokens.empty()) {
    eof_error(token::TOKEN::SEMICOLON);
    return;
  }
  // ===============================

  if (tokens[0].get_token() == token::TOKEN::RETURN) {
    tokens.erase(tokens.begin());
    statement->set_type(ast::statementType::RETURN);

    MAKE_SHARED(ast::AST_exp_Node, exp);
    parse_exp(tokens, exp);
    statement->set_exp(std::move(exp));

    EXPECT(token::TOKEN::SEMICOLON);
  } else if (tokens[0].get_token() == token::TOKEN::IF) {
    // We first check for the if (exp) statement
    MAKE_SHARED(ast::AST_if_else_statement_Node, if_else_statement);
    tokens.erase(tokens.begin());

    EXPECT(token::TOKEN::OPEN_PARANTHESES);

    // exp
    MAKE_SHARED(ast::AST_exp_Node, exp);
    parse_exp(tokens, exp);
    if_else_statement->set_exp(std::move(exp));

    EXPECT(token::TOKEN::CLOSE_PARANTHESES);

    // statement
    MAKE_SHARED(ast::AST_Statement_Node, stmt);
    parse_statement(tokens, stmt);
    if_else_statement->set_stmt1(std::move(stmt));

    if_else_statement->set_labels(get_ifelse_labels());

    // Then we optionally check for the else statement
    if (tokens[0].get_token() == token::TOKEN::ELSE) {
      tokens.erase(tokens.begin());
      if_else_statement->set_type(ast::statementType::IFELSE);

      MAKE_SHARED(ast::AST_Statement_Node, stmt2);
      parse_statement(tokens, stmt2);

      if_else_statement->set_stmt2(std::move(stmt2));
    } else {
      if_else_statement->set_type(ast::statementType::IF);
    }
    statement =
        std::static_pointer_cast<ast::AST_Statement_Node>(if_else_statement);
  } else if (tokens[0].get_token() == token::TOKEN::SEMICOLON) {
    // ignore the empty statement
    tokens.erase(tokens.begin());
    statement->set_type(ast::statementType::NULLSTMT);
  } else if (tokens[0].get_token() == token::TOKEN::GOTO) {
    tokens.erase(tokens.begin());
    statement->set_type(ast::statementType::GOTO);
    EXPECT_IDENTIFIER();
    identifier->set_identifier(currFuncName + "." + identifier->get_value());
    statement->set_labels({std::move(identifier), nullptr});
    EXPECT(token::TOKEN::SEMICOLON);
  } else if (tokens.size() >= 2 &&
             tokens[0].get_token() == token::TOKEN::IDENTIFIER &&
             tokens[1].get_token() == token::TOKEN::COLON) {
    statement->set_type(ast::statementType::LABEL);
    EXPECT_IDENTIFIER();
    identifier->set_identifier(currFuncName + "." + identifier->get_value());
    statement->set_labels({std::move(identifier), nullptr});
    EXPECT(token::TOKEN::COLON);
  } else if (tokens[0].get_token() == token::TOKEN::OPEN_BRACE) {
    MAKE_SHARED(ast::AST_block_statement_node, block_stmt);
    MAKE_SHARED(ast::AST_Block_Node, block_node);
    parse_block(tokens, block_node);
    block_stmt->set_type(ast::statementType::BLOCK);
    block_stmt->set_block(std::move(block_node));
    statement = std::static_pointer_cast<ast::AST_Statement_Node>(block_stmt);
  } else if (tokens[0].get_token() == token::TOKEN::WHILE) {
    MAKE_SHARED(ast::AST_while_statement_Node, while_statement);
    while_statement->set_type(ast::statementType::WHILE);
    tokens.erase(tokens.begin());
    while_statement->set_labels(get_loop_labels());

    EXPECT(token::TOKEN::OPEN_PARANTHESES);
    MAKE_SHARED(ast::AST_exp_Node, exp);
    parse_exp(tokens, exp);
    while_statement->set_exp(std::move(exp));

    EXPECT(token::TOKEN::CLOSE_PARANTHESES);

    MAKE_SHARED(ast::AST_Statement_Node, stmt);
    parse_statement(tokens, stmt);
    while_statement->set_stmt(std::move(stmt));

    remove_loop_labels();
    statement =
        std::static_pointer_cast<ast::AST_Statement_Node>(while_statement);
  } else if (tokens[0].get_token() == token::TOKEN::SWITCH) {
    MAKE_SHARED(ast::AST_switch_statement_Node, switch_statement);
    switch_statement->set_type(ast::statementType::SWITCH);

    switch_stack.push(switch_statement);

    tokens.erase(tokens.begin());
    switch_statement->set_labels(get_switch_loop_labels());

    EXPECT(token::TOKEN::OPEN_PARANTHESES);
    MAKE_SHARED(ast::AST_exp_Node, exp);
    parse_exp(tokens, exp);
    switch_statement->set_exp(std::move(exp));
    EXPECT(token::TOKEN::CLOSE_PARANTHESES);

    MAKE_SHARED(ast::AST_Statement_Node, stmt);
    parse_statement(tokens, stmt);
    switch_statement->set_stmt(std::move(stmt));
    remove_switch_loop_label();

    statement =
        std::static_pointer_cast<ast::AST_Statement_Node>(switch_statement);
    switch_stack.pop();

  } else if (tokens[0].get_token() == token::TOKEN::CASE) {
    MAKE_SHARED(ast::AST_case_statement_Node, case_statement);
    case_statement->set_type(ast::statementType::CASE);
    tokens.erase(tokens.begin());
    MAKE_SHARED(ast::AST_exp_Node, exp);
    parse_exp(tokens, exp);

    if (exp->get_factor_node() == nullptr) {
      error_messages.emplace_back("case expression is empty");
      success = false;
    }

    case_statement->set_labels({get_case_label(), nullptr});
    MAKE_SHARED(ast::AST_identifier_Node, label);
    label = case_statement->get_labels().first;

    if (switch_stack.empty()) {
      error_messages.emplace_back(
          "case statement found outside of switch construct");
      success = false;
    } else {
      switch_stack.top()->set_case_exp_label(exp, label);
    }

    EXPECT(token::TOKEN::COLON);
    statement =
        std::static_pointer_cast<ast::AST_Statement_Node>(case_statement);
  } else if (tokens[0].get_token() == token::TOKEN::DEFAULT_CASE) {
    MAKE_SHARED(ast::AST_case_statement_Node, default_case_statement);
    default_case_statement->set_type(ast::statementType::DEFAULT_CASE);
    default_case_statement->set_labels({get_case_label(), nullptr});
    tokens.erase(tokens.begin());
    MAKE_SHARED(ast::AST_identifier_Node, label);
    label = default_case_statement->get_labels().first;
    if (switch_stack.empty()) {
      error_messages.emplace_back(
          "default case statement found outside of switch construct");
      success = false;
    } else {
      if (switch_stack.top()->get_has_default_case()) {
        success = false;
        error_messages.emplace_back(
            "Multiple default cases found in switch construct");
      } else {
        switch_stack.top()->set_has_default_case(true);
        switch_stack.top()->set_case_exp_label(nullptr, label);
      }
    }
    EXPECT(token::TOKEN::COLON);
    statement = std::static_pointer_cast<ast::AST_Statement_Node>(
        default_case_statement);
  } else if (tokens[0].get_token() == token::TOKEN::CONTINUE) {
    tokens.erase(tokens.begin());
    statement->set_type(ast::statementType::CONTINUE);
    auto lbl = get_prev_loop_start_label();
    if (lbl == nullptr) {
      error_messages.emplace_back(
          "Continue statement found outside of a loop construct");
      success = false;
    } else {
      statement->set_labels({std::move(lbl), nullptr});
    }
    EXPECT(token::TOKEN::SEMICOLON);
  } else if (tokens[0].get_token() == token::TOKEN::BREAK) {
    tokens.erase(tokens.begin());
    statement->set_type(ast::statementType::BREAK);
    auto lbl = get_prev_loop_end_label();
    if (lbl == nullptr) {
      error_messages.emplace_back(
          "Break statement found outside of a loop construct");
      success = false;
    } else {
      statement->set_labels({std::move(lbl), nullptr});
    }
    EXPECT(token::TOKEN::SEMICOLON);
  } else if (tokens[0].get_token() == token::TOKEN::DO) {
    tokens.erase(tokens.begin());
    MAKE_SHARED(ast::AST_while_statement_Node, while_statement);
    while_statement->set_type(ast::statementType::DO_WHILE);
    while_statement->set_labels(get_loop_labels());
    while_statement->set_start_label(get_loop_start_label());

    MAKE_SHARED(ast::AST_Statement_Node, stmt);
    parse_statement(tokens, stmt);
    while_statement->set_stmt(std::move(stmt));

    EXPECT(token::TOKEN::WHILE);
    EXPECT(token::TOKEN::OPEN_PARANTHESES);
    MAKE_SHARED(ast::AST_exp_Node, exp);
    parse_exp(tokens, exp);
    while_statement->set_exp(std::move(exp));
    EXPECT(token::TOKEN::CLOSE_PARANTHESES);
    EXPECT(token::TOKEN::SEMICOLON);

    remove_loop_labels();
    statement =
        std::static_pointer_cast<ast::AST_Statement_Node>(while_statement);
  } else if (tokens[0].get_token() == token::TOKEN::FOR) {
    // For statement has additional constructs, so we use a
    // child class AST_For_Statement_Node to represent it.
    // We will use the power of down and upcasting to handle this.
    // for ( <for-init> ; <exp> ; <exp> ) <statement>
    tokens.erase(tokens.begin());
    EXPECT(token::TOKEN::OPEN_PARANTHESES);

    MAKE_SHARED(ast::AST_For_Statement_Node, for_statement);
    for_statement->set_type(ast::statementType::FOR);
    if (tokens[0].get_token() != token::TOKEN::SEMICOLON) {
      parse_for_init(tokens, for_statement);
    } else {
      EXPECT(token::TOKEN::SEMICOLON);
    }

    if (tokens[0].get_token() != token::TOKEN::SEMICOLON) {
      MAKE_SHARED(ast::AST_exp_Node, exp);
      parse_exp(tokens, exp);
      for_statement->set_exp(std::move(exp));
    }

    EXPECT(token::TOKEN::SEMICOLON);

    if (tokens[0].get_token() != token::TOKEN::CLOSE_PARANTHESES) {
      MAKE_SHARED(ast::AST_exp_Node, exp);
      parse_exp(tokens, exp);
      for_statement->set_exp2(std::move(exp));
    }

    EXPECT(token::TOKEN::CLOSE_PARANTHESES);

    for_statement->set_labels(get_loop_labels());
    for_statement->set_start_label(get_loop_start_label());

    MAKE_SHARED(ast::AST_Statement_Node, stmt);
    parse_statement(tokens, stmt);
    for_statement->set_stmt(std::move(stmt));

    remove_loop_labels();
    statement =
        std::static_pointer_cast<ast::AST_Statement_Node>(for_statement);
  } else {
    statement->set_type(ast::statementType::EXP);
    MAKE_SHARED(ast::AST_exp_Node, exp);
    parse_exp(tokens, exp);
    statement->set_exp(std::move(exp));
    EXPECT(token::TOKEN::SEMICOLON);
  }
}

void parser::parse_for_init(
    std::vector<token::Token> &tokens,
    std::shared_ptr<ast::AST_For_Statement_Node> &forstmt) {
  MAKE_SHARED(ast::AST_For_Init_Node, for_init);
  // for init cannot have storage specifiers
  if (token::is_type_specifier(tokens[0].get_token())) {
    MAKE_SHARED(ast::AST_variable_declaration_Node, varDecl);
    parse_variable_declaration(tokens, varDecl);
    auto decl = std::static_pointer_cast<ast::AST_Declaration_Node>(varDecl);
    decl->set_type(ast::DeclarationType::VARIABLE);
    for_init->set_declaration(std::move(decl));
  } else {
    MAKE_SHARED(ast::AST_exp_Node, exp);
    parse_exp(tokens, exp);
    for_init->set_exp(std::move(exp));
    EXPECT(token::TOKEN::SEMICOLON);
  }
  forstmt->set_for_init(std::move(for_init));
}

} // namespace parser
} // namespace scarlet
