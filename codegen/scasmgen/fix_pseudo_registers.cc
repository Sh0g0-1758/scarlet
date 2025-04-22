#include <codegen/common.hh>

namespace scarlet {
namespace codegen {

#define MAKE_ALIGNED(offset, alignment)                                        \
  if (offset % alignment != 0) {                                               \
    offset = offset + (alignment - (offset % alignment));                      \
  }

#define FIX_PSEUDO(target)                                                     \
  if (NOTNULL(inst->get_##target()) &&                                         \
      (inst->get_##target()->get_type() == scasm::operand_type::PSEUDO or      \
       inst->get_##target()->get_type() == scasm::operand_type::PSEUDO_MEM)) { \
    if (pseudoRegToMemOffset.find(inst->get_##target()->get_identifier()) !=   \
        pseudoRegToMemOffset.end()) {                                          \
      if (inst->get_##target()->get_type() ==                                  \
          scasm::operand_type::PSEUDO_MEM) {                                   \
        inst->get_##target()->set_offset(                                      \
            pseudoRegToMemOffset[inst->get_##target()->get_identifier()] +     \
            inst->get_##target()->get_offset());                               \
      } else {                                                                 \
        inst->get_##target()->set_offset(                                      \
            pseudoRegToMemOffset[inst->get_##target()->get_identifier()]);     \
      }                                                                        \
      inst->get_##target()->set_type(scasm::operand_type::MEMORY);             \
      inst->get_##target()->set_reg(scasm::register_type::BP);                 \
    } else {                                                                   \
      std::string temp = inst->get_##target()->get_identifier();               \
      if (backendSymbolTable[temp].asmType ==                                  \
              scasm::AssemblyType::QUAD_WORD or                                \
          backendSymbolTable[temp].asmType == scasm::AssemblyType::DOUBLE) {   \
        offset += 8;                                                           \
        MAKE_ALIGNED(offset, 8);                                               \
      } else if (backendSymbolTable[temp].asmType ==                           \
                 scasm::AssemblyType::LONG_WORD) {                             \
        offset += 4;                                                           \
        MAKE_ALIGNED(offset, 4);                                               \
      } else if (backendSymbolTable[temp].asmType ==                           \
                 scasm::AssemblyType::BYTE_ARRAY) {                            \
        offset += backendSymbolTable[temp].size;                               \
        MAKE_ALIGNED(offset, backendSymbolTable[temp].alignment);              \
      } else if (backendSymbolTable[temp].asmType ==                           \
                 scasm::AssemblyType::BYTE) {                                  \
        offset += 1;                                                           \
      }                                                                        \
      if (inst->get_##target()->get_type() ==                                  \
          scasm::operand_type::PSEUDO_MEM) {                                   \
        /* the Pseudo Mem operand used by arrays to copy data into some        \
         * offset. As such they already has some base positive offset and      \
         * then we change that according to the offset we calculate for the    \
         * array  */                                                           \
        inst->get_##target()->set_offset(-offset +                             \
                                         inst->get_##target()->get_offset());  \
      } else {                                                                 \
        inst->get_##target()->set_offset(-offset);                             \
      }                                                                        \
      inst->get_##target()->set_offset(-offset);                               \
      pseudoRegToMemOffset[temp] = -offset;                                    \
      inst->get_##target()->set_type(scasm::operand_type::MEMORY);             \
      inst->get_##target()->set_reg(scasm::register_type::BP);                 \
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
