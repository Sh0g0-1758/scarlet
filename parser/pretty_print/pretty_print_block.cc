#include <parser/common.hh>

namespace scarlet {
namespace parser {

void parser::pretty_print_block(std::shared_ptr<ast::AST_Block_Node> block) {
  if (block == nullptr)
    return;
  std::cout << "\t\t\t" << "Block=(" << std::endl;
  for (auto blockItem : block->get_blockItems()) {
    if (blockItem->get_type() == ast::BlockItemType::DECLARATION) {
      pretty_print_declaration(blockItem->get_declaration());
    } else {
      pretty_print_statement(blockItem->get_statement());
    }
  }
  std::cout << "\t\t\t)" << std::endl;
}

} // namespace parser
} // namespace scarlet
