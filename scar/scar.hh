// ####################
// ## IR for scarlet ##
// ####################

#pragma once

#include "unop.hh"
#include <string>
#include <vector>

/*
Grammar:

program = Program(function_definition)
function_definition = Function(identifier, instruction* body)
instruction = Return(val) | Unary(unary_operator, val src, val dst)
val = Constant(int) | Var(identifier)
unary_operator = Complement | Negate

*/

class scar {
public:
  virtual std::string get_scar_name() { return "scar"; }
  virtual ~scar() = default;
};

class scar_Identifier_Node : public scar {
private:
  std::string value;

public:
  std::string get_scar_name() override { return "Identifier"; }
  std::string get_value() { return value; }
  void set_value(std::string value) { this->value = std::move(value); }
};

class scar_Val_Node : public scar {
private:
  std::string type;
  std::string val;
  std::string reg_name;

public:
  std::string get_scar_name() override { return "Val"; }
  std::string get_type() { return type; }
  void set_type(std::string type) { this->type = std::move(type); }
  std::string get_value() { return val; }
  void set_value(std::string value) { this->val = std::move(value); }
  std::string get_reg() { return reg_name; }
  void set_reg_name(std::string identifier) {
    this->reg_name = std::move(identifier);
  }
};

class scar_Instruction_Node : public scar {
private:
  std::string type;
  UNOP op;               // When the instruction is not a return instruction
  scar_Val_Node src_ret; // This can act as both the source and return value
  scar_Val_Node dst;

public:
  std::string get_scar_name() override { return "Instruction"; }
  std::string get_type() { return type; }
  void set_type(std::string type) { this->type = std::move(type); }
  UNOP get_op() { return op; }
  void set_op(UNOP op) { this->op = op; }
  scar_Val_Node get_src_ret() { return src_ret; }
  void set_src_ret(scar_Val_Node src_ret) {
    this->src_ret = std::move(src_ret);
  }
  scar_Val_Node get_dst() { return dst; }
  void set_dst(scar_Val_Node dst) { this->dst = std::move(dst); }
};

class scar_Function_Node : public scar {
private:
  scar_Identifier_Node identifier;
  std::vector<scar_Instruction_Node> body;

public:
  scar_Function_Node() { body.reserve(2); }
  std::string get_scar_name() override { return "Function"; }
  scar_Identifier_Node get_identifier() { return identifier; }
  void set_identifier(scar_Identifier_Node identifier) {
    this->identifier = std::move(identifier);
  }
  void add_instruction(scar_Instruction_Node instruction) {
    body.emplace_back(instruction);
  }
  std::vector<scar_Instruction_Node> &get_instructions() { return body; }
};

class scar_Program_Node : public scar {
private:
  std::vector<scar_Function_Node> functions;

public:
  scar_Program_Node() { functions.reserve(2); }
  std::string get_scar_name() override { return "Program"; }
  std::vector<scar_Function_Node> &get_functions() { return functions; }
  void add_function(scar_Function_Node function) {
    functions.emplace_back(function);
  }
};
