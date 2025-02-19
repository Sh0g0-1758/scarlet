#include "common.hh"
namespace scarlet {
namespace parser {

void parser::parse_program(std::vector<token::Token> tokens) {
  ast::AST_Program_Node program;
  while (!tokens.empty() and success) {
    program.add_function(parse_function(tokens));
  }
  this->program = program;
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

void parser::analyze_declaration(
    std::shared_ptr<ast::AST_Declaration_Node> declaration,
    std::map<std::pair<std::string, int>, std::string> &symbol_table,
    int indx) {
  std::string var_name = declaration->get_identifier()->get_value();
  if (symbol_table.find({var_name, indx}) != symbol_table.end()) {
    // the symbol has been declared twice which is illegal
    success = false;
    error_messages.emplace_back("Variable " + var_name + " already declared");
  } else {
    symbol_table[{var_name, indx}] = get_temp_name(var_name);
    declaration->get_identifier()->set_identifier(
        symbol_table[{var_name, indx}]);
    if (declaration->get_exp() != nullptr)
      analyze_exp(declaration->get_exp(), symbol_table, indx);
  }
}

void parser::analyze_statement(
    std::shared_ptr<ast::AST_Statement_Node> statement,
    std::map<std::pair<std::string, int>, std::string> &symbol_table,
    int indx) {

  // every variable that is used in any of the statement should already
  // have been declared.

  // We need to check that the labels used in goto statements are
  // actually declared and we also need to make sure that there are no
  // duplicate labels

  switch (statement->get_type()) {
  case ast::statementType::RETURN:
    analyze_exp(statement->get_exps(), symbol_table, indx);
    break;
  case ast::statementType::IF: {
    auto if_statement =
        std::static_pointer_cast<ast::AST_if_else_statement_Node>(statement);
    analyze_exp(if_statement->get_exps(), symbol_table, indx);
    analyze_statement(if_statement->get_stmt1(), symbol_table, indx);
  } break;
  case ast::statementType::IFELSE: {
    auto if_else_statement =
        std::static_pointer_cast<ast::AST_if_else_statement_Node>(statement);
    analyze_exp(if_else_statement->get_exps(), symbol_table, indx);
    analyze_statement(if_else_statement->get_stmt1(), symbol_table, indx);
    analyze_statement(if_else_statement->get_stmt2(), symbol_table, indx);
  } break;
  case ast::statementType::WHILE:
  case ast::statementType::DO_WHILE: {
    auto while_statement =
        std::static_pointer_cast<ast::AST_while_statement_Node>(statement);
    analyze_exp(while_statement->get_exps(), symbol_table, indx);
    analyze_statement(while_statement->get_stmt(), symbol_table, indx);
  } break;
  case ast::statementType::FOR:
    analyze_for_statement(
        std::static_pointer_cast<ast::AST_For_Statement_Node>(statement),
        symbol_table, indx);
    break;
  case ast::statementType::BLOCK: {
    auto block_statement =
        std::static_pointer_cast<ast::AST_block_statement_node>(statement);
    std::map<std::pair<std::string, int>, std::string> proxy_symbol_table(
        symbol_table);
    analyze_block(block_statement->get_block(), proxy_symbol_table, indx + 1);
  } break;
  case ast::statementType::GOTO: {
    std::string label = statement->get_labels().first->get_value();
    if (goto_labels.find(label) == goto_labels.end()) {
      goto_labels[label] = false;
    }
  } break;
  case ast::statementType::LABEL: {
    std::string label = statement->get_labels().first->get_value();
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
  } break;
  case ast::statementType::EXP:
    analyze_exp(statement->get_exps(), symbol_table, indx);
    break;
  case ast::statementType::NULLSTMT:
  case ast::statementType::CONTINUE:
  case ast::statementType::BREAK:
    break;
  case ast::statementType::UNKNOWN:
    UNREACHABLE()
  }
}

void parser::analyze_for_statement(
    std::shared_ptr<ast::AST_For_Statement_Node> for_statement,
    std::map<std::pair<std::string, int>, std::string> &symbol_table,
    int indx) {
  // When init is null or init is a simple expression, then
  // we don't need to add another level to the symbol table
  if (for_statement->get_for_init() == nullptr) {
    analyze_exp(for_statement->get_exps(), symbol_table, indx);
    analyze_exp(for_statement->get_exp2(), symbol_table, indx);
    if (for_statement->get_stmt()->get_type() == ast::statementType::FOR) {
      auto forstmt = std::static_pointer_cast<ast::AST_For_Statement_Node>(
          for_statement->get_stmt());
      analyze_for_statement(forstmt, symbol_table, indx);
    } else {
      analyze_statement(for_statement->get_stmt(), symbol_table, indx);
    }
  } else if (for_statement->get_for_init()->get_declaration() == nullptr) {
    analyze_exp(for_statement->get_for_init()->get_exp(), symbol_table, indx);
    analyze_exp(for_statement->get_exps(), symbol_table, indx);
    analyze_exp(for_statement->get_exp2(), symbol_table, indx);
    if (for_statement->get_stmt()->get_type() == ast::statementType::FOR) {
      auto forstmt = std::static_pointer_cast<ast::AST_For_Statement_Node>(
          for_statement->get_stmt());
      analyze_for_statement(forstmt, symbol_table, indx);
    } else {
      analyze_statement(for_statement->get_stmt(), symbol_table, indx);
    }
  } else {
    // Add another level to the symbol table
    std::map<std::pair<std::string, int>, std::string> proxy_symbol_table(
        symbol_table);
    analyze_declaration(for_statement->get_for_init()->get_declaration(),
                        proxy_symbol_table, indx + 1);
    analyze_exp(for_statement->get_exps(), proxy_symbol_table, indx + 1);
    analyze_exp(for_statement->get_exp2(), proxy_symbol_table, indx + 1);
    if (for_statement->get_stmt()->get_type() == ast::statementType::FOR) {
      auto forstmt = std::static_pointer_cast<ast::AST_For_Statement_Node>(
          for_statement->get_stmt());
      analyze_for_statement(forstmt, proxy_symbol_table, indx + 1);
    } else {
      analyze_statement(for_statement->get_stmt(), proxy_symbol_table,
                        indx + 1);
    }
  }
}

// NOTE: symbol table here is a map from {variable_name, block_indx} ->
// temporary_variable_name(used as scar registers later)
void parser::analyze_block(
    std::shared_ptr<ast::AST_Block_Node> block,
    std::map<std::pair<std::string, int>, std::string> &symbol_table,
    int indx) {
  if (block == nullptr)
    return;
  auto block_item = block->get_blockItems().begin();
  auto block_end = block->get_blockItems().end();
  while (block_item != block_end) {
    if (((*block_item)->get_type()) == ast::BlockItemType::DECLARATION) {
      analyze_declaration((*block_item)->get_declaration(), symbol_table, indx);
    } else if ((*block_item)->get_type() == ast::BlockItemType::STATEMENT) {
      analyze_statement((*block_item)->get_statement(), symbol_table, indx);
    }
    block_item++;
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
  case ast::statementType::NULLSTMT:
    return "NullStmt";
  case ast::statementType::RETURN:
    return "Return";
  case ast::statementType::EXP:
    return "Exp";
  case ast::statementType::IF:
    return "If";
  case ast::statementType::IFELSE:
    return "IfElse";
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
  return "";
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

void parser::pretty_print_declaration(
    std::shared_ptr<ast::AST_Declaration_Node> declaration) {
  if (declaration == nullptr)
    return;
  std::cout << "\t\t\tDeclaration=(" << std::endl;
  std::cout << "\t\t\t\tidentifier=\""
            << declaration->get_identifier()->get_value() << "\"," << std::endl;
  if (declaration->get_exp() != nullptr) {
    std::cout << "\t\t\t\texp=(" << std::endl;
    pretty_print_exp(declaration->get_exp());
    std::cout << "\t\t\t\t)," << std::endl;
  }
  std::cout << "\t\t\t)" << std::endl;
}

void parser::pretty_print_statement(
    std::shared_ptr<ast::AST_Statement_Node> statement) {
  if (statement == nullptr)
    return;
  std::cout << "\t\t\tStatement=(" << std::endl;
  std::cout << "\t\t\t\ttype=" << to_string(statement->get_type()) << ","
            << std::endl;
  switch (statement->get_type()) {
  case ast::statementType::RETURN:
    pretty_print_exp(statement->get_exps());
    break;
  case ast::statementType::IF: {
    auto if_statement =
        std::static_pointer_cast<ast::AST_if_else_statement_Node>(statement);
    std::cout << "\t\t\t\tlabels=("
              << if_statement->get_labels().first->get_value() << ","
              << if_statement->get_labels().second->get_value() << "),"
              << std::endl;
    std::cout << "\t\t\t\texps=(" << std::endl;
    pretty_print_exp(if_statement->get_exps());
    std::cout << "\t\t\t\t)," << std::endl;
    std::cout << "\t\t\t\tstmt1=(" << std::endl;
    pretty_print_statement(if_statement->get_stmt1());
    std::cout << "\t\t\t\t)" << std::endl;
  } break;
  case ast::statementType::IFELSE: {
    auto if_else_statement =
        std::static_pointer_cast<ast::AST_if_else_statement_Node>(statement);
    std::cout << "\t\t\t\tlabels=("
              << if_else_statement->get_labels().first->get_value() << ","
              << if_else_statement->get_labels().second->get_value() << "),"
              << std::endl;
    std::cout << "\t\t\t\texps=(" << std::endl;
    pretty_print_exp(if_else_statement->get_exps());
    std::cout << "\t\t\t\t)," << std::endl;
    std::cout << "\t\t\t\tstmt1=(" << std::endl;
    pretty_print_statement(if_else_statement->get_stmt1());
    std::cout << "\t\t\t\t)," << std::endl;
    std::cout << "\t\t\t\tstmt2=(" << std::endl;
    pretty_print_statement(if_else_statement->get_stmt2());
    std::cout << "\t\t\t\t)" << std::endl;
  } break;
  case ast::statementType::WHILE:
  case ast::statementType::DO_WHILE: {
    auto while_statement =
        std::static_pointer_cast<ast::AST_while_statement_Node>(statement);
    std::cout << "\t\t\t\tlabels=("
              << while_statement->get_labels().first->get_value() << ","
              << while_statement->get_labels().second->get_value() << "),"
              << std::endl;
    std::cout << "\t\t\t\texps=(" << std::endl;
    pretty_print_exp(while_statement->get_exps());
    std::cout << "\t\t\t\t)," << std::endl;
    std::cout << "\t\t\t\tstmt=(" << std::endl;
    pretty_print_statement(while_statement->get_stmt());
    std::cout << "\t\t\t\t)" << std::endl;
  } break;
  case ast::statementType::FOR: {
    auto for_statement =
        std::static_pointer_cast<ast::AST_For_Statement_Node>(statement);
    std::cout << "\t\t\t\tlabels=("
              << for_statement->get_labels().first->get_value() << ","
              << for_statement->get_labels().second->get_value() << "),"
              << std::endl;
    if (for_statement->get_for_init() != nullptr) {
      std::cout << "\t\t\t\tfor_init=(" << std::endl;
      if (for_statement->get_for_init()->get_declaration() != nullptr) {
        pretty_print_declaration(
            for_statement->get_for_init()->get_declaration());
      } else {
        pretty_print_exp(for_statement->get_for_init()->get_exp());
      }
      std::cout << "\t\t\t\t)," << std::endl;
    }
    std::cout << "\t\t\t\tcondition=(" << std::endl;
    pretty_print_exp(for_statement->get_exps());
    std::cout << "\t\t\t\t)," << std::endl;
    std::cout << "\t\t\t\tpost=(" << std::endl;
    pretty_print_exp(for_statement->get_exp2());
    std::cout << "\t\t\t\t)," << std::endl;
    std::cout << "\t\t\t\tstmt=(" << std::endl;
    pretty_print_statement(for_statement->get_stmt());
    std::cout << "\t\t\t\t)" << std::endl;
  } break;
  case ast::statementType::BLOCK: {
    auto block_statement =
        std::static_pointer_cast<ast::AST_block_statement_node>(statement);
    pretty_print_block(block_statement->get_block());
  } break;
  case ast::statementType::GOTO:
  case ast::statementType::LABEL:
  case ast::statementType::CONTINUE:
  case ast::statementType::BREAK:
    std::cout << "\t\t\t\tlabel=(" << statement->get_labels().first->get_value()
              << ")" << std::endl;
    break;
  case ast::statementType::EXP:
    pretty_print_exp(statement->get_exps());
    break;
  case ast::statementType::NULLSTMT:
    break;
  case ast::statementType::UNKNOWN:
    UNREACHABLE()
  }
}

void parser::pretty_print_block(std::shared_ptr<ast::AST_Block_Node> block) {
  if (block == nullptr)
    return;
  std::cout << "\t\t\t" << "Block=(" << std::endl;
  for (auto blockItem : block->get_blockItems()) {
    if (blockItem->get_type() == ast::BlockItemType::DECLARATION) {
      pretty_print_declaration(blockItem->get_declaration());
    } else {
      pretty_print_statement(blockItem->get_statement());
    }
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
