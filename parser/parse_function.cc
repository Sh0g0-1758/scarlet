#include "common.hh"

namespace scarlet {
namespace parser {

void parser::parse_function(std::vector<token::Token> &tokens,
                            std::shared_ptr<ast::AST_Function_Node> function) {
  MAKE_SHARED(ast::AST_function_declaration_Node, declaration);
  declaration->set_type(ast::DeclarationType::FUNCTION);
  parse_function_declaration(tokens, declaration);
  function->set_declaration(std::move(declaration));
  function_name = function->get_declaration()->get_identifier()->get_value();
  if (tokens[0].get_token() == token::TOKEN::SEMICOLON) {
    tokens.erase(tokens.begin());
    return;
  } else {
    MAKE_SHARED(ast::AST_Block_Node, block);
    parse_block(tokens, block);
    function->set_block(block);
  }
}

} // namespace parser
} // namespace scarlet
