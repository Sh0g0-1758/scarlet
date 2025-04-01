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
      auto vars =
          std::static_pointer_cast<ast::AST_variable_declaration_Node>(decls);
      analyze_global_variable_declaration(vars, symbol_table);
    } else if (decls->get_type() == ast::DeclarationType::FUNCTION) {
      auto funcs =
          std::static_pointer_cast<ast::AST_function_declaration_Node>(decls);
      currFuncName = funcs->get_declarator()->get_identifier()->get_value();
      analyze_global_function_declaration(funcs, symbol_table);

      // Each function has its own set of goto labels
      // Two functions can have labels with the same name and goto label
      // for each function will point to the label in that function
      analyze_goto_labels();
      goto_labels.clear();
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
