#include "common.hh"
namespace scarlet {
namespace parser {

void parser::parse_program(std::vector<token::Token> tokens) {
  ast::AST_Program_Node program;
  while (!tokens.empty() and success) {
    MAKE_SHARED(ast::AST_Declaration_Node, declaration);
    parse_declaration(tokens, declaration, true);
    program.add_declaration(std::move(declaration));
  }
  this->program = program;
}

void parser::expect(token::TOKEN actual_token, token::TOKEN expected_token) {
  if (actual_token != expected_token) {
    success = false;
    error_messages.emplace_back("Expected token " +
                                token::to_string(expected_token) + " but got " +
                                token::to_string(actual_token));
  }
}

#define EXPISCONSTANT(exp)                                                     \
  (exp->get_binop_node() == nullptr and exp->get_left() == nullptr and         \
   exp->get_right() == nullptr and exp->get_factor_node() != nullptr and       \
   exp->get_factor_node()->get_int_node() != nullptr and                       \
   exp->get_factor_node()->get_unop_nodes().empty() and                        \
   exp->get_factor_node()->get_identifier_node() == nullptr and                \
   exp->get_factor_node()->get_exp_node() == nullptr)

void parser::semantic_analysis() {
  std::map<std::pair<std::string, int>, symbolTable::symbolInfo> symbol_table;
  for (auto decls : program.get_declarations()) {
    if (decls->get_type() == ast::DeclarationType::VARIABLE) {
      auto vars =
          std::static_pointer_cast<ast::AST_variable_declaration_Node>(decls);
      std::string var_name = vars->get_identifier()->get_value();

      // Check if the symbol has been declared before
      if (globalSymbolTable.find(var_name) != globalSymbolTable.end()) {
        // Ensure that the previous declaration has the same type
        if (globalSymbolTable[var_name].type !=
                symbolTable::symbolType::VARIABLE or
            globalSymbolTable[var_name].typeDef[0] != vars->get_type()) {
          success = false;
          error_messages.emplace_back(
              var_name + " redeclared as a different kind of symbol");
        }

        // If the symbol has been redefined, throw an error
        if (globalSymbolTable[var_name].def == symbolTable::defType::TRUE and
            vars->get_exp() != nullptr) {
          success = false;
          error_messages.emplace_back("Variable " + var_name +
                                      " has already been defined");
        }

        if (vars->get_specifier() == ast::SpecifierType::EXTERN) {
          // The linkage will be the same as the linkage of the previous
          // declaration.
        } else if (vars->get_specifier() == ast::SpecifierType::STATIC) {
          // If the previous declaration was external, throw an error
          if (globalSymbolTable[var_name].link ==
              symbolTable::linkage::EXTERNAL) {
            success = false;
            error_messages.emplace_back(
                "Variable " + var_name +
                " declared with static storage specifier after being declared "
                "with external linkage");
          }
        } else {
          // If the previous declaration was internal, throw an error
          if (globalSymbolTable[var_name].link ==
              symbolTable::linkage::INTERNAL) {
            success = false;
            error_messages.emplace_back("Variable " + var_name +
                                        " declared with external linkage after "
                                        "being declared with internal linkage");
          }
        }

        // If the previous declaration was because of a block scope extern
        // then it is possible that the symbol definition is not present in the
        // symbol table at index 0
        if (symbol_table.find({var_name, 0}) == symbol_table.end()) {
          symbol_table[{var_name, 0}] = globalSymbolTable[var_name];
        }

        // If the variable has been defined, check that it is initialized with a
        // constant integer
        if (vars->get_exp() != nullptr) {
          symbol_table[{var_name, 0}].def = symbolTable::defType::TRUE;
          globalSymbolTable[var_name].def = symbolTable::defType::TRUE;
          if (!EXPISCONSTANT(vars->get_exp())) {
            success = false;
            error_messages.emplace_back(
                "Variable " + var_name +
                " is not initialized with a constant integer");
          } else {
            globalSymbolTable[var_name].value =
                std::stoi(vars->get_exp()
                              ->get_factor_node()
                              ->get_int_node()
                              ->get_value());
          }
        } else if (vars->get_specifier() != ast::SpecifierType::EXTERN) {
          // If the variable has not been defined and is not extern,
          // mark it as a tentative definition
          symbol_table[{var_name, 0}].def = symbolTable::defType::TENTATIVE;
          globalSymbolTable[var_name].def = symbolTable::defType::TENTATIVE;
        }
      } else { // symbol has not been declared before
        // Give appropriate linkage to the variable
        symbol_table[{var_name, 0}] = {var_name,
                                       symbolTable::linkage::EXTERNAL,
                                       symbolTable::symbolType::VARIABLE,
                                       {vars->get_type()},
                                       symbolTable::defType::TENTATIVE};
        if (vars->get_specifier() == ast::SpecifierType::STATIC) {
          symbol_table[{var_name, 0}].link = symbolTable::linkage::INTERNAL;
        } else if (vars->get_specifier() == ast::SpecifierType::EXTERN) {
          symbol_table[{var_name, 0}].def = symbolTable::defType::FALSE;
        }

        // Make sure that global variables are initialized only with
        // constant integers
        if (vars->get_exp() != nullptr) {
          symbol_table[{var_name, 0}].def = symbolTable::defType::TRUE;
          if (!EXPISCONSTANT(vars->get_exp())) {
            success = false;
            error_messages.emplace_back(
                "Global variable " + var_name +
                " is not initialized with a constant integer");
          } else {
            symbol_table[{var_name, 0}].value =
                std::stoi(vars->get_exp()
                              ->get_factor_node()
                              ->get_int_node()
                              ->get_value());
          }
        }
        globalSymbolTable[var_name] = symbol_table[{var_name, 0}];
      }
    } else if (decls->get_type() == ast::DeclarationType::FUNCTION) {
      auto funcs =
          std::static_pointer_cast<ast::AST_function_declaration_Node>(decls);
      if (funcs->get_block() == nullptr) {
        analyze_declaration(funcs, symbol_table, 0);
      } else {
        std::string func_name = funcs->get_identifier()->get_value();
        if (globalSymbolTable.find(func_name) != globalSymbolTable.end() and
            globalSymbolTable[func_name].def == symbolTable::defType::TRUE) {
          success = false;
          error_messages.emplace_back("Function " + func_name +
                                      " has already been defined");
        }
        analyze_declaration(funcs, symbol_table, 0);
        symbol_table[{func_name, 0}].def = symbolTable::defType::TRUE;
        globalSymbolTable[func_name].def = symbolTable::defType::TRUE;
        std::map<std::pair<std::string, int>, symbolTable::symbolInfo>
            proxy_symbol_table(symbol_table);
        for (auto param : funcs->get_params()) {
          std::string temp_name = get_temp_name(param->identifier->get_value());
          proxy_symbol_table[{param->identifier->get_value(), 1}] = {
              temp_name,
              symbolTable::linkage::NONE,
              symbolTable::symbolType::VARIABLE,
              {param->type}};
          param->identifier->set_identifier(temp_name);
        }
        goto_labels.clear();
        analyze_block(funcs->get_block(), proxy_symbol_table, 1);
      }
    }
  }
  // Check that all labels are declared
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
  error_messages.emplace_back("Expected " +
                              token::to_string(token.get_token()) +
                              " but got end of file");
}

void parser::pretty_print() {
  std::cout << "Program(" << std::endl;
  for (auto declaration : program.get_declarations()) {
    pretty_print_declaration(declaration);
  }
  std::cout << ")" << std::endl;
}

} // namespace parser
} // namespace scarlet
