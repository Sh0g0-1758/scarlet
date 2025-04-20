#include <codegen/common.hh>

namespace scarlet {
namespace codegen {

void Codegen::transfer_stores(cfg::node &block) {
  for (auto it = block.get_body().end() - 1; it >= block.get_body().begin();
       --it) {
    auto instr = *it;
    auto src = instr->get_src1();
    auto src2 = instr->get_src2();
    auto dst = instr->get_dst();

    if (dst != nullptr)
      block.live_vars.erase(dst->get_reg());
    if (src != nullptr and src->get_type() == scar::val_type::VAR)
      block.live_vars[src->get_reg()] = true;
    if (src2 != nullptr and src2->get_type() == scar::val_type::VAR)
      block.live_vars[src2->get_reg()] = true;

    if (instr->get_type() == scar::instruction_type::CALL) {
      auto funCall =
          std::static_pointer_cast<scar::scar_FunctionCall_Instruction_Node>(
              instr);
      for (auto arg : funCall->get_args()) {
        if (arg->get_type() == scar::val_type::VAR) {
          block.live_vars[arg->get_reg()] = true;
        }
      }
    }
  }
}

std::map<std::string, bool> Codegen::merge_stores(std::vector<cfg::node> &cfg,
                                                  cfg::node &block) {
  if (block.get_succ().empty())
    return {};

  auto initMap = getNodeFromID(cfg, block.get_succ()[0]).live_vars;
  for (int i = 1; i < (int)block.get_succ().size(); i++) {
    for (auto it : getNodeFromID(cfg, block.get_succ()[i]).live_vars) {
      initMap[it.first] = it.second;
    }
  }
  return initMap;
}

bool Codegen::is_dead_store(scar::scar_Instruction_Node &instr,
                            std::map<std::string, bool> &live_vars) {
  if (instr.get_type() == scar::instruction_type::CALL)
    return false;
  auto dst = instr.get_dst();
  if (dst == nullptr)
    return false;
  if (aliased_vars[dst->get_reg()])
    return false;
  if (live_vars[dst->get_reg()])
    return false;
  return true;
}

bool Codegen::dead_store_elimination(std::vector<cfg::node> &cfg) {
  bool ran_dead_store_elimination{};

  std::queue<unsigned int> worklist;
  std::map<unsigned int, bool> worklistMap;
  initialize_worklist(cfg, cfg[0], worklist, worklistMap);

  // liveness analysis
  while (!worklist.empty()) {
    auto &block = getNodeFromID(cfg, worklist.front());
    worklistMap[worklist.front()] = false;
    worklist.pop();
    auto old_live_vars = block.live_vars;
    block.live_vars = merge_stores(cfg, block);
    transfer_stores(block);
    auto new_live_vars = block.live_vars;
    if (old_live_vars != new_live_vars) {
      for (auto predID : block.get_pred()) {
        if (predID == 0)
          continue;
        if (worklistMap[predID])
          continue;
        worklist.push(predID);
        worklistMap[predID] = true;
      }
    }
  }

  // remove dead stores
  for (auto block = cfg.begin(); block != cfg.end(); block++) {
    auto live_vars = merge_stores(cfg, *block);
    for (auto it = block->get_body().end() - 1; it >= block->get_body().begin();
         --it) {
      auto instr = *it;
      if (is_dead_store(*instr, live_vars)) {
        ran_dead_store_elimination = true;
        it = block->get_body().erase(it);
      } else {
        auto src = instr->get_src1();
        auto src2 = instr->get_src2();
        auto dst = instr->get_dst();

        if (dst != nullptr)
          live_vars.erase(dst->get_reg());
        if (src != nullptr and src->get_type() == scar::val_type::VAR)
          live_vars[src->get_reg()] = true;
        if (src2 != nullptr and src2->get_type() == scar::val_type::VAR)
          live_vars[src2->get_reg()] = true;

        if (instr->get_type() == scar::instruction_type::CALL) {
          auto funCall = std::static_pointer_cast<
              scar::scar_FunctionCall_Instruction_Node>(instr);
          for (auto arg : funCall->get_args()) {
            if (arg->get_type() == scar::val_type::VAR) {
              live_vars[arg->get_reg()] = true;
            }
          }
        }
      }
    }
  }

  return ran_dead_store_elimination;
}

// Initialize the worklist in postorder
void Codegen::initialize_worklist(std::vector<cfg::node> &cfg, cfg::node &block,
                                  std::queue<unsigned int> &worklist,
                                  std::map<unsigned int, bool> &worklistMap) {
  for (auto succID : block.get_succ())
    initialize_worklist(cfg, getNodeFromID(cfg, succID), worklist, worklistMap);
  if (block.is_empty())
    return;
  worklist.push(block.get_id());
  worklistMap[block.get_id()] = true;
}

} // namespace codegen
} // namespace scarlet
