#include <codegen/codegen.hh>

namespace scarlet {
namespace codegen {

void Codegen::gen_scar_block(
    std::shared_ptr<ast::AST_Block_Node> block,
    std::shared_ptr<scar::scar_Function_Node> scar_function) {
  for (auto inst : block->get_blockItems()) {
    switch (inst->get_type()) {
    case ast::BlockItemType::STATEMENT:
      gen_scar_statement(inst->get_statement(), scar_function);
      break;
    case ast::BlockItemType::DECLARATION:
      gen_scar_declaration(inst->get_declaration(), scar_function);
      break;
    case ast::BlockItemType::UNKNOWN:
      UNREACHABLE()
    }
  }
}

} // namespace codegen
} // namespace scarlet
