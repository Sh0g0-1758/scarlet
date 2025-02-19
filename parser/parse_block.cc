#include "common.hh"

namespace scarlet {

namespace parser {

void parser::parse_block(std::vector<token::Token> &tokens,
                         std::shared_ptr<ast::AST_Block_Node> block) {
  EXPECT(token::TOKEN::OPEN_BRACE);
  while (!tokens.empty() and success and
         tokens[0].get_token() != token::TOKEN::CLOSE_BRACE) {
    parse_block_item(tokens, block);
  }
  EXPECT(token::TOKEN::CLOSE_BRACE);
}

void parser::parse_block_item(std::vector<token::Token> &tokens,
                              std::shared_ptr<ast::AST_Block_Node> &block) {
  MAKE_SHARED(ast::AST_Block_Item_Node, block_item);
  if (tokens[0].get_token() == token::TOKEN::INT) {
    block_item->set_type(ast::BlockItemType::DECLARATION);
    MAKE_SHARED(ast::AST_Declaration_Node, declaration);
    parse_declaration(tokens, declaration);
    block_item->set_declaration(std::move(declaration));
  } else {
    block_item->set_type(ast::BlockItemType::STATEMENT);
    MAKE_SHARED(ast::AST_Statement_Node, statement);
    parse_statement(tokens, statement);
    block_item->set_statement(std::move(statement));
  }
  block->add_blockItem(std::move(block_item));
}

} // namespace parser

} // namespace scarlet
