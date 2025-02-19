#include "common.hh"

namespace scarlet {

namespace parser {

std::shared_ptr<ast::AST_Function_Node>
parser::parse_function(std::vector<token::Token> &tokens) {
  MAKE_SHARED(ast::AST_Function_Node, function);
  EXPECT_FUNC(token::TOKEN::INT);
  parse_identifier(tokens, function);
  EXPECT_FUNC(token::TOKEN::OPEN_PARANTHESES);
  EXPECT_FUNC(token::TOKEN::VOID);
  EXPECT_FUNC(token::TOKEN::CLOSE_PARANTHESES);
  MAKE_SHARED(ast::AST_Block_Node, block);
  parse_block(tokens, block);
  function->set_block(block);
  return function;
}

} // namespace parser

} // namespace scarlet
