// ####################
// ## IR for scarlet ##
// ####################

#pragma once

#include "binary_operations/binop.hh"
#include "unary_operations/unop.hh"
#include <iostream>
#include <memory>
#include <string>
#include <vector>

// clang-format off

/*
Grammar:

program = Program(function_definition)
function_definition = Function(identifier, instruction* body)
instruction = Return(val) | Unary(unary_operator, val src, val dst) | Binary(binary_operator, val src1, val src2, val dst) | Copy(val src, val dst) | Jump(identifier target) | JumpIfZero(val condition, identifier target) | JumpIfNotZero(val condition, identifier target) | Label(Identifier)
val = Constant(int) | Var(identifier)
unary_operator = Complement | Negate | Not
binary_operator = Add | Subtract | Multiply | Divide | Remainder | And | Or | Xor | leftShift | rightShift | Equal | notEqual | lessThan | LessOrEqual | greaterThan | greaterThanOrEqual

*/

// clang-format on

namespace scarlet {
namespace scar {

enum class instruction_type {
  RETURN,
  UNARY,
  BINARY,
  COPY,
  JUMP,
  JUMP_IF_ZERO,
  JUMP_IF_NOT_ZERO,
  LABEL
};
enum class val_type { UNKNOWN, CONSTANT, VAR };

std::string to_string(instruction_type type);
std::string to_string(val_type type);

class scar_Identifier_Node {
private:
  std::string value;

public:
  std::string get_scar_name() { return "Identifier"; }
  std::string get_value() { return value; }
  void set_value(std::string value) { this->value = std::move(value); }
};

// NOTE: DO NOT CONFUSE SCAR REGISTERS WITH ACTUAL REGISTERS
// SCAR REGISTERS ARE JUST PSEUDO REGISTERS(VARIABLES), WHICH WILL BE REPLACED
// BY FIRST STACK OPERANDS AND THEN REGISTERS IN THE INSTRUCTION FIXING PHASE
// SCAR REGISTERS ARE JUST A WAY TO KEEP TRACK OF THE VALUES IN THE SCAR IR
class scar_Val_Node {
private:
  val_type type;
  std::string val;
  std::string reg_name;

public:
  std::string get_scar_name() { return "Val"; }
  val_type get_type() { return type; }
  void set_type(val_type type) { this->type = std::move(type); }
  std::string get_value() { return val; }
  void set_value(std::string value) { this->val = std::move(value); }
  std::string get_reg() { return reg_name; }
  void set_reg_name(std::string identifier) {
    this->reg_name = std::move(identifier);
  }
};

class scar_Instruction_Node {
private:
  instruction_type type; // Return, Unary, Binary
  unop::UNOP unop;       // When the instruction is a unary instruction
  binop::BINOP binop;    // When the instruction is a binary instruction
  std::shared_ptr<scar_Val_Node> src1;
  std::shared_ptr<scar_Val_Node> src2;
  std::shared_ptr<scar_Val_Node> dst;

public:
  std::string get_scar_name() { return "Instruction"; }
  instruction_type get_type() { return type; }
  void set_type(instruction_type type) { this->type = std::move(type); }
  unop::UNOP get_unop() { return unop; }
  void set_unop(unop::UNOP unop) { this->unop = unop; }
  binop::BINOP get_binop() { return binop; }
  void set_binop(binop::BINOP binop) { this->binop = binop; }
  std::shared_ptr<scar_Val_Node> get_src1() { return src1; }
  void set_src1(std::shared_ptr<scar_Val_Node> src1) {
    this->src1 = std::move(src1);
  }
  std::shared_ptr<scar_Val_Node> get_src2() { return src2; }
  void set_src2(std::shared_ptr<scar_Val_Node> src2) {
    this->src2 = std::move(src2);
  }
  std::shared_ptr<scar_Val_Node> get_dst() { return dst; }
  void set_dst(std::shared_ptr<scar_Val_Node> dst) {
    this->dst = std::move(dst);
  }
};

class scar_Function_Node {
private:
  ;
  std::shared_ptr<scar_Identifier_Node> identifier;
  std::vector<std::shared_ptr<scar_Instruction_Node>> body;

public:
  scar_Function_Node() { body.reserve(2); }
  std::string get_scar_name() { return "Function"; }
  std::shared_ptr<scar_Identifier_Node> get_identifier() { return identifier; }
  void set_identifier(std::shared_ptr<scar_Identifier_Node> identifier) {
    this->identifier = std::move(identifier);
  }

  void add_instruction(std::shared_ptr<scar_Instruction_Node> instruction) {
    body.emplace_back(std::move(instruction));
  }
  std::vector<std::shared_ptr<scar_Instruction_Node>> &get_instructions() {
    return body;
  }
};

class scar_Program_Node {
private:
  std::vector<std::shared_ptr<scar_Function_Node>> functions;

public:
  scar_Program_Node() { functions.reserve(2); }
  std::string get_scar_name() { return "Program"; }
  std::vector<std::shared_ptr<scar_Function_Node>> &get_functions() {
    return functions;
  }
  void add_function(std::shared_ptr<scar_Function_Node> function) {
    functions.emplace_back(std::move(function));
  }
};

} // namespace scar
} // namespace scarlet
