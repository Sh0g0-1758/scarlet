// ##############################
// ## Assembly AST for scarlet ##
// ##############################

#pragma once

#include <binary_operations/binop.hh>
#include <memory>
#include <string>
#include <unary_operations/unop.hh>
#include <vector>

// clang-format off

/*

Grammar:

program = Program(function_definition)
function_definition = Function(identifier, instruction* body)
instruction = Mov(Operand src, Operand dst) | Binary(binary_operator, Operand src, Operand dst) | Idiv(Operand src) | Cdq | Ret | Unary(Unary_operator, Operand src/dst) | AllocateStack(Operand)
unary_operator = Neg | Not
binary_operator = Add | Sub | Mul | And | Or | Xor | LeftShift | RightShift
Operand = Imm(int) | Reg(reg) | Pseudo(Identifier) | stack(identifier)
reg = AX | DX | R10 | R11 | CX | CL

*/

// clang-format on
namespace scarlet {
namespace scasm {

enum class operand_type { UNKNOWN, IMM, REG, PSEUDO, STACK };
enum class Unop { UNKNOWN, NEG, NOT };
enum class Binop {
  UNKNOWN,
  ADD,
  SUB,
  MUL,
  AND,
  OR,
  XOR,
  LEFT_SHIFT,
  RIGHT_SHIFT
};
enum class instruction_type {
  UNKNOWN,
  MOV,
  BINARY,
  IDIV,
  CDQ,
  RET,
  UNARY,
  ALLOCATE_STACK
};
enum class register_type { UNKNOWN, AX, DX, R10, R11, CX, CL };

Unop scar_unop_to_scasm_unop(unop::UNOP unop);
Binop scar_binop_to_scasm_binop(binop::BINOP binop);
std::string to_string(register_type reg);
std::string to_string(Unop unop);
std::string to_string(Binop binop);

class scasm_operand {
private:
  operand_type type;
  int imm;
  register_type reg;
  std::string identifier_stack;

public:
  std::string get_scasm_name() { return "Operand"; }
  operand_type get_type() { return type; }
  void set_type(operand_type type) { this->type = type; }
  int get_imm() { return imm; }
  void set_imm(int imm) { this->imm = imm; }
  register_type get_reg() { return reg; }
  void set_reg(register_type reg) { this->reg = reg; }
  std::string get_identifier_stack() { return identifier_stack; }
  void set_identifier_stack(std::string identifier_stack) {
    this->identifier_stack = identifier_stack;
  }
};

class scasm_instruction {
private:
  instruction_type type;
  Unop unop;
  Binop binop;
  std::shared_ptr<scasm_operand> src;
  std::shared_ptr<scasm_operand> dst;

public:
  std::string get_scasm_name() { return "Instruction"; }
  instruction_type get_type() { return type; }
  void set_type(instruction_type type) { this->type = type; }
  Unop get_unop() { return unop; }
  void set_unop(unop::UNOP op) { this->unop = scar_unop_to_scasm_unop(op); }
  Binop get_binop() { return binop; }
  void set_binop(Binop op) { this->binop = op; }
  std::shared_ptr<scasm_operand> get_src() { return src; }
  void set_src(std::shared_ptr<scasm_operand> src) {
    this->src = std::move(src);
  }
  std::shared_ptr<scasm_operand> get_dst() { return dst; }
  void set_dst(std::shared_ptr<scasm_operand> dst) {
    this->dst = std::move(dst);
  }
};

class scasm_function {
private:
  std::string name;
  std::vector<std::shared_ptr<scasm_instruction>> body;

public:
  std::string get_scasm_name() { return "Function"; }
  std::string get_name() { return name; }
  void set_name(std::string name) { this->name = std::move(name); }
  std::vector<std::shared_ptr<scasm_instruction>> &get_instructions() {
    return body;
  }
  void add_instruction(std::shared_ptr<scasm_instruction> instruction) {
    body.emplace_back(std::move(instruction));
  }
};

class scasm_program {
private:
  std::vector<scasm_function> functions;

public:
  std::string get_scasm_name() { return "Program"; }
  std::vector<scasm_function> &get_functions() { return functions; }
  void add_function(scasm_function function) {
    functions.emplace_back(std::move(function));
  }
};

} // namespace scasm
} // namespace scarlet
