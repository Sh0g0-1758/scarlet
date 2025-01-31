#pragma once

#include <ast/ast.hh>
#include <fstream>
#include <map>
#include <scar/scar.hh>
#include <scasm/scasm.hh>
#include <sstream>
#include <string>
#include <vector>

// clang-format off
/*

Grammar:

<program> ::= function_definition
<function_definition> ::= <identifier> <instruction>+
<instruction> = "movl" <operand> "," <operand> | "ret"
<operand> ::= <imm> | <register>
<imm> ::= "$" <number>
<register> ::= "%eax"
<identifier> ::= ? An identifier token ?

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
  std::string constant_buffer;
  bool success = true;
  int curr_regNum = 1;
  std::map<std::string, std::string> pseduo_registers;
  int stack_offset{};
  void gen_scar_exp(std::shared_ptr<ast::AST_exp_Node> exp,
                    scar::scar_Function_Node &scar_function);
  void gen_scar_factor(std::shared_ptr<ast::AST_factor_Node> factor,
                       scar::scar_Function_Node &scar_function);

public:
  Codegen(ast::AST_Program_Node program) : program(program) {
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
    std::string reg_name = "temp." + std::to_string(curr_regNum);
    curr_regNum++;
    return reg_name;
  }
  std::string get_prev_reg_name() {
    std::string reg_name = "temp." + std::to_string(curr_regNum - 1);
    return reg_name;
  }
  void pretty_print();
};

} // namespace codegen
} // namespace scarlet
