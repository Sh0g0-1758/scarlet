// ##############################
// ## Assembly AST for scarlet ##
// ##############################

#pragma once

#include <string>
#include <unary_operations/unop.hh>
#include <vector>

// clang-format off

/*

Grammar:

program = Program(function_definition)
function_definition = Function(identifier, instruction* body)
instruction = Mov(Operand src, Operand dst) | Ret | Unary(Unary_operator, operand) | AllocateStack(Operand)
unary_operator = Neg | Not
Operand = Imm(int) | Reg(reg) | Pseudo(Identifier) | stack(int)

*/

// clang-format on

class scasm {
public:
  virtual std::string get_scasm_name() { return "scasm"; }
  virtual ~scasm() = default;
};

class scasm_operand : public scasm {
private:
  std::string type;
  std::string value;

public:
  std::string get_scasm_name() override { return "Operand"; }
  std::string get_type() { return type; }
  void set_type(std::string type) { this->type = std::move(type); }
  std::string get_value() { return value; }
  void set_value(std::string value) { this->value = std::move(value); }
};

class scasm_instruction : public scasm {
private:
  std::string type;
  UNOP op;
  scasm_operand src;
  scasm_operand dst;

public:
  std::string get_scasm_name() override { return "Instruction"; }
  std::string get_type() { return type; }
  void set_type(std::string type) { this->type = std::move(type); }
  UNOP get_op() { return op; }
  void set_op(UNOP op) { this->op = op; }
  scasm_operand &get_src() { return src; }
  void set_src(scasm_operand src) { this->src = std::move(src); }
  scasm_operand &get_dst() { return dst; }
  void set_dst(scasm_operand dst) { this->dst = std::move(dst); }
};

class scasm_function : public scasm {
private:
  std::string name;
  std::vector<scasm_instruction> body;

public:
  std::string get_scasm_name() override { return "Function"; }
  std::string get_name() { return name; }
  void set_name(std::string name) { this->name = std::move(name); }
  std::vector<scasm_instruction> &get_instructions() { return body; }
  void add_instruction(scasm_instruction instruction) {
    body.emplace_back(std::move(instruction));
  }
};

class scasm_program : public scasm {
private:
  std::vector<scasm_function> functions;

public:
  std::string get_scasm_name() override { return "Program"; }
  std::vector<scasm_function> &get_functions() { return functions; }
  void add_function(scasm_function function) {
    functions.emplace_back(std::move(function));
  }
};
