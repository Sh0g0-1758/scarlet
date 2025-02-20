#pragma once

#include "codegen.hh"

namespace scarlet {
namespace codegen {

#define SETVARCONSTANTREG(src)                                                 \
  if (!variable_buffer.empty()) {                                              \
    src->set_type(scar::val_type::VAR);                                        \
    src->set_reg_name(variable_buffer);                                        \
    variable_buffer.clear();                                                   \
  } else if (!constant_buffer.empty()) {                                       \
    src->set_type(scar::val_type::CONSTANT);                                   \
    src->set_value(constant_buffer);                                           \
    constant_buffer.clear();                                                   \
  } else {                                                                     \
    src->set_type(scar::val_type::VAR);                                        \
    src->set_reg_name(get_prev_reg_name());                                    \
  }

} // namespace codegen
} // namespace scarlet
