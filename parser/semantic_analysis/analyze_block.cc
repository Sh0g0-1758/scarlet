#include <parser/common.hh>

namespace scarlet {
namespace parser {

// NOTE: symbol table here is a map from {variable_name, block_indx} ->
// temporary_variable_name(used as scar registers later)
void parser::analyze_block(
    std::shared_ptr<ast::AST_Block_Node> block,
    std::map<std::pair<std::string, int>, symbolInfo> &symbol_table, int indx) {
  if (block == nullptr)
    return;
  auto block_item = block->get_blockItems().begin();
  auto block_end = block->get_blockItems().end();
  while (block_item != block_end) {
    if (((*block_item)->get_type()) == ast::BlockItemType::DECLARATION) {
      analyze_declaration((*block_item)->get_declaration(), symbol_table, indx);
    } else if ((*block_item)->get_type() == ast::BlockItemType::STATEMENT) {
      analyze_statement((*block_item)->get_statement(), symbol_table, indx);
    }
    block_item++;
  }
}

} // namespace parser
} // namespace scarlet
