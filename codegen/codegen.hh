#pragma once

#include <ast/ast.hh>
#include <fstream>
#include <map>
#include <scar/scar.hh>
#include <scasm/scasm.hh>
#include <sstream>
#include <stack>
#include <string>
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

#define NOTNULL(x) x != nullptr

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
  int curr_regNum;
  std::map<std::string, std::string> pseduo_registers;
  int stack_offset{};
  void gen_scar_exp(std::shared_ptr<ast::AST_exp_Node> exp,
                    std::shared_ptr<scar::scar_Function_Node> scar_function);
  void gen_scar_factor(std::shared_ptr<ast::AST_factor_Node> factor,
                       std::shared_ptr<scar::scar_Function_Node> scar_function);
  int fr_label_counter = 1;
  int res_label_counter = 1;
  std::stack<std::string> fr_label_stack;
  std::stack<std::string> res_label_stack;

public:
  Codegen(ast::AST_Program_Node program, int counter)
      : program(program), curr_regNum(counter) {
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
  std::string get_fr_label_name() {
    std::string label_name = "label" + std::to_string(fr_label_counter);
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
    std::string label_name = "labelRes" + std::to_string(res_label_counter);
    res_label_counter++;
    res_label_stack.push(label_name);
    return label_name;
  }
  std::string get_last_res_label_name() {
    std::string tmp = res_label_stack.top();
    res_label_stack.pop();
    return tmp;
  }
};

} // namespace codegen
} // namespace scarlet
