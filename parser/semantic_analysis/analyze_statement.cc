#include <parser/common.hh>

namespace scarlet {
namespace parser {

void parser::analyze_statement(
    std::shared_ptr<ast::AST_Statement_Node> statement,
    std::map<std::pair<std::string, int>, symbolTable::symbolInfo>
        &symbol_table,
    int indx) {

  // every variable that is used in any of the statement should already
  // have been declared.

  // We need to check that the labels used in goto statements are
  // actually declared and we also need to make sure that there are no
  // duplicate labels

  switch (statement->get_type()) {
  case ast::statementType::RETURN: {
    analyze_exp(statement->get_exps(), symbol_table, indx);
    decay_arr_to_pointer(nullptr, statement->get_exps());
    auto funcType = globalSymbolTable[currFuncName].typeDef[0];
    auto funcDerivedType = globalSymbolTable[currFuncName].derivedTypeMap[0];
    auto expType = statement->get_exps()->get_type();
    auto expDerivedType = statement->get_exps()->get_derived_type();
    auto [castType, castDerivedType] =
        ast::getAssignType(funcType, funcDerivedType, expType, expDerivedType,
                           statement->get_exps());
    if (castType == ast::ElemType::NONE) {
      success = false;
      error_messages.emplace_back("Invalid return value of function " +
                                  currFuncName);
    } else {
      if (castType != expType or castDerivedType != expDerivedType) {
        add_cast_to_exp(statement->get_exps(), funcType, funcDerivedType);
      }
    }
  } break;
  case ast::statementType::CASE:
  case ast::statementType::DEFAULT_CASE:
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
  case ast::statementType::SWITCH: {
    // iterate over case_exp_label and analyze the case expression
    analyze_exp(statement->get_exps(), symbol_table, indx);
    ast::ElemType switchType = statement->get_exps()->get_type();
    auto switch_statement =
        std::static_pointer_cast<ast::AST_switch_statement_Node>(statement);
    switch (switchType) {
    case ast::ElemType::INT:
      analyze_switch_case<int>(switch_statement, constant::Type::INT);
      break;
    case ast::ElemType::LONG:
      analyze_switch_case<long>(switch_statement, constant::Type::LONG);
      break;
    case ast::ElemType::UINT:
      analyze_switch_case<unsigned int>(switch_statement, constant::Type::UINT);
      break;
    case ast::ElemType::ULONG:
      analyze_switch_case<unsigned long>(switch_statement,
                                         constant::Type::ULONG);
      break;
    case ast::ElemType::DOUBLE: {
      success = false;
      error_messages.emplace_back(
          "Switch expression cannot be a double precision");
    } break;
    case ast::ElemType::DERIVED: {
      success = false;
      error_messages.emplace_back("Switch expression cannot be a derived type");
    }
    case ast::ElemType::POINTER:
    case ast::ElemType::VOID:
    /*fixme? okay for void */
    case ast::ElemType::NONE:
      break;
    }
    // analyze the statement inside the switch
    analyze_statement(switch_statement->get_stmt(), symbol_table, indx);

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
    std::map<std::pair<std::string, int>, symbolTable::symbolInfo>
        proxy_symbol_table(symbol_table);
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
    std::map<std::pair<std::string, int>, symbolTable::symbolInfo>
        &symbol_table,
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
    std::map<std::pair<std::string, int>, symbolTable::symbolInfo>
        proxy_symbol_table(symbol_table);
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

template <typename T>
void parser::analyze_switch_case(
    std::shared_ptr<ast::AST_switch_statement_Node> switch_statement,
    constant::Type switchType) {
  std::map<T, bool> case_val;
  for (auto case_exp_label : switch_statement->get_case_exp_label()) {
    // default case has no expression
    if (case_exp_label.first == nullptr) {
      continue;
    }
    // switch expression should be a constant value
    if (!EXPISCONSTANT(case_exp_label.first)) {
      success = false;
      error_messages.emplace_back(
          "Case expression is not a constant integer expression");
      continue;
    } else {
      auto constVal = case_exp_label.first->get_factor_node()
                          ->get_const_node()
                          ->get_constant();
      T val;
      switch (constVal.get_type()) {
      case constant::Type::INT:
        val = static_cast<T>(constVal.get_value().i);
        break;
      case constant::Type::LONG:
        val = static_cast<T>(constVal.get_value().l);
        break;
      case constant::Type::DOUBLE: {
        success = false;
        error_messages.emplace_back(
            "Case expression cannot be a double precision");
      } break;
      case constant::Type::UINT:
        val = static_cast<T>(constVal.get_value().ui);
        break;
      case constant::Type::ULONG:
        val = static_cast<T>(constVal.get_value().ul);
        break;
      default:
        break;
      }
      constant::Constant newConst;
      newConst.set_type(switchType);
      switch (switchType) {
      case constant::Type::INT:
        newConst.set_value({.i = static_cast<int>(val)});
        break;
      case constant::Type::LONG:
        newConst.set_value({.l = static_cast<long>(val)});
        break;
      case constant::Type::DOUBLE: {
        success = false;
        error_messages.emplace_back(
            "Case expression cannot be a double precision");
      } break;
      case constant::Type::UINT:
        newConst.set_value({.ui = static_cast<unsigned int>(val)});
        break;
      case constant::Type::ULONG:
        newConst.set_value({.ul = static_cast<unsigned long>(val)});
        break;
      default:
        break;
      }

      case_exp_label.first->get_factor_node()->get_const_node()->set_constant(
          newConst);

      if (case_val.find(val) != case_val.end()) {
        success = false;
        error_messages.emplace_back("Duplicate case value in switch statement");
        continue;
      }
      case_val[val] = true;
    }
  }
}

} // namespace parser
} // namespace scarlet
