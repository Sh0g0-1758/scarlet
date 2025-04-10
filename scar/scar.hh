// ####################
// ## IR for scarlet ##
// ####################

#pragma once

#include "binary_operations/binop.hh"
#include "unary_operations/unop.hh"
#include <iostream>
#include <memory>
#include <string>
#include <tools/constant/constant.hh>
#include <tools/macros/macros.hh>
#include <vector>

// clang-format off

/*

Grammar:

program = Program(top_level*)

top_level = Function(identifier, bool global, identifier* params, instruction* body) 
          | StaticVariable(identifier, bool global, const* init)

instruction = Return(val) 
            | Unary(unary_operator, val src, val dst) 
            | Binary(binary_operator, val src1, val src2, val dst) 
            | Copy(val src, val dst) 
            | Jump(identifier target) 
            | JumpIfZero(val condition, identifier target) 
            | JumpIfNotZero(val condition, identifier target) 
            | Label(Identifier) 
            | FunCall(identifier name, val* args, val dst) 
            | SignExtend(val src, val dst) 
            | Truncate(val src, val dst) 
            | ZeroExtend(val src, val dst) 
            | DoubleToInt(val src, val dst) 
            | DoubleToUint(val src, val dst) 
            | IntToDouble(val src, val dst) 
            | UintToDouble(val src, val dst) 
            | GetAddress(val src , val dst) 
            | Load(val src_ptr, val dst) 
            | Store(val src, val dst_ptr)
            | AddPtr(val src, val index, int scale, val dst)
            | CopyToOffset(val src, identifier dst, int offset)

val = Constant(const) | Var(identifier)

unary_operator = Complement | Negate | Not

binary_operator = Add 
                | Subtract 
                | Multiply 
                | Divide 
                | Remainder 
                | And 
                | Or 
                | Xor 
                | leftShift 
                | rightShift 
                | Equal 
                | notEqual
                | lessThan 
                | LessOrEqual 
                | greaterThan 
                | greaterThanOrEqual

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
  LABEL,
  CALL,
  SIGN_EXTEND,
  TRUNCATE,
  ZERO_EXTEND,
  DOUBLE_TO_INT,
  DOUBLE_TO_UINT,
  INT_TO_DOUBLE,
  UINT_TO_DOUBLE,
  LOAD,
  STORE,
  GET_ADDRESS,
  ADD_PTR,
  COPY_TO_OFFSET
};
// CONSTANT is a constant value (1, 2 , 42 etc.)
// VAR is a scar register
// LABEL is label which can be jumped to
enum class val_type { LABEL, CONSTANT, VAR };

std::string to_string(instruction_type type);
std::string to_string(val_type type);
bool is_type_cast(instruction_type type);

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
// HERE VAL IS FOR A CONSTANT VALUE AND REG_NAME IS FOR USER DEFINED IDENTIFIERS
// AND TEMPORARY REGISTERS GENERATED INTERNALLY BY SCAR
class scar_Val_Node {
private:
  val_type type;
  constant::Constant const_val;
  std::string reg_name;
  std::string label;

public:
  std::string get_scar_name() { return "Val"; }
  val_type get_type() { return type; }
  void set_type(val_type type) { this->type = std::move(type); }
  constant::Constant get_const_val() { return const_val; }
  void set_const_val(constant::Constant const_val) {
    this->const_val = const_val;
  }
  std::string get_reg() { return reg_name; }
  void set_reg_name(std::string identifier) {
    this->reg_name = std::move(identifier);
  }
  std::string get_label() { return label; }
  void set_label(std::string label) { this->label = std::move(label); }
};

class scar_Instruction_Node {
private:
  instruction_type type;
  unop::UNOP unop;
  binop::BINOP binop;
  std::shared_ptr<scar_Val_Node> src1;
  std::shared_ptr<scar_Val_Node> src2;
  std::shared_ptr<scar_Val_Node> dst;
  /*
   * For arrays, this offset is used when we want to copy the value of array
   element into its correct location
   * For pointers, offset is used during pointer arithmetic. So consider if we
   have {pointer + int}, then we store the size of the referenced type of the
   pointer in this variable.
   */
  long offset;

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
  long get_offset() { return offset; }
  void set_offset(long offset) { this->offset = offset; }
};

class scar_FunctionCall_Instruction_Node : public scar_Instruction_Node {
private:
  std::shared_ptr<scar_Identifier_Node> name;
  std::vector<std::shared_ptr<scar_Val_Node>> args;

public:
  std::string get_scar_name() { return "FunctionCall"; }
  std::shared_ptr<scar_Identifier_Node> get_name() { return name; }
  void set_name(std::shared_ptr<scar_Identifier_Node> name) {
    this->name = std::move(name);
  }
  std::vector<std::shared_ptr<scar_Val_Node>> &get_args() { return args; }
  void add_arg(std::shared_ptr<scar_Val_Node> arg) {
    args.emplace_back(std::move(arg));
  }
};

enum class topLevelType { FUNCTION, STATICVARIABLE };

class scar_Top_Level_Node {
private:
  bool global = true;
  topLevelType type;

public:
  std::string get_scar_name() { return "TopLevel"; }
  bool is_global() { return global; }
  void set_global(bool global) { this->global = global; }
  topLevelType get_type() { return type; }
  void set_type(topLevelType type) { this->type = type; }
};

class scar_Function_Node : public scar_Top_Level_Node {
private:
  std::vector<std::shared_ptr<scar_Identifier_Node>> params;
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

  std::vector<std::shared_ptr<scar_Identifier_Node>> &get_params() {
    return params;
  }
  void add_param(std::shared_ptr<scar_Identifier_Node> param) {
    params.emplace_back(std::move(param));
  }
};

class scar_StaticVariable_Node : public scar_Top_Level_Node {
private:
  std::shared_ptr<scar_Identifier_Node> identifier;
  std::vector<constant::Constant> init;

public:
  std::string get_scar_name() { return "StaticVariable"; }
  std::shared_ptr<scar_Identifier_Node> get_identifier() { return identifier; }
  void set_identifier(std::shared_ptr<scar_Identifier_Node> identifier) {
    this->identifier = std::move(identifier);
  }
  std::vector<constant::Constant> &get_init() { return init; }
  void add_init(constant::Constant init) {
    this->init.emplace_back(std::move(init));
  }
  void set_init(std::vector<constant::Constant> init) {
    this->init = std::move(init);
  }
};

class scar_Program_Node {
private:
  std::vector<std::shared_ptr<scar_Top_Level_Node>> elems;

public:
  scar_Program_Node() { elems.reserve(4); }
  std::string get_scar_name() { return "Program"; }
  std::vector<std::shared_ptr<scar_Top_Level_Node>> &get_elems() {
    return elems;
  }
  void add_elem(std::shared_ptr<scar_Top_Level_Node> elem) {
    elems.emplace_back(std::move(elem));
  }
};

} // namespace scar
} // namespace scarlet
