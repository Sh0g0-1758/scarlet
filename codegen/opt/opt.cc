#include <codegen/common.hh>

namespace scarlet {
namespace codegen {
void Codegen::optimize(scarcmd &cmd) {
  optInit(cmd);

  for (auto elem : scar.get_elems()) {
    if (elem->get_type() == scar::topLevelType::FUNCTION) {
      auto funcBody = std::static_pointer_cast<scar::scar_Function_Node>(elem)
                          ->get_instructions();
      if (funcBody.empty())
        continue;
      while (true) {
        bool ran_constant_folding;
        if (enable_constant_folding or enable_all) {
          ran_constant_folding = constant_folding(funcBody);
        }
        if (!ran_constant_folding)
          break;
      }
    }
  }
}

void Codegen::optInit(scarcmd &cmd) {
  if (cmd.has_option("fold-constants"))
    enable_constant_folding = true;
  if (cmd.has_option("eliminate-unreachable-code"))
    enable_unreachable_code_elimination = true;
  if (cmd.has_option("propagate-copies"))
    enable_copy_propagation = true;
  if (cmd.has_option("eliminate-dead-stores"))
    enable_dead_store_elimination = true;
  if (cmd.has_option("optimize"))
    enable_all = true;
}

} // namespace codegen
} // namespace scarlet
