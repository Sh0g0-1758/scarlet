#include <codegen/common.hh>

namespace scarlet {
namespace codegen {

/**
 * @note:
 * While allocating hard registers to pseudoregisters, we take note of any
 * callee saved registers that we used and emit instructions to preserve their
 * values. We do not do the same for caller saved registers because these
 * registers are only ever used for parameter passing, idiv instruction, return
 * value and as scratch registers. For Parameter passing, we copy the values
 * from the registers to the stack and use these stack values, so we don't have
 * to worry about them being clobbered. Though the register allocator will try
 * to use these registers only by replacing the pseudoregister with the
 * corresponding hard register and thus remove this mov instruction. Through the
 * liveness analysis, it will also make sure to use a different hard register or
 * spill the pseudo-register if the hard register gets clobbered later. When
 * there is a function call, we also mark all caller saved registers as
 * clobbered and thus make sure of the correctness of the hard-register
 * assignment. For idiv instruction, the result which gets stored in DX,AX
 * registers must be used immediately after it and thus we do not need to worry
 * about saving it The same is true for return value and scratch register.
 * However the same is not true for callee saved registers because the caller
 * can safely assume that those values are preserved accross function calls and
 * thus do not mark those registers as updated in the liveness analysis. That is
 * why we emit special instructions only for callee saved and not for caller
 * saved.
 * @caller_saved:      AX, DI, SI, DX, CX, R8, R9, R10, R11
 * @return_value:      AX
 * @parameter_passing: DI, SI, DX, CX, R8, R9
 * @scratch_registers: R10, R11
 * @callee_saved:      SP, BP, BX, R12, R13, R14, R15
 * @stack_pointer:     SP
 * @base_pointer:      BP
 */

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
     * @goal: Build an interference graph from the scasm instructions.
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
    std::vector<std::shared_ptr<regalloc::node>> xmm_graph;
    std::vector<scasm::register_type> xmm_registers = {
        scasm::register_type::XMM0,  scasm::register_type::XMM1,
        scasm::register_type::XMM2,  scasm::register_type::XMM3,
        scasm::register_type::XMM4,  scasm::register_type::XMM5,
        scasm::register_type::XMM6,  scasm::register_type::XMM7,
        scasm::register_type::XMM8,  scasm::register_type::XMM9,
        scasm::register_type::XMM10, scasm::register_type::XMM11,
        scasm::register_type::XMM12, scasm::register_type::XMM13,
    };
    for (auto base_reg : base_registers) {
      MAKE_SHARED(regalloc::node, node);
      regalloc::Reg reg;
      reg.type = scasm::operand_type::REG;
      reg.reg = base_reg;
      node->set_reg(reg);
      node->set_spill_cost(INT_MAX);
      graph.emplace_back(node);
    }
    for (auto node : graph) {
      for (auto other : graph) {
        if (node == other)
          continue;
        node->add_neighbor(other);
      }
    }

    for (auto base_reg : xmm_registers) {
      MAKE_SHARED(regalloc::node, node);
      regalloc::Reg reg;
      reg.type = scasm::operand_type::REG;
      reg.reg = base_reg;
      node->set_reg(reg);
      node->set_spill_cost(INT_MAX);
      xmm_graph.emplace_back(node);
    }
    for (auto node : xmm_graph) {
      for (auto other : xmm_graph) {
        if (node == other)
          continue;
        node->add_neighbor(other);
      }
    }

    std::map<std::string, bool> pseudoRegInGraph;
    for (auto &inst : func->get_instructions()) {
      auto src = inst->get_src();
      auto dst = inst->get_dst();
      if (src != nullptr and src->get_type() == scasm::operand_type::PSEUDO) {
        std::string pseudoReg = src->get_identifier();
        if (pseudoRegInGraph[pseudoReg] or aliasedPseudoRegs[pseudoReg])
          continue;
        MAKE_SHARED(regalloc::node, node);
        regalloc::Reg reg;
        reg.type = scasm::operand_type::PSEUDO;
        reg.pseudoreg = pseudoReg;
        node->set_reg(reg);
        if (isDoubleReg(reg))
          xmm_graph.emplace_back(node);
        else
          graph.emplace_back(node);
        pseudoRegInGraph[pseudoReg] = true;
      }
      if (dst != nullptr and dst->get_type() == scasm::operand_type::PSEUDO) {
        std::string pseudoReg = dst->get_identifier();
        if (pseudoRegInGraph[pseudoReg] or aliasedPseudoRegs[pseudoReg])
          continue;
        MAKE_SHARED(regalloc::node, node);
        regalloc::Reg reg;
        reg.type = scasm::operand_type::PSEUDO;
        reg.pseudoreg = pseudoReg;
        node->set_reg(reg);
        if (isDoubleReg(reg))
          xmm_graph.emplace_back(node);
        else
          graph.emplace_back(node);
        pseudoRegInGraph[pseudoReg] = true;
      }
    }

    std::vector<regalloc::cfg_node> cfg;
    gen_cfg_from_funcBody(cfg, func->get_instructions());
    liveness_analysis(cfg, func->get_name());

    for (auto &block : cfg) {
      if (block.is_empty())
        continue;
      auto live_regs = merge_live_regs(cfg, block);
      for (auto it = block.get_body().rbegin(); it != block.get_body().rend();
           ++it) {
        auto [used, updated] = used_and_updated_regs(*it);

        // If a register was updated while another register is still alive,
        // add an edge between the two registers. However, if instruction is a
        // mov instruction and the live register is the source of the mov,
        // we can ignore the edge as the two registers do not interfere.
        for (auto lreg : live_regs) {
          if (!lreg.second)
            continue;
          auto live_reg = lreg.first;

          // (instr is Mov) and (live_reg == instr.src)
          if ((*it)->get_type() == scasm::instruction_type::MOV) {
            auto src = (*it)->get_src();
            if (src->get_type() == scasm::operand_type::REG) {
              if (live_reg.type == scasm::operand_type::REG and
                  live_reg.reg == src->get_reg())
                continue;
            }
            if (src->get_type() == scasm::operand_type::PSEUDO) {
              if (live_reg.type == scasm::operand_type::PSEUDO and
                  live_reg.pseudoreg == src->get_identifier())
                continue;
            }
          }

          for (auto updated_reg : updated) {
            if (isDoubleReg(updated_reg) and isDoubleReg(live_reg)) {
              add_edge(xmm_graph, live_reg, updated_reg);
            } else if (!isDoubleReg(updated_reg) and !isDoubleReg(live_reg)) {
              add_edge(graph, live_reg, updated_reg);
            }
          }
        }
        for (auto reg : updated) {
          live_regs.erase(reg);
        }
        for (auto reg : used) {
          live_regs[reg] = true;
        }
      }
    }

    /**
     * @goal: Add spill costs to the nodes in the graph.
     * @steps:
     *        2.1 Naively assign spill costs according to the number of times
     *            a pseudoregister is used.
     *        2.2 The hard registers are assigned a spill cost of infinity
     */

    std::map<std::string, int> pseudoRegSpillCost;
    for (auto &inst : func->get_instructions()) {
      auto src = inst->get_src();
      auto dst = inst->get_dst();
      if (src != nullptr and src->get_type() == scasm::operand_type::PSEUDO) {
        std::string pseudoReg = src->get_identifier();
        pseudoRegSpillCost[pseudoReg]++;
      }
      if (dst != nullptr and dst->get_type() == scasm::operand_type::PSEUDO) {
        std::string pseudoReg = dst->get_identifier();
        pseudoRegSpillCost[pseudoReg]++;
      }
    }
    for (auto node : graph) {
      if (node->get_reg().type == scasm::operand_type::PSEUDO) {
        node->set_spill_cost(pseudoRegSpillCost[node->get_reg().pseudoreg]);
      }
    }
    for (auto node : xmm_graph) {
      if (node->get_reg().type == scasm::operand_type::PSEUDO) {
        node->set_spill_cost(pseudoRegSpillCost[node->get_reg().pseudoreg]);
      }
    }

    /**
     * @goal: Color the graph using a graph coloring algorithm.
     * @steps:
     *        3.1 prune nodes with degree less than k and put them in a stack
     *        3.2 repeat this process until we either cannot prune or no node is
     * left 3.3 if we cannot prune, spill the node with the lowest spill cost
     *        3.4 assign color to the nodes
     */
    color_graph(graph, 12);
    color_graph(xmm_graph, 14);

    /**
     * @goal: Create a register map from the colored graph.
     * @steps:
     *        4.1 create a map from the color to hard registers
     *        4.2 use the map to assign registers to the pseudoregisters
     *        4.3 save the callee saved registers used by a function
     */
    std::map<int, scasm::register_type> colorToReg;
    for (auto node : graph) {
      if (node->get_reg().type == scasm::operand_type::REG) {
        colorToReg[node->get_color()] = node->get_reg().reg;
      }
    }
    std::map<std::string, scasm::register_type> pseudoRegToReg;
    for (auto node : graph) {
      if (node->get_reg().type == scasm::operand_type::PSEUDO) {
        if (node->get_color() != 0) {
          auto hard_reg = colorToReg[node->get_color()];
          pseudoRegToReg[node->get_reg().pseudoreg] = hard_reg;
          if (callee_savedReg[hard_reg])
            calleeSavedRegisters[func->get_name()].insert(hard_reg);
        }
      }
    }
    colorToReg.clear();
    for (auto node : xmm_graph) {
      if (node->get_reg().type == scasm::operand_type::REG) {
        colorToReg[node->get_color()] = node->get_reg().reg;
      }
    }
    std::map<std::string, scasm::register_type> pseudoRegToXMMReg;
    for (auto node : xmm_graph) {
      if (node->get_reg().type == scasm::operand_type::PSEUDO) {
        if (node->get_color() != 0) {
          auto hard_reg = colorToReg[node->get_color()];
          pseudoRegToXMMReg[node->get_reg().pseudoreg] = hard_reg;
        }
      }
    }

    /**
     * @goal: Replace pseudoregisters in the instructions with the allocated
     *        hard registers.
     * @steps:
     *        5.1 Simply use the map to figure out the hard register allocated
     *            to a particular pseudoregister
     *        5.2 Remove instructions in which we move a hard register to itself
     */
    for (auto it = func->get_instructions().begin();
         it != func->get_instructions().end();) {
      auto src = (*it)->get_src();
      auto dst = (*it)->get_dst();
      if (src != nullptr and src->get_type() == scasm::operand_type::PSEUDO) {
        std::string pseudoReg = src->get_identifier();
        if (pseudoRegToReg.find(pseudoReg) != pseudoRegToReg.end()) {
          src->set_type(scasm::operand_type::REG);
          src->set_reg(pseudoRegToReg[pseudoReg]);
        }
        if (pseudoRegToXMMReg.find(pseudoReg) != pseudoRegToXMMReg.end()) {
          src->set_type(scasm::operand_type::REG);
          src->set_reg(pseudoRegToXMMReg[pseudoReg]);
        }
      }
      if (dst != nullptr and dst->get_type() == scasm::operand_type::PSEUDO) {
        std::string pseudoReg = dst->get_identifier();
        if (pseudoRegToReg.find(pseudoReg) != pseudoRegToReg.end()) {
          dst->set_type(scasm::operand_type::REG);
          dst->set_reg(pseudoRegToReg[pseudoReg]);
        }
        if (pseudoRegToXMMReg.find(pseudoReg) != pseudoRegToXMMReg.end()) {
          dst->set_type(scasm::operand_type::REG);
          dst->set_reg(pseudoRegToXMMReg[pseudoReg]);
        }
      }
      if ((*it)->get_type() == scasm::instruction_type::MOV and
          src != nullptr and dst != nullptr and
          src->get_type() == scasm::operand_type::REG and
          dst->get_type() == scasm::operand_type::REG and
          src->get_reg() == dst->get_reg()) {
        it = func->get_instructions().erase(it);
      } else {
        ++it;
      }
    }

    std::string funcName = func->get_name();
    std::vector<scasm::register_type> calleeSavedRegs{};
    for (auto it : calleeSavedRegisters[funcName])
      calleeSavedRegs.emplace_back(it);

    // push the callee saved registers on the stack
    int regNum = 0;
    for (auto it = calleeSavedRegs.begin(); it != calleeSavedRegs.end(); it++) {
      MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
      scasm_inst->set_type(scasm::instruction_type::PUSH);
      scasm_inst->set_asm_type(scasm::AssemblyType::QUAD_WORD);
      MAKE_SHARED(scasm::scasm_operand, scasm_src);
      scasm_src->set_type(scasm::operand_type::REG);
      scasm_src->set_reg(*it);
      scasm_inst->set_src(std::move(scasm_src));
      func->get_instructions().insert(func->get_instructions().begin() + regNum,
                                      scasm_inst);
      ++regNum;
    }

    // pop the callee saved value from the stack into the register before every
    // ret instr
    for (auto it = func->get_instructions().begin();
         it != func->get_instructions().end(); ++it) {
      if ((*it)->get_type() == scasm::instruction_type::RET) {
        for (auto reg = calleeSavedRegs.rbegin(); reg != calleeSavedRegs.rend();
             ++reg) {
          MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
          scasm_inst->set_type(scasm::instruction_type::POP);
          scasm_inst->set_asm_type(scasm::AssemblyType::QUAD_WORD);
          MAKE_SHARED(scasm::scasm_operand, scasm_src);
          scasm_src->set_type(scasm::operand_type::REG);
          scasm_src->set_reg(*reg);
          scasm_inst->set_src(std::move(scasm_src));
          it = func->get_instructions().insert(it, scasm_inst);
          ++it;
        }
      }
    }
  }
}

void Codegen::color_graph(std::vector<std::shared_ptr<regalloc::node>> &graph,
                          int k) {
  std::vector<std::shared_ptr<regalloc::node>> remaining;
  for (auto node : graph) {
    if (node->is_pruned())
      continue;
    remaining.emplace_back(node);
  }
  if (remaining.empty())
    return;

  std::shared_ptr<regalloc::node> chosen_node = nullptr;
  for (auto node : remaining) {
    int unpruned_neighbors = 0;
    for (auto neighbor : node->get_neighbors()) {
      if (!neighbor->is_pruned()) {
        unpruned_neighbors++;
      }
    }
    if (unpruned_neighbors < k) {
      node->prune();
      chosen_node = node;
      break;
    }
  }

  if (chosen_node == nullptr) {
    double best_spill_metric = INT_MAX;
    for (auto node : remaining) {
      int degree = 0;
      for (auto neighbor : node->get_neighbors()) {
        if (!neighbor->is_pruned()) {
          degree++;
        }
      }
      double spill_metric = (double)node->get_spill_cost() / degree;
      if (spill_metric < best_spill_metric) {
        best_spill_metric = spill_metric;
        chosen_node = node;
      }
    }
    chosen_node->prune();
  }
  color_graph(graph, k);
  std::map<int, bool> colors;
  for (int i = 1; i <= k; i++)
    colors[i] = true;
  for (auto neighbour : chosen_node->get_neighbors()) {
    if (neighbour->get_color() != 0)
      colors[neighbour->get_color()] = false;
  }
  std::vector<int> available_colors;
  for (auto color : colors)
    if (color.second)
      available_colors.push_back(color.first);
  if (!available_colors.empty()) {
    if (chosen_node->get_reg().type == scasm::operand_type::PSEUDO) {
      chosen_node->set_color(available_colors[0]);
    } else {
      if (callee_savedReg[chosen_node->get_reg().reg]) {
        chosen_node->set_color(available_colors[available_colors.size() - 1]);
      } else {
        chosen_node->set_color(available_colors[0]);
      }
    }
    chosen_node->unprune();
  }
}

/************************************************************************
 ********** LIVENESS ANALYIS AND CONTROL FLOW GRAPH FOR SCASM ***********
 ************************************************************************/

void Codegen::alias_analyis(
    std::vector<std::shared_ptr<scasm::scasm_instruction>> &funcBody) {
  // in scasm_generation, we already converted pseudoregisters which are static
  // variables to data type, hence we do not need to worry about them here
  aliasedPseudoRegs.clear();
  for (auto instr : funcBody) {
    if (instr->get_type() == scasm::instruction_type::LEA) {
      auto src = instr->get_src();
      if (src != nullptr and src->get_type() == scasm::operand_type::PSEUDO)
        aliasedPseudoRegs[src->get_identifier()] = true;
    }
  }
}

void Codegen::liveness_analysis(std::vector<regalloc::cfg_node> &cfg,
                                std::string funcName) {
  std::queue<unsigned int> worklist;
  std::map<unsigned int, bool> worklistMap;
  initialize_worklist(cfg, cfg[0], worklist, worklistMap);

  if (globalSymbolTable[funcName].typeDef[0] == ast::ElemType::VOID) {
    // do nothing
  } else {
    regalloc::Reg reg;
    reg.type = scasm::operand_type::REG;
    if (globalSymbolTable[funcName].typeDef[0] == ast::ElemType::DOUBLE) {
      reg.reg = scasm::register_type::XMM0;
    } else {
      reg.reg = scasm::register_type::AX;
    }
    cfg[cfg.size() - 1].live_regs[reg] = true;
  }

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

// It is possible that SP or BP register have become a part of the
// live registers because of scasm generation during function call
// (16-byte alignment) and stack cleanup after the function call
// and function declaration (to copy arguments on the stack)
// For sanity, we also check for scratch registers
#define IS_BASE_REG(operand)                                                   \
  operand->get_reg() != scasm::register_type::SP and                           \
      operand->get_reg() != scasm::register_type::BP and                       \
      operand->get_reg() != scasm::register_type::R10 and                      \
      operand->get_reg() != scasm::register_type::R11 and                      \
      operand->get_reg() != scasm::register_type::XMM14 and                    \
      operand->get_reg() != scasm::register_type::XMM15

#define CHECK_USED(operand)                                                    \
  if (operand != nullptr) {                                                    \
    if (operand->get_type() == scasm::operand_type::REG and                    \
        IS_BASE_REG(operand)) {                                                \
      regalloc::Reg reg;                                                       \
      reg.type = scasm::operand_type::REG;                                     \
      reg.reg = operand->get_reg();                                            \
      used.insert(reg);                                                        \
    } else if (operand->get_type() == scasm::operand_type::PSEUDO and          \
               !aliasedPseudoRegs[operand->get_identifier()]) {                \
      regalloc::Reg reg;                                                       \
      reg.pseudoreg = operand->get_identifier();                               \
      reg.type = scasm::operand_type::PSEUDO;                                  \
      used.insert(reg);                                                        \
    } else if (operand->get_type() == scasm::operand_type::MEMORY and          \
               IS_BASE_REG(operand)) {                                         \
      regalloc::Reg reg;                                                       \
      reg.type = scasm::operand_type::REG;                                     \
      reg.reg = operand->get_reg();                                            \
      used.insert(reg);                                                        \
    } else if (operand->get_type() == scasm::operand_type::INDEXED and         \
               IS_BASE_REG(operand)) {                                         \
      regalloc::Reg reg;                                                       \
      reg.type = scasm::operand_type::REG;                                     \
      reg.reg = operand->get_reg();                                            \
      used.insert(reg);                                                        \
      regalloc::Reg reg2;                                                      \
      reg2.type = scasm::operand_type::REG;                                    \
      reg2.reg = operand->get_index();                                         \
      used.insert(reg2);                                                       \
    }                                                                          \
  }

#define CHECK_UPDATED(operand)                                                 \
  if (operand != nullptr) {                                                    \
    if (operand->get_type() == scasm::operand_type::REG and                    \
        IS_BASE_REG(operand)) {                                                \
      regalloc::Reg reg;                                                       \
      reg.type = scasm::operand_type::REG;                                     \
      reg.reg = operand->get_reg();                                            \
      updated.insert(reg);                                                     \
    } else if (operand->get_type() == scasm::operand_type::PSEUDO and          \
               !aliasedPseudoRegs[operand->get_identifier()]) {                \
      regalloc::Reg reg;                                                       \
      reg.type = scasm::operand_type::PSEUDO;                                  \
      reg.pseudoreg = operand->get_identifier();                               \
      updated.insert(reg);                                                     \
    } else if (operand->get_type() == scasm::operand_type::MEMORY and          \
               IS_BASE_REG(operand)) {                                         \
      regalloc::Reg reg;                                                       \
      reg.type = scasm::operand_type::REG;                                     \
      reg.reg = operand->get_reg();                                            \
      used.insert(reg);                                                        \
    }                                                                          \
  }

std::pair<std::set<regalloc::Reg>, std::set<regalloc::Reg>>
Codegen::used_and_updated_regs(
    std::shared_ptr<scasm::scasm_instruction> instr) {
  auto src = instr->get_src();
  auto dst = instr->get_dst();
  auto instrType = instr->get_type();

  std::set<regalloc::Reg> used{};
  std::set<regalloc::Reg> updated{};

  if (instrType == scasm::instruction_type::MOV) {
    CHECK_USED(src);
    CHECK_UPDATED(dst);
  } else if (instrType == scasm::instruction_type::BINARY) {
    CHECK_USED(src);
    CHECK_USED(dst);
    CHECK_UPDATED(dst);
  } else if (instrType == scasm::instruction_type::UNARY) {
    CHECK_USED(dst);
    CHECK_UPDATED(dst);
  } else if (instrType == scasm::instruction_type::CMP) {
    CHECK_USED(src);
    CHECK_USED(dst);
  } else if (instrType == scasm::instruction_type::SETCC) {
    CHECK_UPDATED(dst);
  } else if (instrType == scasm::instruction_type::PUSH) {
    CHECK_USED(src);
  } else if (instrType == scasm::instruction_type::POP) {
    CHECK_UPDATED(src);
  } else if (instrType == scasm::instruction_type::IDIV or
             instrType == scasm::instruction_type::DIV) {
    CHECK_USED(src);
    regalloc::Reg reg_ax;
    reg_ax.type = scasm::operand_type::REG;
    reg_ax.reg = scasm::register_type::AX;
    regalloc::Reg reg_dx;
    reg_dx.type = scasm::operand_type::REG;
    reg_dx.reg = scasm::register_type::DX;
    used.insert(reg_ax);
    used.insert(reg_dx);
    updated.insert(reg_ax);
    updated.insert(reg_dx);
  } else if (instrType == scasm::instruction_type::CDQ) {
    regalloc::Reg reg_ax;
    reg_ax.type = scasm::operand_type::REG;
    reg_ax.reg = scasm::register_type::AX;
    regalloc::Reg reg_dx;
    reg_dx.type = scasm::operand_type::REG;
    reg_dx.reg = scasm::register_type::DX;
    used.insert(reg_ax);
    updated.insert(reg_dx);
  } else if (instrType == scasm::instruction_type::CALL) {
    std::string funcName = src->get_identifier();
    for (auto it : funcParamRegs[funcName]) {
      regalloc::Reg reg;
      reg.type = scasm::operand_type::REG;
      reg.reg = it;
      used.insert(reg);
    }

    // Mark all caller saved registers as updated
    for (auto it : caller_savedReg) {
      regalloc::Reg reg;
      reg.type = scasm::operand_type::REG;
      reg.reg = it.first;
      updated.insert(reg);
    }
    for (auto it : xmm_caller_savedReg) {
      regalloc::Reg reg;
      reg.type = scasm::operand_type::REG;
      reg.reg = it.first;
      updated.insert(reg);
    }
  } else if (instrType == scasm::instruction_type::MOVSX) {
    CHECK_USED(src);
    CHECK_UPDATED(dst);
  } else if (instrType == scasm::instruction_type::MOVZX) {
    CHECK_USED(src);
    CHECK_UPDATED(dst);
  } else if (instrType == scasm::instruction_type::LEA) {
    CHECK_USED(src);
    CHECK_UPDATED(dst);
  } else if (instrType == scasm::instruction_type::CVTSI2SD) {
    CHECK_USED(src);
    CHECK_UPDATED(dst);
  } else if (instrType == scasm::instruction_type::CVTTS2DI) {
    CHECK_USED(src);
    CHECK_UPDATED(dst);
  }
  return {used, updated};
}

void Codegen::transfer_live_regs(regalloc::cfg_node &block) {
  if (block.is_empty())
    return;
  for (auto it = block.get_body().rbegin(); it != block.get_body().rend();
       ++it) {
    auto [used, updated] = used_and_updated_regs(*it);
    for (auto reg : updated) {
      block.live_regs.erase(reg);
    }
    for (auto reg : used) {
      block.live_regs[reg] = true;
    }
  }
}

std::map<regalloc::Reg, bool>
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
