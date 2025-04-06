#pragma once

#include <ast/ast.hh>
#include <cmath>
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

// Custom comparator for double values to handle -0.0 and 0.0
struct DoubleCompare {
  bool operator()(double a, double b) const {
    if (a == b) {
      return std::signbit(a) < std::signbit(b); // Distinguishes 0.0 from -0.0
    }
    return a < b;
  }
};

class Codegen {
private:
  ast::AST_Program_Node program;
  scar::scar_Program_Node scar;
  scasm::scasm_program scasm;
  std::string file_name;
  constant::Constant constant_buffer;
  std::string variable_buffer;
  bool success = true;
  int curr_regNum;
  int doubleLabelCounter = 0;
  int doubleCastCounter = 0;
  std::string reg_name;
  std::map<std::string, std::string> pseduo_registers;
  std::map<std::string, symbolTable::symbolInfo> globalSymbolTable;
  std::map<std::string, scasm::backendSymbol> backendSymbolTable;
  std::map<double, std::string, DoubleCompare> doubleLabelMap;
  void gen_scar_exp(std::shared_ptr<ast::AST_exp_Node> exp,
                    std::shared_ptr<scar::scar_Function_Node> scar_function);
  void gen_scar_def_assign_exp(
      std::shared_ptr<ast::AST_exp_Node> exp,
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
  void
  gen_scar_pointer_exp(std::shared_ptr<ast::AST_exp_Node> exp,
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

  void gen_unop_scasm(std::shared_ptr<scar::scar_Instruction_Node> inst,
                      std::shared_ptr<scasm::scasm_function> scasm_func,
                      scasm::scasm_program &scasm_program);
  void gen_binop_scasm(std::shared_ptr<scar::scar_Instruction_Node> inst,
                       std::shared_ptr<scasm::scasm_function> scasm_func,
                       scasm::scasm_program &scasm_program);
  void gen_funcall_scasm(std::shared_ptr<scar::scar_Instruction_Node> inst,
                         std::shared_ptr<scasm::scasm_function> scasm_func,
                         scasm::scasm_program &scasm_program);
  void
  gen_scar_factor_array(std::shared_ptr<ast::AST_factor_Node> factor,
                        std::shared_ptr<scar::scar_Function_Node> scar_function,
                        std::vector<long> derivedType);
  int fr_label_counter = 1;
  int res_label_counter = 1;
  std::stack<std::string> fr_label_stack;
  std::stack<std::string> res_label_stack;
  void
  pretty_print_function(std::shared_ptr<scar::scar_Function_Node> function);
  void pretty_print_static_variable(
      std::shared_ptr<scar::scar_StaticVariable_Node> static_variable);
  void pretty_print_type(ast::ElemType type, std::vector<long> derivedType);

public:
  Codegen(ast::AST_Program_Node program, int counter,
          std::map<std::string, symbolTable::symbolInfo> gst)
      : program(program), curr_regNum(counter), globalSymbolTable(gst) {}
  // ###### COMPILER PASSES ######
  // IR PASS
  void gen_scar();
  // ASM PASS
  void gen_scasm();
  void asm_gen_func(std::shared_ptr<scasm::scasm_top_level> elem,
                    std::stringstream &assembly);
  void asm_gen_static_variable(std::shared_ptr<scasm::scasm_top_level> elem,
                               std::stringstream &assembly);
  void asm_gen_static_constant(std::shared_ptr<scasm::scasm_top_level> elem,
                               std::stringstream &assembly);
  // STACK ALLOCATION PASS
  void fix_pseudo_registers();
  // FIXING INSTRUCTIONS PASS
  void fix_instructions();
  // CODEGEN PASS
  void codegen();
  // #############################
  void set_file_name(std::string file_name) { this->file_name = file_name; }
  bool is_success() { return success; }
  std::string get_reg_name(ast::ElemType type, std::vector<long> derivedType) {
    reg_name = "scarReg." + std::to_string(curr_regNum);
    symbolTable::symbolInfo scarReg;
    scarReg.name = reg_name;
    scarReg.link = symbolTable::linkage::NONE;
    scarReg.type = symbolTable::symbolType::VARIABLE;
    scarReg.typeDef = {type};
    scarReg.derivedTypeMap[0] = derivedType;
    globalSymbolTable[reg_name] = scarReg;
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

  std::string get_const_label_name() {
    return "C." + std::to_string(doubleLabelCounter++);
  }

  scasm::AssemblyType valToAsmType(std::shared_ptr<scar::scar_Val_Node> val) {
    if (val == nullptr) {
      return scasm::AssemblyType::NONE;
    }
    switch (val->get_type()) {
    case scar::val_type::CONSTANT:
      switch (val->get_const_val().get_type()) {
      case constant::Type::INT:
      case constant::Type::UINT:
        return scasm::AssemblyType::LONG_WORD;
      case constant::Type::LONG:
      case constant::Type::ULONG:
        return scasm::AssemblyType::QUAD_WORD;
      case constant::Type::DOUBLE:
        return scasm::AssemblyType::DOUBLE;
      // TODO: FIXME
      case constant::Type::ZERO:
      case constant::Type::NONE:
        return scasm::AssemblyType::NONE;
      }
      break;
    case scar::val_type::VAR:
      switch (globalSymbolTable[val->get_reg()].typeDef[0]) {
      case ast::ElemType::INT:
      case ast::ElemType::UINT:
        return scasm::AssemblyType::LONG_WORD;
      case ast::ElemType::LONG:
      case ast::ElemType::ULONG:
        return scasm::AssemblyType::QUAD_WORD;
      case ast::ElemType::DOUBLE:
        return scasm::AssemblyType::DOUBLE;
      // TODO: FIXME
      case ast::ElemType::DERIVED:
      case ast::ElemType::POINTER:
      case ast::ElemType::NONE:
        return scasm::AssemblyType::NONE;
      }
      break;
    case scar::val_type::LABEL:
      return scasm::AssemblyType::NONE;
    }
    UNREACHABLE();
  }

  scasm::AssemblyType elemToAsmType(ast::ElemType type) {
    switch (type) {
    case ast::ElemType::INT:
      return scasm::AssemblyType::LONG_WORD;
    case ast::ElemType::LONG:
      return scasm::AssemblyType::QUAD_WORD;
    case ast::ElemType::UINT:
      return scasm::AssemblyType::LONG_WORD;
    case ast::ElemType::ULONG:
      return scasm::AssemblyType::QUAD_WORD;
    case ast::ElemType::DOUBLE:
      return scasm::AssemblyType::DOUBLE;
    // TODO: FIXME
    case ast::ElemType::DERIVED:
    case ast::ElemType::POINTER:
    case ast::ElemType::NONE:
      return scasm::AssemblyType::NONE;
    }
    UNREACHABLE();
  }

  constant::Type valToConstType(std::shared_ptr<scar::scar_Val_Node> val) {
    switch (val->get_type()) {
    case scar::val_type::CONSTANT:
      return val->get_const_val().get_type();
    case scar::val_type::VAR:
      return ast::elemTypeToConstType(
          globalSymbolTable[val->get_reg()].typeDef[0]);
    case scar::val_type::LABEL:
      return constant::Type::NONE;
    }
    UNREACHABLE();
  }

  void calssify_parameters(
      std::vector<constant::Type> &param_types,
      std::vector<std::pair<scasm::AssemblyType, int>> &int_param_indx,
      std::vector<int> &double_param_indx,
      std::vector<std::pair<scasm::AssemblyType, int>> &stack_param_indx) {
    int int_reg_count = 0;
    int double_reg_count = 0;
    for (int i = 0; i < (int)param_types.size(); i++) {
      switch (param_types[i]) {
      case constant::Type::INT:
      case constant::Type::UINT: {
        if (int_reg_count < 6) {
          int_param_indx.push_back({scasm::AssemblyType::LONG_WORD, i});
          int_reg_count++;
        } else {
          stack_param_indx.push_back({scasm::AssemblyType::LONG_WORD, i});
        }
      } break;
      case constant::Type::LONG:
      case constant::Type::ULONG: {
        if (int_reg_count < 6) {
          int_param_indx.push_back({scasm::AssemblyType::QUAD_WORD, i});
          int_reg_count++;
        } else {
          stack_param_indx.push_back({scasm::AssemblyType::QUAD_WORD, i});
        }
      } break;
      case constant::Type::DOUBLE:
        if (double_reg_count < 8) {
          double_param_indx.push_back(i);
          double_reg_count++;
        } else {
          stack_param_indx.push_back({scasm::AssemblyType::DOUBLE, i});
        }
        break;
      // TODO: FIXME
      case constant::Type::ZERO:
      case constant::Type::NONE:
        break;
      }
    }
  }

  bool is_deref_lval(std::shared_ptr<ast::AST_factor_Node> factor) {
    if (factor == nullptr)
      return false;

    if (factor->get_unop_node() != nullptr and
        factor->get_unop_node()->get_op() == unop::UNOP::DEREFERENCE) {
      return true;
    }

    return factor->get_exp_node() != nullptr
               ? is_deref_lval(factor->get_exp_node()->get_factor_node())
               : false;
  }

  std::vector<scasm::register_type> int_argReg = {
      scasm::register_type::DI, scasm::register_type::SI,
      scasm::register_type::DX, scasm::register_type::CX,
      scasm::register_type::R8, scasm::register_type::R9};
  std::vector<scasm::register_type> double_argReg = {
      scasm::register_type::XMM0, scasm::register_type::XMM1,
      scasm::register_type::XMM2, scasm::register_type::XMM3,
      scasm::register_type::XMM4, scasm::register_type::XMM5,
      scasm::register_type::XMM6, scasm::register_type::XMM7};
};

} // namespace codegen
} // namespace scarlet
