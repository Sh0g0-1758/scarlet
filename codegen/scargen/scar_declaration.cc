#include <codegen/common.hh>

namespace scarlet {
namespace codegen {

void Codegen::gen_scar_declaration(
    std::shared_ptr<ast::AST_Declaration_Node> declaration,
    std::shared_ptr<scar::scar_Function_Node> scar_function) {
  // if there is no definition, we ignore it
  if (declaration->get_type() == ast::DeclarationType::VARIABLE) {
    auto variable_declaration =
        std::static_pointer_cast<ast::AST_variable_declaration_Node>(
            declaration);

    // if the declaration is local static, do nothing as it will be handled
    // later
    if (variable_declaration->get_specifier() == ast::SpecifierType::STATIC) {
      ;
    } else {
      std::string varName = variable_declaration->get_identifier()->get_value();
      if (variable_declaration->get_exp() != nullptr) {
        gen_scar_exp(variable_declaration->get_exp(), scar_function);
        MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction);
        scar_instruction->set_type(scar::instruction_type::COPY);
        MAKE_SHARED(scar::scar_Val_Node, scar_val_src);
        SETVARCONSTANTREG(scar_val_src);
        scar_instruction->set_src1(scar_val_src);
        MAKE_SHARED(scar::scar_Val_Node, scar_val_dst);
        scar_val_dst->set_type(scar::val_type::VAR);
        scar_val_dst->set_reg_name(varName);
        scar_instruction->set_dst(scar_val_dst);
        scar_function->add_instruction(scar_instruction);
      } else if (variable_declaration->get_initializer() != nullptr) {
        auto derivedType = globalSymbolTable[varName].derivedTypeMap[0];
        ast::ElemType baseType{};
        for (auto it : derivedType) {
          if (it < 0) {
            baseType = static_cast<ast::ElemType>(it);
          }
        }
        long offset = 0;
        gen_scar_initializer(variable_declaration->get_initializer(),
                             scar_function, varName, offset,
                             ast::getSizeOfTypeOnArch(baseType));
      }
    }
  }
}

void Codegen::gen_scar_initializer(
    std::shared_ptr<ast::initializer> init,
    std::shared_ptr<scar::scar_Function_Node> scar_function,
    std::string arrName, long &offset, long jump) {
  if (init == nullptr)
    return;

  if (init->initializer_list.size() > 0) {
    for (auto nested_init : init->initializer_list) {
      gen_scar_initializer(nested_init, scar_function, arrName, offset, jump);
    }
  } else {
    for (auto exp : init->exp_list) {
      MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction);
      scar_instruction->set_type(scar::instruction_type::COPY_TO_OFFSET);

      MAKE_SHARED(scar::scar_Val_Node, scar_val_src);
      gen_scar_exp(exp, scar_function);
      SETVARCONSTANTREG(scar_val_src);
      scar_instruction->set_src1(scar_val_src);

      MAKE_SHARED(scar::scar_Val_Node, scar_val_dst);
      scar_val_dst->set_type(scar::val_type::VAR);
      scar_val_dst->set_reg_name(arrName);
      scar_instruction->set_dst(scar_val_dst);

      scar_instruction->set_offset(offset);
      scar_function->add_instruction(scar_instruction);

      offset += jump;
    }
  }
}

} // namespace codegen
} // namespace scarlet
