#include <codegen/common.hh>

namespace scarlet {
namespace codegen {

#define REMOVE_BLOCK()                                                         \
  auto prevID = (block - 1)->get_id();                                         \
  auto currID = block->get_id();                                               \
  auto nextID = (block + 1)->get_id();                                         \
  for (auto succID : block->get_succ()) {                                      \
    cfg[succID].remove_pred(currID);                                           \
    cfg[succID].add_pred(prevID);                                              \
  }                                                                            \
  for (auto predID : block->get_pred()) {                                      \
    cfg[predID].remove_succ(currID);                                           \
    cfg[predID].add_succ(nextID);                                              \
  }                                                                            \
  block = cfg.erase(block);                                                    \
  --block;

bool Codegen::unreachable_code_elimination(std::vector<cfg::node> &cfg) {
  bool isChanged{};
  // eliminate unreachable blocks
  std::map<unsigned int, bool> visited;
  std::map<unsigned int, bool> reachable;

  std::queue<unsigned int> q;
  q.push(0);

  while (!q.empty()) {
    unsigned int blockId = q.front();
    q.pop();
    if (visited[blockId])
      continue;
    visited[blockId] = true;
    reachable[blockId] = true;

    for (int succ : cfg[blockId].get_succ()) {
      if (!visited[succ]) {
        q.push(succ);
      }
    }
  }

  for (auto block = cfg.begin(); block != cfg.end(); ++block) {
    unsigned int blockId = block->get_id();
    if (reachable[blockId])
      continue;
    isChanged = true;
    REMOVE_BLOCK();
  }

  // eliminate useless jumps
  for (auto block = cfg.begin(); block != cfg.end(); ++block) {
    if (block->is_empty())
      continue;
    auto instr = block->get_body().back();
    if (instr->get_type() == scar::instruction_type::JUMP or
        instr->get_type() == scar::instruction_type::JUMP_IF_ZERO or
        instr->get_type() == scar::instruction_type::JUMP_IF_NOT_ZERO) {
      bool keepJump = false;
      for (auto succID : block->get_succ()) {
        if (succID != (block + 1)->get_id())
          keepJump = true;
      }
      if (!keepJump) {
        isChanged = true;
        block->get_body().pop_back();
        if (block->get_body().empty()) {
          REMOVE_BLOCK();
        }
      }
    }
  }

  // eliminate useless labels
  for (auto block = cfg.begin(); block != cfg.end(); ++block) {
    if (block->is_empty())
      continue;
    auto instr = block->get_body().front();
    if (instr->get_type() == scar::instruction_type::LABEL) {
      bool keepLabel = false;
      for (auto predID : block->get_pred()) {
        if (predID != (block - 1)->get_id())
          keepLabel = true;
      }
      if (!keepLabel) {
        isChanged = true;
        block->get_body().erase(block->get_body().begin());
        if (block->get_body().empty()) {
          REMOVE_BLOCK();
        }
      }
    }
  }

  return isChanged;
}

} // namespace codegen
} // namespace scarlet
