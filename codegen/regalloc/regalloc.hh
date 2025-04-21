#pragma once

#include <scasm/scasm.hh>

namespace scarlet {
namespace regalloc {

struct Reg {
  scasm::operand_type type{};
  scasm::register_type reg{};
  std::string pseudoreg{};
  bool operator==(const Reg &other) const {
    if (type != other.type)
      return false;
    if (type == scasm::operand_type::REG)
      return reg == other.reg;
    if (type == scasm::operand_type::PSEUDO)
      return pseudoreg == other.pseudoreg;
    return false;
  }
  bool operator!=(const Reg &other) const { return !(*this == other); }
  bool operator<(const Reg &other) const {
    if (type != other.type) {
      if (type == scasm::operand_type::REG)
        return true;
      else
        return false;
    } else {
      if (type == scasm::operand_type::REG)
        return reg < other.reg;
      else
        return pseudoreg < other.pseudoreg;
    }
  }
};

/**
 * @info:
 * The node class represents a node in the interference graph
 * - neighbors:  the nodes that are connected to the current node
 * - spill_cost: the cost of spilling the current node
 * - reg:        the register assigned to the current node
 * - color:      the color assigned to the current node
 * - pruned:     whether the current node is pruned or not
 */
class node {
private:
  std::vector<std::shared_ptr<node>> neighbors{};
  int spill_cost{};
  Reg reg{};
  int color{};
  bool pruned{};

public:
  void add_neighbor(std::shared_ptr<node> n) { neighbors.emplace_back(n); }
  std::vector<std::shared_ptr<node>> get_neighbors() { return neighbors; }
  void set_spill_cost(int cost) { spill_cost = cost; }
  int get_spill_cost() { return spill_cost; }
  void set_reg(Reg reg) { this->reg = reg; }
  Reg get_reg() { return reg; }
  void set_color(int color) { this->color = color; }
  int get_color() { return color; }
  void prune() { pruned = true; }
  void unprune() { pruned = false; }
  bool is_pruned() { return pruned; }
};

/**
 * @info:
 * The cfg_node class represents a node in the control flow graph
 * id represents the block number
 * succ and pred contains the blocks that are connected to the current block
 * body contains the scasm instructions present in the current block
 */
class cfg_node {
private:
  std::vector<unsigned int> succ{};
  std::vector<unsigned int> pred{};
  unsigned int id{};
  std::vector<std::shared_ptr<scasm::scasm_instruction>> body{};
  bool empty = true;

public:
  std::map<Reg, bool> live_regs;
  void add_succ(unsigned int block) {
    if (std::find(succ.begin(), succ.end(), block) == succ.end())
      succ.emplace_back(block);
  }
  void add_pred(unsigned int block) {
    if (std::find(pred.begin(), pred.end(), block) == pred.end())
      pred.emplace_back(block);
  }
  std::vector<unsigned int> &get_succ() { return succ; }
  std::vector<unsigned int> &get_pred() { return pred; }
  bool is_succ(unsigned int block) {
    return std::find(succ.begin(), succ.end(), block) != succ.end();
  }
  bool is_pred(unsigned int block) {
    return std::find(pred.begin(), pred.end(), block) != pred.end();
  }
  void remove_pred(unsigned int block) {
    pred.erase(std::remove(pred.begin(), pred.end(), block), pred.end());
  }
  void remove_succ(unsigned int block) {
    succ.erase(std::remove(succ.begin(), succ.end(), block), succ.end());
  }
  void set_id(unsigned int id) { this->id = id; }
  unsigned int get_id() { return id; }
  void add_instruction(std::shared_ptr<scasm::scasm_instruction> instruction) {
    body.emplace_back(instruction);
    empty = false;
  }
  std::vector<std::shared_ptr<scasm::scasm_instruction>> &get_body() {
    return body;
  }
  bool is_empty() { return empty; }
};

} // namespace regalloc
} // namespace scarlet
