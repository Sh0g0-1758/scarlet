#pragma once

#include <ast/ast.hh>
#include <fstream>
#include <map>
#include <scar/scar.hh>
#include <scasm/scasm.hh>
#include <sstream>
#include <stack>
#include <string>
#include <tools/constant/constant.hh>
#include <tools/macros/macros.hh>
#include <tools/symbolTable/symbolTable.hh>
#include <vector>

// clang-format off
/*

Notes about X86-64 Assembly:

1.  Non Executable stack: .section	.note.GNU-stack,"",@progbits
2.  stack frames are just the space between the stack pointer and the base
pointer. The stack grows towards lower memory addresses. The prologue and the
epilogue are used to set up and tear down the stack frame.
Prologue:   pushq %rbp          # Stores the value of the current base pointer on the stack
            movq  %rsp, %rbp    # Sets the base pointer to the current stack pointer
            subq  $n, %rsp      # Decrement the stack pointer n * 8 bytes. This is where local variables are stored
Epilogue:   movq  %rbp, %rsp    # Restores the stack pointer to the base pointer
            popq  %rbp          # Restores the base pointer to the original base pointer, thus making the earlier stack frame usable
3. binary operations take as input: op, src and dst where:
    3.a op is an instruction
    3.b src is an immediate value, register or memory address
    3.c dst is a register or memory address
4. assembly instructions take l suffix if their operands are 32 bits and q suffix if their operands are 64 bits. 

*/
// clang-format on
namespace scarlet {
namespace codegen {

class Codegen {
private:
  ast::AST_Program_Node program;
  scar::scar_Program_Node scar;
  scasm::scasm_program scasm;
  std::string file_name;
  int curr_buff = 0;
  std::vector<std::vector<unop::UNOP>> unop_buffer;
  constant::Constant constant_buffer;
  std::string variable_buffer;
  bool success = true;
  int curr_regNum;
  std::string reg_name;
  std::map<std::string, std::string> pseduo_registers;
  std::map<std::string, symbolTable::symbolInfo> globalSymbolTable;
  void gen_scar_exp(std::shared_ptr<ast::AST_exp_Node> exp,
                    std::shared_ptr<scar::scar_Function_Node> scar_function);
  void
  gen_scar_assign_exp(std::shared_ptr<ast::AST_exp_Node> exp,
                      std::shared_ptr<scar::scar_Function_Node> scar_function);
  void gen_scar_short_circuit_exp(
      std::shared_ptr<ast::AST_exp_Node> exp,
      std::shared_ptr<scar::scar_Function_Node> scar_function);
  void
  gen_scar_ternary_exp(std::shared_ptr<ast::AST_exp_Node> exp,
                       std::shared_ptr<scar::scar_Function_Node> scar_function);
  void gen_scar_factor(std::shared_ptr<ast::AST_factor_Node> factor,
                       std::shared_ptr<scar::scar_Function_Node> scar_function);
  void gen_scar_factor_function_call(
      std::shared_ptr<ast::AST_factor_function_call_Node> factor,
      std::shared_ptr<scar::scar_Function_Node> scar_function);
  void
  gen_scar_statement(std::shared_ptr<ast::AST_Statement_Node> statement,
                     std::shared_ptr<scar::scar_Function_Node> scar_function);
  void
  gen_scar_declaration(std::shared_ptr<ast::AST_Declaration_Node> declaration,
                       std::shared_ptr<scar::scar_Function_Node> scar_function);
  void gen_scar_block(std::shared_ptr<ast::AST_Block_Node> block,
                      std::shared_ptr<scar::scar_Function_Node> scar_function);
  int fr_label_counter = 1;
  int res_label_counter = 1;
  std::stack<std::string> fr_label_stack;
  std::stack<std::string> res_label_stack;
  void
  pretty_print_function(std::shared_ptr<scar::scar_Function_Node> function);
  void pretty_print_static_variable(
      std::shared_ptr<scar::scar_StaticVariable_Node> static_variable);

public:
  Codegen(ast::AST_Program_Node program, int counter,
          std::map<std::string, symbolTable::symbolInfo> gst)
      : program(program), curr_regNum(counter), globalSymbolTable(gst) {
    unop_buffer.resize(2);
  }
  // ###### COMPILER PASSES ######
  // IR PASS
  void gen_scar();
  // ASM PASS
  void gen_scasm();
  // STACK ALLOCATION PASS
  void fix_pseudo_registers();
  // FIXING INSTRUCTIONS PASS
  void fix_instructions();
  // CODEGEN PASS
  void codegen();
  // #############################
  void set_file_name(std::string file_name) { this->file_name = file_name; }
  bool is_success() { return success; }
  std::string get_reg_name() {
    reg_name = "temp." + std::to_string(curr_regNum);
    curr_regNum++;
    return reg_name;
  }
  std::string get_prev_reg_name() { return reg_name; }
  void pretty_print();
  std::string get_fr_label_name() {
    std::string label_name = "label." + std::to_string(fr_label_counter);
    fr_label_counter++;
    fr_label_stack.push(label_name);
    return label_name;
  }
  std::string get_last_fr_label_name(bool pop = false) {
    std::string tmp = fr_label_stack.top();
    if (pop)
      fr_label_stack.pop();
    return tmp;
  }
  std::string get_res_label_name() {
    std::string label_name = "labelRes." + std::to_string(res_label_counter);
    res_label_counter++;
    res_label_stack.push(label_name);
    return label_name;
  }
  std::string get_last_res_label_name() {
    std::string tmp = res_label_stack.top();
    res_label_stack.pop();
    return tmp;
  }

  std::map<binop::BINOP, binop::BINOP> compound_to_base = {
      {binop::BINOP::COMPOUND_DIFFERENCE, binop::BINOP::SUB},
      {binop::BINOP::COMPOUND_DIVISION, binop::BINOP::DIV},
      {binop::BINOP::COMPOUND_PRODUCT, binop::BINOP::MUL},
      {binop::BINOP::COMPOUND_REMAINDER, binop::BINOP::MOD},
      {binop::BINOP::COMPOUND_SUM, binop::BINOP::ADD},
      {binop::BINOP::COMPOUND_AND, binop::BINOP::AAND},
      {binop::BINOP::COMPOUND_OR, binop::BINOP::AOR},
      {binop::BINOP::COMPOUND_XOR, binop::BINOP::XOR},
      {binop::BINOP::COMPOUND_LEFTSHIFT, binop::BINOP::LEFT_SHIFT},
      {binop::BINOP::COMPOUND_RIGHTSHIFT, binop::BINOP::RIGHT_SHIFT}};

  std::vector<scasm::register_type> argReg = {
      scasm::register_type::DI, scasm::register_type::SI,
      scasm::register_type::DX, scasm::register_type::CX,
      scasm::register_type::R8, scasm::register_type::R9};
};

} // namespace codegen
} // namespace scarlet
