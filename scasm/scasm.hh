// ##############################
// ## Assembly AST for scarlet ##
// ##############################

#pragma once

#include <binary_operations/binop.hh>
#include <iostream>
#include <memory>
#include <string>
#include <tools/macros/macros.hh>
#include <unary_operations/unop.hh>
#include <vector>

// clang-format off

/*

Grammar:

program = Program(top_level*)

top_level = Function(identifier name, bool global, instruction* instructions) | StaticVariable(identifier name, bool global, int init)

instruction = Mov(Operand src, Operand dst) | Binary(binary_operator, Operand src, Operand dst) | Idiv(Operand src) | Cdq | Ret | Unary(Unary_operator, Operand src/dst) | AllocateStack(Operand) | Cmp(Operand, Operand) | Jmp(Identifier) | JmpCC(cond_code, label) | SetCC(cond_code, operand) | Label(label) | Push(Operand) | Call(Identifier) | DeallocateStack(int)

unary_operator = Neg | Not

binary_operator = Add | Sub | Mul | And | Or | Xor | LeftShift | RightShift

Operand = Imm(int) | Reg(reg) | Pseudo(Identifier) | stack(identifier) | Label(identifier) | Data(Identifier)

cond_code = E | NE | G | GE | L | LE

reg = AX | CX | DX | DI | SI | R8 | R9 | R10 | R11 | CL

*/

// clang-format on
namespace scarlet {
namespace scasm {

// NOTE: Every Pseudo Operand gets converted into a stack operand
enum class operand_type { UNKNOWN, IMM, REG, PSEUDO, STACK, LABEL, COND, DATA };
enum class Unop { UNKNOWN, NEG, ANOT, LNOT };
enum class Binop {
  UNKNOWN,
  ADD,
  SUB,
  MUL,
  AAND,
  AOR,
  XOR,
  LEFT_SHIFT,
  RIGHT_SHIFT,
  LAND,
  LOR,
  EQUAL,
  NOTEQUAL,
  LESSTHAN,
  GREATERTHAN,
  LESSTHANEQUAL,
  GREATERTHANEQUAL,
  ASSIGN
};

enum class instruction_type {
  UNKNOWN,
  MOV,
  BINARY,
  IDIV,
  CDQ,
  RET,
  UNARY,
  ALLOCATE_STACK,
  DEALLOCATE_STACK,
  CMP,
  JMP,
  JMPCC,
  SETCC,
  LABEL,
  PUSH,
  CALL,
};

enum class register_type { UNKNOWN, AX, CX, DX, DI, SI, R8, R9, R10, R11, CL };
// Word = 16 bits, Dword = 32 bits, Qword = 64 bits
enum class register_size { BYTE, DWORD, QWORD };

enum class cond_code { UNKNOWN, E, NE, G, GE, L, LE };

Unop scar_unop_to_scasm_unop(unop::UNOP unop);
Binop scar_binop_to_scasm_binop(binop::BINOP binop);
std::string to_string(register_type reg, register_size size);
std::string to_string(Unop unop);
std::string to_string(Binop binop);
std::string to_string(cond_code cond);

class scasm_operand {
private:
  operand_type type;
  int imm;
  register_type reg;
  cond_code cond;
  std::string identifier_stack;

public:
  std::string get_scasm_name() { return "Operand"; }
  operand_type get_type() { return type; }
  void set_type(operand_type type) { this->type = type; }
  int get_imm() { return imm; }
  void set_imm(int imm) { this->imm = imm; }
  register_type get_reg() { return reg; }
  void set_reg(register_type reg) { this->reg = reg; }
  cond_code get_cond() { return cond; }
  void set_cond(cond_code cond) { this->cond = cond; }
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

enum class scasm_top_level_type { FUNCTION, STATIC_VARIABLE };

class scasm_top_level {
private:
  bool global;
  scasm_top_level_type type;

public:
  std::string get_scasm_name() { return "TopLevel"; }
  bool is_global() { return global; }
  void set_global(bool global) { this->global = global; }
  scasm_top_level_type get_type() { return type; }
  void set_type(scasm_top_level_type type) { this->type = type; }
};

class scasm_function : public scasm_top_level {
private:
  std::string name;
  std::vector<std::shared_ptr<scasm_instruction>> body;
  int frameSize = 0;

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
  int get_frame_size() { return frameSize; }
  void set_frame_size(int frameSize) { this->frameSize = frameSize; }
};

class scasm_static_variable : public scasm_top_level {
private:
  std::string name;
  int init;

public:
  std::string get_scasm_name() { return "StaticVariable"; }
  std::string get_name() { return name; }
  void set_name(std::string name) { this->name = std::move(name); }
  int get_init() { return init; }
  void set_init(int init) { this->init = init; }
};

class scasm_program {
private:
  std::vector<std::shared_ptr<scasm_top_level>> elems;

public:
  std::string get_scasm_name() { return "Program"; }
  std::vector<std::shared_ptr<scasm_top_level>> &get_elems() { return elems; }
  void add_elem(std::shared_ptr<scasm_top_level> elem) {
    elems.emplace_back(std::move(elem));
  }
};

} // namespace scasm
} // namespace scarlet
