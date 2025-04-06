#include <codegen/common.hh>

namespace scarlet {
namespace codegen {

#define MAKE_ALIGNED(offset, alignment)                                        \
  if (offset % alignment != 0) {                                               \
    offset = offset + (alignment - (offset % alignment));                      \
  }

#define FIX_PSEUDO(target)                                                     \
  if (NOTNULL(inst->get_##target()) &&                                         \
      inst->get_##target()->get_type() == scasm::operand_type::PSEUDO) {       \
    if (pseudoRegToMemOffset.find(inst->get_##target()->get_identifier()) !=   \
        pseudoRegToMemOffset.end()) {                                          \
      inst->get_##target()->set_offset(                                        \
          pseudoRegToMemOffset[inst->get_##target()->get_identifier()]);       \
      inst->get_##target()->set_type(scasm::operand_type::MEMORY);             \
    } else {                                                                   \
      std::string temp = inst->get_##target()->get_identifier();               \
      if (backendSymbolTable[temp].isTopLevel) {                               \
        inst->get_##target()->set_type(scasm::operand_type::DATA);             \
      } else {                                                                 \
        if (backendSymbolTable[temp].asmType ==                                \
                scasm::AssemblyType::QUAD_WORD or                              \
            backendSymbolTable[temp].asmType == scasm::AssemblyType::DOUBLE) { \
          offset += 8;                                                         \
          MAKE_ALIGNED(offset, 8);                                             \
        } else if (backendSymbolTable[temp].asmType ==                         \
                   scasm::AssemblyType::LONG_WORD) {                           \
          offset += 4;                                                         \
          MAKE_ALIGNED(offset, 4);                                             \
        }                                                                      \
        inst->get_##target()->set_type(scasm::operand_type::MEMORY);           \
        inst->get_##target()->set_reg(scasm::register_type::BP);               \
        inst->get_##target()->set_offset(-offset);                             \
        pseudoRegToMemOffset[temp] = -offset;                                  \
      }                                                                        \
    }                                                                          \
  }

void Codegen::fix_pseudo_registers() {
  for (auto &elem : scasm.get_elems()) {
    if (elem->get_type() != scasm::scasm_top_level_type::FUNCTION) {
      continue;
    }
    auto func = std::static_pointer_cast<scasm::scasm_function>(elem);
    int offset = 0;

    for (auto &inst : func->get_instructions()) {
      FIX_PSEUDO(src);
      FIX_PSEUDO(dst);
    }

    MAKE_ALIGNED(offset, 16);

    func->set_frame_size(offset);
  }
}

} // namespace codegen
} // namespace scarlet
