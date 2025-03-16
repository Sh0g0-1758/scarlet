#include <parser/common.hh>

namespace scarlet {
namespace parser {

std::string to_string(ast::statementType type) {
  switch (type) {
  case ast::statementType::NULLSTMT:
    return "NullStmt";
  case ast::statementType::SWITCH:
    return "Switch";
  case ast::statementType::CASE:
    return "Case";
  case ast::statementType::DEFAULT_CASE:
    return "Default";
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
  case ast::statementType::CASE:
    std::cout << "\t\t\t\tlabel=(" << statement->get_labels().first->get_value()
              << ")" << std::endl;
    break;
  case ast::statementType::DEFAULT_CASE:
    std::cout << "\t\t\t\tlabel=(" << statement->get_labels().first->get_value()
              << ")" << std::endl;
    // pretty_print_exp(statement->get_exps());
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
  case ast::statementType::SWITCH: {
    auto switch_statement =
        std::static_pointer_cast<ast::AST_switch_statement_Node>(statement);
  } break;
  case ast::statementType::WHILE:
  case ast::statementType::DO_WHILE: {
    auto while_statement =
        std::static_pointer_cast<ast::AST_while_statement_Node>(statement);
    std::cout << "\t\t\t\tlabels=(";
    if (while_statement->get_labels().first)
      std::cout << while_statement->get_labels().first->get_value() << ",";
    std::cout << while_statement->get_labels().second->get_value() << "),"
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

} // namespace parser
} // namespace scarlet
