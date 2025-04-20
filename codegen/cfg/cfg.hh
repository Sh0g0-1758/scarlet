#pragma once

#include <scar/scar.hh>

namespace scarlet {
namespace cfg {

/*
 * Used during data flow analysis. Stores the src and dst of a copy instruction
 */
class copy_info {
private:
  constant::Constant const_val;
  std::string reg_name;
  scar::val_type copy_type;

public:
  void set_const_val(constant::Constant const_val) {
    this->const_val = const_val;
  }
  void set_reg_name(std::string reg_name) { this->reg_name = reg_name; }
  void set_copy_type(scar::val_type copy_type) { this->copy_type = copy_type; }
  constant::Constant get_const_val() { return const_val; }
  std::string get_reg_name() { return reg_name; }
  scar::val_type get_copy_type() { return copy_type; }
  bool operator==(const copy_info &other) const {
    if (copy_type != other.copy_type)
      return false;
    if (copy_type == scar::val_type::CONSTANT)
      return const_val == other.const_val;
    if (copy_type == scar::val_type::VAR)
      return reg_name == other.reg_name;
    return false;
  }
};

/*
 * The node class represents a node in the control flow graph
 * id represents the block number
 * succ and pred contains the blocks that are connected to the current block
 * body contains the scar instructions present in the current block
 */
class node {
private:
  std::vector<unsigned int> succ{};
  std::vector<unsigned int> pred{};
  unsigned int id{};
  std::vector<std::shared_ptr<scar::scar_Instruction_Node>> body{};
  bool empty = true;

public:
  std::map<std::string, copy_info> copy_map{};
  std::map<std::string, bool> live_vars{};
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
  void
  add_instruction(std::shared_ptr<scar::scar_Instruction_Node> instruction) {
    body.emplace_back(instruction);
    empty = false;
  }
  std::vector<std::shared_ptr<scar::scar_Instruction_Node>> &get_body() {
    return body;
  }
  bool is_empty() { return empty; }
};

} // namespace cfg
} // namespace scarlet
