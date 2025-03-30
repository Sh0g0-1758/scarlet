#pragma once

#include "codegen.hh"

namespace scarlet {
namespace codegen {

/* COMMON MACROS FOR SCAR GENERATION */

#define SETVARCONSTANTREG(src)                                                 \
  if (!variable_buffer.empty()) {                                              \
    src->set_type(scar::val_type::VAR);                                        \
    src->set_reg_name(variable_buffer);                                        \
    variable_buffer.clear();                                                   \
  } else if (!constant_buffer.empty()) {                                       \
    src->set_type(scar::val_type::CONSTANT);                                   \
    src->set_const_val(constant_buffer);                                       \
    constant_buffer.clear();                                                   \
  } else {                                                                     \
    src->set_type(scar::val_type::VAR);                                        \
    src->set_reg_name(get_prev_reg_name());                                    \
  }

/* COMMON MACROS FOR SCASM GENERATION */

#define MAKE_DOUBLE_CONSTANT(target, constVal, alignment)                      \
  if (doubleLabelMap.find(constVal.get_value().d) == doubleLabelMap.end()) {   \
    /* declare a top level constant for the double */                          \
    std::string doubleName = get_const_label_name();                           \
    MAKE_SHARED(scasm::scasm_static_constant, doubleConst);                    \
    doubleConst->set_name(doubleName);                                         \
    doubleConst->set_init(constVal);                                           \
    doubleConst->set_alignment(alignment);                                     \
    auto top_level_elem =                                                      \
        std::static_pointer_cast<scasm::scasm_top_level>(doubleConst);         \
    top_level_elem->set_type(scasm::scasm_top_level_type::STATIC_CONSTANT);    \
    top_level_elem->set_global(false);                                         \
    scasm_program.add_elem(std::move(top_level_elem));                         \
    /* add the constant in the backend symbol table */                         \
    scasm::backendSymbol sym;                                                  \
    sym.type = scasm::backendSymbolType::STATIC_CONSTANT;                      \
    sym.isTopLevel = true;                                                     \
    sym.asmType = scasm::AssemblyType::DOUBLE;                                 \
    backendSymbolTable[doubleName] = sym;                                      \
    /* add it to the double map so that it can be used again */                \
    doubleLabelMap[constVal.get_value().d] = doubleName;                       \
    /* put the identifier for the constant in target */                        \
    target->set_type(scasm::operand_type::PSEUDO);                             \
    target->set_identifier_stack(doubleName);                                  \
  } else {                                                                     \
    /* get the identifier from doubleMap and put it in target */               \
    std::string doubleName = doubleLabelMap[constVal.get_value().d];           \
    target->set_type(scasm::operand_type::PSEUDO);                             \
    target->set_identifier_stack(doubleName);                                  \
  }

/*
 * target      - the scasm target operand
 * set_target  - the scasm target operand setter
 * get_target  - the scar operand getter
 * instruction - the scasm instruction
 */
#define SET_OPERAND(target, set_target, get_target, instruction)               \
  switch (inst->get_target()->get_type()) {                                    \
  case scar::val_type::VAR: {                                                  \
    target->set_type(scasm::operand_type::PSEUDO);                             \
    target->set_identifier_stack(inst->get_target()->get_reg());               \
  } break;                                                                     \
  case scar::val_type::CONSTANT: {                                             \
    if (inst->get_target()->get_const_val().get_type() ==                      \
        constant::Type::DOUBLE) {                                              \
      MAKE_DOUBLE_CONSTANT(target, inst->get_target()->get_const_val(), 8);    \
    } else {                                                                   \
      target->set_type(scasm::operand_type::IMM);                              \
      target->set_imm(inst->get_target()->get_const_val());                    \
    }                                                                          \
  } break;                                                                     \
  default:                                                                     \
    break;                                                                     \
  }                                                                            \
  instruction->set_target(std::move(target));

} // namespace codegen
} // namespace scarlet
