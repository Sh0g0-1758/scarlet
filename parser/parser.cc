#include "common.hh"
namespace scarlet {
namespace parser {

void parser::parse_program(std::vector<token::Token> tokens) {
  ast::AST_Program_Node program;
  while (!tokens.empty() and success) {
    MAKE_SHARED(ast::AST_Function_Node, function);
    parse_function(tokens, function);
    program.add_function(std::move(function));
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
  std::map<std::pair<std::string, int>, symbolInfo> global_symbol_table;
  for (auto funcs : program.get_functions()) {
    std::string func_name =
        funcs->get_declaration()->get_identifier()->get_value();
    if (global_symbol_table.find({func_name, 0}) != global_symbol_table.end()) {
      success = false;
      error_messages.emplace_back("Function " + func_name +
                                  " has already been defined");
    } else {
      symbolInfo si;
      si.name = func_name;
      si.link = linkage::EXTERNAL;
      si.type = symbolType::FUNCTION;

      std::map<std::pair<std::string, int>, symbolInfo> symbol_table(
          global_symbol_table);
      analyze_block(funcs->get_block(), symbol_table, 0);
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
  for (auto function : program.get_functions()) {
    std::cout << "\tFunction(" << std::endl;
    std::cout << "\t\tname=\""
              << function->get_declaration()->get_identifier()->get_value()
              << "\"," << std::endl;
    std::cout << "\t\tparams=[" << std::endl;
    for (auto param : function->get_declaration()->get_params()) {
      std::cout << "\t\t\tParam(" << std::endl;
      std::cout << "\t\t\t\ttype=\"" << type_to_string(param->type) << "\","
                << std::endl;
      std::cout << "\t\t\t\tidentifier=\"" << param->identifier->get_value()
                << "\"" << std::endl;
      std::cout << "\t\t\t)," << std::endl;
    }
    std::cout << "\t\t]," << std::endl;
    std::cout << "\t\tbody=[" << std::endl;
    pretty_print_block(function->get_block());
    std::cout << "\t\t]" << std::endl;
    std::cout << "\t)," << std::endl;
  }
  std::cout << ")" << std::endl;
}

} // namespace parser
} // namespace scarlet
