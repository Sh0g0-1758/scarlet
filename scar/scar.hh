// ####################
// ## IR for scarlet ##
// ####################

#pragma once

#include "binary_operations/binop.hh"
#include "unary_operations/unop.hh"
#include <string>
#include <vector>

// clang-format off

/*
Grammar:

program = Program(function_definition)
function_definition = Function(identifier, instruction* body)
instruction = Return(val) | Unary(unary_operator, val src, val dst) | Binary(binary_operator, val src1, val src2, val dst)
val = Constant(int) | Var(identifier)
unary_operator = Complement | Negate
binary_operator = Add | Subtract | Multiply | Divide | Remainder

*/

// clang-format on

namespace scarlet {
namespace scar {

enum class scar_instruction_type { RETURN, UNARY, BINARY };
enum class scar_val_type { CONSTANT, VAR };

std::string to_string(scar_instruction_type type);
std::string to_string(scar_val_type type);

class scar_Identifier_Node {
private:
  std::string value;

public:
  std::string get_scar_name() { return "Identifier"; }
  std::string get_value() { return value; }
  void set_value(std::string value) { this->value = std::move(value); }
};

class scar_Val_Node {
private:
  scar_val_type type;
  std::string val;
  std::string reg_name;

public:
  std::string get_scar_name() { return "Val"; }
  scar_val_type get_type() { return type; }
  void set_type(scar_val_type type) { this->type = std::move(type); }
  std::string get_value() { return val; }
  void set_value(std::string value) { this->val = std::move(value); }
  std::string get_reg() { return reg_name; }
  void set_reg_name(std::string identifier) {
    this->reg_name = std::move(identifier);
  }
};

class scar_Instruction_Node {
private:
  scar_instruction_type type; // Return, Unary, Binary
  unop::UNOP unop;            // When the instruction is a unary instruction
  binop::BINOP binop;         // When the instruction is a binary instruction
  scar_Val_Node src_ret; // This can act as both the source and return value
  scar_Val_Node src2;
  scar_Val_Node dst;

public:
  std::string get_scar_name() { return "Instruction"; }
  scar_instruction_type get_type() { return type; }
  void set_type(scar_instruction_type type) { this->type = std::move(type); }
  unop::UNOP get_unop() { return unop; }
  void set_unop(unop::UNOP unop) { this->unop = unop; }
  binop::BINOP get_binop() { return binop; }
  void set_binop(binop::BINOP binop) { this->binop = binop; }
  scar_Val_Node &get_src_ret() { return src_ret; }
  void set_src_ret(scar_Val_Node src_ret) {
    this->src_ret = std::move(src_ret);
  }
  scar_Val_Node &get_src2() { return src2; }
  void set_src2(scar_Val_Node src2) { this->src2 = std::move(src2); }
  scar_Val_Node &get_dst() { return dst; }
  void set_dst(scar_Val_Node dst) { this->dst = std::move(dst); }
};

class scar_Function_Node {
private:
  scar_Identifier_Node identifier;
  std::vector<scar_Instruction_Node> body;

public:
  scar_Function_Node() { body.reserve(2); }
  std::string get_scar_name() { return "Function"; }
  scar_Identifier_Node &get_identifier() { return identifier; }
  void set_identifier(scar_Identifier_Node identifier) {
    this->identifier = std::move(identifier);
  }
  void add_instruction(scar_Instruction_Node instruction) {
    body.emplace_back(instruction);
  }
  std::vector<scar_Instruction_Node> &get_instructions() { return body; }
};

class scar_Program_Node {
private:
  std::vector<scar_Function_Node> functions;

public:
  scar_Program_Node() { functions.reserve(2); }
  std::string get_scar_name() { return "Program"; }
  std::vector<scar_Function_Node> &get_functions() { return functions; }
  void add_function(scar_Function_Node function) {
    functions.emplace_back(function);
  }
};

} // namespace scar
} // namespace scarlet
