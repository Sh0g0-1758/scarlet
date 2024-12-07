#pragma once

#include <ast/ast.hh>
#include <fstream>
#include <token/token.hh>
#include <vector>

enum class Regiser { EAX };

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
epilogue are used to set up and tear down the stack frame. Prologue:   pushq
%rbp          # Stores the value of the current base pointer on the stack movq
%rsp, %rbp     # Sets the base pointer to the current stack pointer subq $n,
%rsp       # Decrement the stack pointer n * 8 bytes. This is where local
variables are stored Epilogue:   movq %rbp, %rsp     # Restores the stack
pointer to the base pointer popq %rbp           # Restores the base pointer to
the original base pointer, thus making the earlier stack frame usable

*/

class Codegen {
private:
  AST_Program_Node program;
  std::string file_name;
  bool success = true;

public:
  Codegen(AST_Program_Node program) : program(program) {}
  void codegen();
  void set_file_name(std::string file_name) { this->file_name = file_name; }
  bool is_success() { return success; }
};