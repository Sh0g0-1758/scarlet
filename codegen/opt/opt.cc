#include <codegen/common.hh>

namespace scarlet {
namespace codegen {
void Codegen::optimize(scarcmd &cmd) {
  optInit(cmd);

  for (auto &elem : scar.get_elems()) {
    if (elem->get_type() == scar::topLevelType::FUNCTION) {
      auto &funcBody = std::static_pointer_cast<scar::scar_Function_Node>(elem)
                           ->get_instructions();
      if (funcBody.empty())
        continue;
      std::vector<cfg::node> cfg;
      gen_cfg_from_funcBody(cfg, funcBody);
      while (true) {
        bool ran_constant_folding{};
        if (enable_constant_folding or enable_all) {
          ran_constant_folding = constant_folding(cfg);
        }
        bool ran_unreachable_code_elimination{};
        if (enable_unreachable_code_elimination or enable_all) {
          ran_unreachable_code_elimination = unreachable_code_elimination(cfg);
        }
        bool ran_copy_propagation{};
        if (enable_copy_propagation or enable_all) {
          ran_copy_propagation = copy_propagation(cfg);
        }
        if (!ran_constant_folding and !ran_unreachable_code_elimination and
            !ran_copy_propagation)
          break;
      }
      gen_funcBody_from_cfg(cfg, funcBody);
    }
  }
}

void Codegen::gen_funcBody_from_cfg(
    std::vector<cfg::node> &cfg,
    std::vector<std::shared_ptr<scar::scar_Instruction_Node>> &funcBody) {
  funcBody.clear();
  for (auto block = cfg.begin(); block != cfg.end(); ++block) {
    if (block->is_empty())
      continue;
    auto body = block->get_body();
    for (auto it : body) {
      funcBody.emplace_back(it);
    }
  }
}

void Codegen::gen_cfg_from_funcBody(
    std::vector<cfg::node> &cfg,
    std::vector<std::shared_ptr<scar::scar_Instruction_Node>> &funcBody) {
  NodeLabelToId.clear();
  cfg::node entryNode = cfg::node();
  entryNode.set_id(0);
  entryNode.add_succ(1);
  cfg.emplace_back(entryNode);

  cfg::node node = cfg::node();
  unsigned int blockId = 1;
  for (auto it : funcBody) {
    auto instType = it->get_type();
    if (instType == scar::instruction_type::LABEL) {
      if (!node.is_empty()) {
        node.set_id(blockId++);
        cfg.emplace_back(node);
        node = cfg::node();
      }
      NodeLabelToId[it->get_src1()->get_label()] = blockId;
      node.add_instruction(it);
    } else if (instType == scar::instruction_type::RETURN or
               instType == scar::instruction_type::JUMP or
               instType == scar::instruction_type::JUMP_IF_ZERO or
               instType == scar::instruction_type::JUMP_IF_NOT_ZERO) {
      node.add_instruction(it);
      node.set_id(blockId++);
      cfg.emplace_back(node);
      node = cfg::node();
    } else {
      node.add_instruction(it);
    }
  }
  if (!node.is_empty()) {
    node.set_id(blockId++);
    cfg.emplace_back(node);
  }

  cfg::node exitNode = cfg::node();
  exitNode.set_id(blockId);
  cfg.emplace_back(exitNode);

  for (auto node = cfg.begin(); node != cfg.end(); node++) {
    if (node->get_id() == 0 or node->get_id() == blockId)
      continue;
    if (node->get_id() == 1)
      node->add_pred(0);
    auto instr = node->get_body().back();
    if (instr->get_type() == scar::instruction_type::RETURN) {
      node->add_succ(blockId);
      (cfg.end() - 1)->add_pred(node->get_id());
    } else if (instr->get_type() == scar::instruction_type::JUMP) {
      unsigned int dstId = NodeLabelToId[instr->get_src1()->get_label()];
      node->add_succ(dstId);
      (cfg.begin() + dstId)->add_pred(node->get_id());
    } else if (instr->get_type() == scar::instruction_type::JUMP_IF_ZERO or
               instr->get_type() == scar::instruction_type::JUMP_IF_NOT_ZERO) {
      unsigned int dstId = NodeLabelToId[instr->get_dst()->get_label()];
      node->add_succ(dstId);
      (cfg.begin() + dstId)->add_pred(node->get_id());

      node->add_succ(node->get_id() + 1);
      (cfg.begin() + node->get_id() + 1)->add_pred(node->get_id());
    } else {
      node->add_succ(node->get_id() + 1);
      (cfg.begin() + node->get_id() + 1)->add_pred(node->get_id());
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
