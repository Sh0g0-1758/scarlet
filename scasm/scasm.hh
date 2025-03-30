// ##############################
// ## Assembly AST for scarlet ##
// ##############################

#pragma once

#include <binary_operations/binop.hh>
#include <iostream>
#include <memory>
#include <string>
#include <tools/constant/constant.hh>
#include <tools/macros/macros.hh>
#include <unary_operations/unop.hh>
#include <vector>

// clang-format off

/*

Grammar:

program = Program(top_level*)

assembly_type = LongWord 
              | QuadWord
              | Double

top_level = Function(identifier name, bool global, instruction* instructions) 
          | StaticVariable(identifier name, bool global, int alignment, const init)

instruction = Mov(assembly_type, Operand src, Operand dst)
            | Movsx(Operand src, Operand dst)
            | MovZeroExtend(Operand src, Operand dst)
            | Binary(binary_operator, assembly_type,  Operand src, Operand dst)
            | Idiv(assembly_type, Operand src)
            | Div(assembly_type, Operand src)
            | Cdq(assembly_type)
            | Ret
            | Unary(Unary_operator, assembly_type, Operand src/dst)
            | Cmp(assembly_type, Operand, Operand)
            | Jmp(Identifier)
            | JmpCC(cond_code, label)
            | SetCC(cond_code, operand)
            | Label(label)
            | Push(Operand)
            | Call(Identifier)
            | Cvtts2di(assembly_type, Operand src, Operand dst)
            | Cvtsi2sd(assembly_type, Operand src, Operand dst)

unary_operator = Neg 
               | Not
               | Shr

binary_operator = Add 
                | Sub 
                | Mul 
                | And 
                | Or 
                | Xor 
                | LeftShift 
                | RightShift
                | DivDouble

Operand = Imm(int) 
        | Reg(reg) 
        | Pseudo(Identifier) 
        | stack(identifier) 
        | Label(identifier) 
        | Data(Identifier)

cond_code = E | NE | G | GE | L | LE

reg = AX
    | CX 
    | DX 
    | DI 
    | SI 
    | R8 
    | R9 
    | R10 
    | R11 
    | CL 
    | SP 
    | B 
    | BE 
    | A 
    | AE
    | XMM0
    | XMM1
    | XMM2
    | XMM3
    | XMM4
    | XMM5
    | XMM6
    | XMM7
    | XMM8
    | XMM9
    | XMM10
    | XMM11
    | XMM12
    | XMM13
    | XMM14
    | XMM15

*/

// clang-format on
namespace scarlet {
namespace scasm {

// BYTE = 8 bits, LONG WORD = 32 bits, QUAD WORD = 64 bits
enum class AssemblyType { NONE, BYTE, LONG_WORD, QUAD_WORD, DOUBLE };
// NOTE: Every Pseudo Operand gets converted into a stack operand
enum class operand_type { UNKNOWN, IMM, REG, PSEUDO, STACK, LABEL, COND, DATA };
enum class Unop { UNKNOWN, NEG, ANOT, LNOT, SHR };
enum class Binop {
  UNKNOWN,
  ADD,
  SUB,
  MUL,
  /* division for double emits the same instructions as integer mul|add|sub */
  DIV_DOUBLE,
  AAND,
  AOR,
  XOR,
  LEFT_SHIFT,
  RIGHT_SHIFT,
  LOGICAL_LEFT_SHIFT,
  LOGICAL_RIGHT_SHIFT,
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
  MOVSX,
  MOVZX,
  BINARY,
  IDIV,
  DIV,
  CDQ,
  RET,
  UNARY,
  CMP,
  JMP,
  JMPCC,
  SETCC,
  LABEL,
  PUSH,
  CALL,
  CVTTS2DI,
  CVTSI2SD,
};

enum class register_type {
  UNKNOWN,
  AX,
  CX,
  DX,
  DI,
  SI,
  R8,
  R9,
  R10,
  R11,
  CL,
  SP,
  XMM0,
  XMM1,
  XMM2,
  XMM3,
  XMM4,
  XMM5,
  XMM6,
  XMM7,
  XMM14,
  XMM15
};
// Byte = 8 bits, Word = 16 bits, Lword = 32 bits, Qword = 64 bits
enum class register_size { BYTE, LWORD, QWORD };

enum class cond_code { UNKNOWN, E, NE, G, GE, L, LE, B, BE, A, AE };

Unop scar_unop_to_scasm_unop(unop::UNOP unop);
Binop scar_binop_to_scasm_binop(binop::BINOP binop);
std::string to_string(register_type reg, register_size size);
std::string to_string(Unop unop);
std::string to_string(Binop binop);
std::string to_string(cond_code cond);

class scasm_operand {
private:
  operand_type type;
  constant::Constant imm;
  register_type reg;
  cond_code cond;
  std::string identifier_stack;

public:
  std::string get_scasm_name() { return "Operand"; }
  operand_type get_type() { return type; }
  void set_type(operand_type type) { this->type = type; }
  constant::Constant get_imm() { return imm; }
  void set_imm(constant::Constant imm) { this->imm = imm; }
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
  AssemblyType asmType;

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
  AssemblyType get_asm_type() { return asmType; }
  void set_asm_type(AssemblyType asmType) { this->asmType = asmType; }
};

enum class scasm_top_level_type { FUNCTION, STATIC_VARIABLE, STATIC_CONSTANT };

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
  constant::Constant init;
  int alignment;

public:
  std::string get_scasm_name() { return "StaticVariable"; }
  std::string get_name() { return name; }
  void set_name(std::string name) { this->name = std::move(name); }
  constant::Constant get_init() { return init; }
  void set_init(constant::Constant init) { this->init = init; }
  int get_alignment() { return alignment; }
  void set_alignment(int alignment) { this->alignment = alignment; }
};

class scasm_static_constant : public scasm_top_level {
private:
  std::string name;
  constant::Constant init;
  int alignment;

public:
  std::string get_scasm_name() { return "StaticConstant"; }
  std::string get_name() { return name; }
  void set_name(std::string name) { this->name = std::move(name); }
  constant::Constant get_init() { return init; }
  void set_init(constant::Constant init) { this->init = init; }
  int get_alignment() { return alignment; }
  void set_alignment(int alignment) { this->alignment = alignment; }
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

enum backendSymbolType { FUNCTION, STATIC_VARIABLE, STATIC_CONSTANT };

struct backendSymbol {
  /* Use for Static Variable and Static Constant */
  backendSymbolType type;
  AssemblyType asmType;
  bool isTopLevel;
  /* Use for Functions */
  bool isDefined;
};

} // namespace scasm
} // namespace scarlet
