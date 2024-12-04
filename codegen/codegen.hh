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

*/

/*

Notes about X86-64 Assembly:

1. Non Executable stack: .section	.note.GNU-stack,"",@progbits

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