#include <codegen/common.hh>
#include <scar/scar.hh>

namespace scarlet {
namespace cfg {

/*
 * The node class represents a node in the control flow graph
 * id represents the block number
 * succ and pred contains the blocks that are connected to the current block
 * start and end point to the start and end scar instructions that the block contains
 */
class node {
private:
    std::vector<unsigned int> succ{};
    std::vector<unsigned int> pred{};
    unsigned int id{};
    std::vector<std::shared_ptr<scar::scar_Instruction_Node>> body;
    bool empty = true;
public:
    void add_succ(unsigned int block) { succ.push_back(block); }
    void add_pred(unsigned int block) { pred.push_back(block); }
    std::vector<unsigned int> get_succ() { return succ; }
    std::vector<unsigned int> get_pred() { return pred; }
    void remove_pred(unsigned int block) { pred.erase(std::remove(pred.begin(), pred.end(), block), pred.end()); }
    void remove_succ(unsigned int block) { succ.erase(std::remove(succ.begin(), succ.end(), block), succ.end()); }
    void set_id(unsigned int id) { this->id = id; }
    unsigned int get_id() { return id; }
    void add_instruction(std::shared_ptr<scar::scar_Instruction_Node> instruction) { body.emplace_back(instruction); empty = false; }
    std::vector<std::shared_ptr<scar::scar_Instruction_Node>>& get_body() { return body; }
    bool is_empty() { return empty; }
};

} // namespace cfg
} // namespace scarlet
