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

void parser::semantic_analysis() {
  std::map<std::pair<std::string, int>, symbolTable::symbolInfo> symbol_table;
  for (auto decls : program.get_declarations()) {
    if (decls->get_type() == ast::DeclarationType::VARIABLE) {
      analyze_declaration(decls, symbol_table, 0);
    } else if (decls->get_type() == ast::DeclarationType::FUNCTION) {
      auto funcs =
          std::static_pointer_cast<ast::AST_function_declaration_Node>(decls);
      if (funcs->get_block() == nullptr) {
        analyze_declaration(funcs, symbol_table, 0);
      } else {
        std::string func_name = funcs->get_identifier()->get_value();
        if (globalSymbolTable.find(func_name) != globalSymbolTable.end() and
            globalSymbolTable[func_name].isDefined) {
          success = false;
          error_messages.emplace_back("Function " + func_name +
                                      " has already been defined");
        }
        analyze_declaration(funcs, symbol_table, 0);
        symbol_table[{func_name, 0}].isDefined = true;
        globalSymbolTable[func_name].isDefined = true;
        std::map<std::pair<std::string, int>, symbolTable::symbolInfo>
            proxy_symbol_table(symbol_table);
        for (auto param : funcs->get_params()) {
          std::string temp_name = get_temp_name(param->identifier->get_value());
          proxy_symbol_table[{param->identifier->get_value(), 1}] = {
              temp_name,
              symbolTable::linkage::INTERNAL,
              symbolTable::symbolType::VARIABLE,
              {param->type}};
          param->identifier->set_identifier(temp_name);
        }
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
