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
        alias_analysis(cfg);
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
        bool ran_dead_store_elimination{};
        if (enable_dead_store_elimination or enable_all) {
          ran_dead_store_elimination = dead_store_elimination(cfg);
        }
        if (!ran_constant_folding and !ran_unreachable_code_elimination and
            !ran_copy_propagation and !ran_dead_store_elimination)
          break;
      }
      gen_funcBody_from_cfg(cfg, funcBody);
    }
  }
}

/*
 * aliased variables are variables whose value can be changed in ways other than
 * normal assignment / flow of the function. For instance, a pointer can change
 * the value of a variable through a store instruction and a function call can
 * potentially update variables with global linkage and even local linkage
 * through pointers. This pass is run for each specific function and the
 * information it collects is local to that function only.
 */
void Codegen::alias_analysis(std::vector<cfg::node> &cfg) {
  aliased_vars.clear();
  for (auto block = cfg.begin(); block != cfg.end(); ++block) {
    if (block->is_empty())
      continue;
    for (auto instr : block->get_body()) {
      if (instr->get_type() == scar::instruction_type::CALL) {
        auto args =
            std::static_pointer_cast<scar::scar_FunctionCall_Instruction_Node>(
                instr)
                ->get_args();
        for (auto arg : args) {
          if (arg->get_type() == scar::val_type::VAR and
              globalSymbolTable[arg->get_reg()].link !=
                  symbolTable::linkage::NONE) {
            aliased_vars[arg->get_reg()] = true;
          }
        }
        auto dst = instr->get_dst();
        if (dst != nullptr and dst->get_type() == scar::val_type::VAR and
            globalSymbolTable[dst->get_reg()].link !=
                symbolTable::linkage::NONE) {
          aliased_vars[dst->get_reg()] = true;
        }
      } else if (instr->get_type() == scar::instruction_type::GET_ADDRESS) {
        aliased_vars[instr->get_src1()->get_reg()] = true;
      } else {
        auto src1 = instr->get_src1();
        auto src2 = instr->get_src2();
        auto dst = instr->get_dst();
        if (src1 != nullptr and src1->get_type() == scar::val_type::VAR and
            globalSymbolTable[src1->get_reg()].link !=
                symbolTable::linkage::NONE) {
          aliased_vars[src1->get_reg()] = true;
        }
        if (src2 != nullptr and src2->get_type() == scar::val_type::VAR and
            globalSymbolTable[src2->get_reg()].link !=
                symbolTable::linkage::NONE) {
          aliased_vars[src2->get_reg()] = true;
        }
        if (dst != nullptr and dst->get_type() == scar::val_type::VAR and
            globalSymbolTable[dst->get_reg()].link !=
                symbolTable::linkage::NONE) {
          aliased_vars[dst->get_reg()] = true;
        }
      }
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
  nodeLabelToId.clear();
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
      nodeLabelToId[it->get_src1()->get_label()] = blockId;
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
      unsigned int dstId = nodeLabelToId[instr->get_src1()->get_label()];
      node->add_succ(dstId);
      (cfg.begin() + dstId)->add_pred(node->get_id());
    } else if (instr->get_type() == scar::instruction_type::JUMP_IF_ZERO or
               instr->get_type() == scar::instruction_type::JUMP_IF_NOT_ZERO) {
      unsigned int dstId = nodeLabelToId[instr->get_dst()->get_label()];
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
