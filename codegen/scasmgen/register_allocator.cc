#include <codegen/common.hh>

namespace scarlet {
namespace codegen {

/**
 * @brief: Allocates registers for the given instructions using graph coloring.
 * @structure:
 * 1. Build an interference graph from the instructions.
 * 2. Add spill costs to the nodes in the graph.
 * 3. Color the graph using a graph coloring algorithm.
 * 4. Create a register map from the colored graph.
 * 5. Replace pseudoregisters in the instructions with the allocated registers.
 */
void Codegen::allocate_registers() {
  for (auto &elem : scasm.get_elems()) {
    if (elem->get_type() != scasm::scasm_top_level_type::FUNCTION)
      continue;
    auto func = std::static_pointer_cast<scasm::scasm_function>(elem);
    /**
     * @goal: Build an interference graph from the instructions.
     * @steps:
     *        1.1 Create a base graph with available hard registers.
     *        1.2 Add pseudoregisters to the graph.
     *        1.3 Analyze liveness of the instructions.
     *        1.4 Add edges to the graph based on liveness analysis.
     */

    std::vector<std::shared_ptr<regalloc::node>> graph;
    std::vector<scasm::register_type> base_registers = {
        scasm::register_type::AX,  scasm::register_type::BX,
        scasm::register_type::CX,  scasm::register_type::DX,
        scasm::register_type::DI,  scasm::register_type::SI,
        scasm::register_type::R8,  scasm::register_type::R9,
        scasm::register_type::R12, scasm::register_type::R13,
        scasm::register_type::R14, scasm::register_type::R15};
    for (auto reg : base_registers) {
      MAKE_SHARED(regalloc::node, node);
      node->set_reg(reg);
      node->set_type(scasm::operand_type::REG);
      graph.emplace_back(node);
    }
    for (auto node : graph) {
      for (auto other : graph) {
        if (node == other)
          continue;
        node->add_neighbor(other);
      }
    }
    std::map<std::string, bool> pseudoRegInGraph;
    for (auto &inst : func->get_instructions()) {
      auto src = inst->get_src();
      auto dst = inst->get_dst();
      if (src->get_type() == scasm::operand_type::PSEUDO) {
        std::string pseudoReg = src->get_identifier();
        if (pseudoRegInGraph[pseudoReg])
          continue;
        MAKE_SHARED(regalloc::node, node);
        node->set_pseudoreg(pseudoReg);
        node->set_type(scasm::operand_type::PSEUDO);
        graph.emplace_back(node);
        pseudoRegInGraph[pseudoReg] = true;
      }
      if (dst->get_type() == scasm::operand_type::PSEUDO) {
        std::string pseudoReg = dst->get_identifier();
        if (pseudoRegInGraph[pseudoReg])
          continue;
        MAKE_SHARED(regalloc::node, node);
        node->set_pseudoreg(pseudoReg);
        node->set_type(scasm::operand_type::PSEUDO);
        graph.emplace_back(node);
        pseudoRegInGraph[pseudoReg] = true;
      }
    }

    std::vector<regalloc::cfg_node> cfg;
    gen_cfg_from_funcBody(cfg, func->get_instructions());
  }
}

void Codegen::liveness_analysis(std::vector<regalloc::cfg_node> &cfg) {
  std::queue<unsigned int> worklist;
  std::map<unsigned int, bool> worklistMap;
  initialize_worklist(cfg, cfg[0], worklist, worklistMap);

  regalloc::reg reg;
  reg.type = scasm::operand_type::REG;
  reg.reg = scasm::register_type::AX;
  cfg[cfg.size() - 1].live_regs[reg] = true;

  while (!worklist.empty()) {
    auto &block = getNodeFromID(cfg, worklist.front());
    worklistMap[worklist.front()] = false;
    worklist.pop();
    auto old_live_regs = block.live_regs;
    block.live_regs = merge_live_regs(cfg, block);
    transfer_live_regs(block);
    auto new_live_regs = block.live_regs;
    if (old_live_regs != new_live_regs) {
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
}

void Codegen::transfer_live_regs(regalloc::cfg_node &block) {
  if (block.is_empty())
    return;
  for (auto it = block.get_body().end() - 1; it >= block.get_body().begin();
       --it) {
    auto instr = *it;
    auto src = instr->get_src();
    auto dst = instr->get_dst();
    auto instrType = instr->get_type();
  }
}

std::map<regalloc::reg, bool>
Codegen::merge_live_regs(std::vector<regalloc::cfg_node> &cfg,
                         regalloc::cfg_node &block) {
  if (block.get_succ().empty())
    return {};

  auto initMap = getNodeFromID(cfg, block.get_succ()[0]).live_regs;
  for (int i = 1; i < (int)block.get_succ().size(); i++) {
    for (auto it : getNodeFromID(cfg, block.get_succ()[i]).live_regs) {
      initMap[it.first] = it.second;
    }
  }
  return initMap;
}

// Initialize the worklist in postorder
void Codegen::initialize_worklist(std::vector<regalloc::cfg_node> &cfg,
                                  regalloc::cfg_node &block,
                                  std::queue<unsigned int> &worklist,
                                  std::map<unsigned int, bool> &worklistMap) {
  worklistMap[block.get_id()] = true;
  for (auto succID : block.get_succ()) {
    if (worklistMap[succID])
      continue;
    initialize_worklist(cfg, getNodeFromID(cfg, succID), worklist, worklistMap);
  }
  if (block.is_empty())
    return;
  worklist.push(block.get_id());
}

void Codegen::gen_cfg_from_funcBody(
    std::vector<regalloc::cfg_node> &cfg,
    std::vector<std::shared_ptr<scasm::scasm_instruction>> &funcBody) {
  cfgNodeLabelToId.clear();
  regalloc::cfg_node entryNode = regalloc::cfg_node();
  entryNode.set_id(0);
  entryNode.add_succ(1);
  cfg.emplace_back(entryNode);

  regalloc::cfg_node node = regalloc::cfg_node();
  unsigned int blockId = 1;
  for (auto it : funcBody) {
    auto instType = it->get_type();
    if (instType == scasm::instruction_type::LABEL) {
      if (!node.is_empty()) {
        node.set_id(blockId++);
        cfg.emplace_back(node);
        node = regalloc::cfg_node();
      }
      cfgNodeLabelToId[it->get_src()->get_identifier()] = blockId;
      node.add_instruction(it);
    } else if (instType == scasm::instruction_type::RET or
               instType == scasm::instruction_type::JMP or
               instType == scasm::instruction_type::JMPCC) {
      node.add_instruction(it);
      node.set_id(blockId++);
      cfg.emplace_back(node);
      node = regalloc::cfg_node();
    } else {
      node.add_instruction(it);
    }
  }
  if (!node.is_empty()) {
    node.set_id(blockId++);
    cfg.emplace_back(node);
  }

  regalloc::cfg_node exitNode = regalloc::cfg_node();
  exitNode.set_id(blockId);
  cfg.emplace_back(exitNode);

  for (auto node = cfg.begin(); node != cfg.end(); node++) {
    if (node->get_id() == 0 or node->get_id() == blockId)
      continue;
    if (node->get_id() == 1)
      node->add_pred(0);
    auto instr = node->get_body().back();
    if (instr->get_type() == scasm::instruction_type::RET) {
      node->add_succ(blockId);
      (cfg.end() - 1)->add_pred(node->get_id());
    } else if (instr->get_type() == scasm::instruction_type::JMP) {
      unsigned int dstId = cfgNodeLabelToId[instr->get_src()->get_identifier()];
      node->add_succ(dstId);
      (cfg.begin() + dstId)->add_pred(node->get_id());
    } else if (instr->get_type() == scasm::instruction_type::JMPCC) {
      unsigned int dstId = cfgNodeLabelToId[instr->get_dst()->get_identifier()];
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

} // namespace codegen
} // namespace scarlet
