#include <parser/common.hh>

namespace scarlet {
namespace parser {

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

} // namespace parser
} // namespace scarlet
