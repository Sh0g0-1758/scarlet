#include <codegen/codegen.hh>

namespace scarlet {
namespace codegen {

void Codegen::gen_scar() {
  scar::scar_Program_Node scar_program;
  for (auto it : program.get_declarations()) {
    if (it->get_type() == ast::DeclarationType::FUNCTION) {
      // If its a function declaration (no body) skip it
      auto funDecl =
          std::static_pointer_cast<ast::AST_function_declaration_Node>(it);
      if (funDecl->get_block() == nullptr)
        continue;

      MAKE_SHARED(scar::scar_Function_Node, scar_function);
      if (globalSymbolTable[funDecl->get_identifier()->get_value()].link ==
          symbolTable::linkage::INTERNAL) {
        scar_function->set_global(false);
      }
      MAKE_SHARED(scar::scar_Identifier_Node, identifier);
      identifier->set_value(funDecl->get_identifier()->get_value());
      scar_function->set_identifier(identifier);

      for (auto param : funDecl->get_params()) {
        MAKE_SHARED(scar::scar_Identifier_Node, param_id);
        param_id->set_value(param->identifier->get_value());
        scar_function->add_param(param_id);
      }

      gen_scar_block(funDecl->get_block(), scar_function);

      // Add a complementary return 0 at the end of the function
      // in case there is no return statement
      MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction);
      scar_instruction->set_type(scar::instruction_type::RETURN);
      MAKE_SHARED(scar::scar_Val_Node, scar_val_src);
      scar_val_src->set_type(scar::val_type::CONSTANT);
      constant::Constant ret_0;
      ret_0.set_type(constant::Type::INT);
      ret_0.set_value({.i = 0});
      scar_val_src->set_const_val(ret_0);
      scar_instruction->set_src1(scar_val_src);
      scar_function->add_instruction(scar_instruction);

      MAKE_SHARED(scar::scar_Top_Level_Node, top_level);
      top_level =
          std::static_pointer_cast<scar::scar_Top_Level_Node>(scar_function);
      top_level->set_type(scar::topLevelType::FUNCTION);

      scar_program.add_elem(std::move(top_level));
    }
  }

  for (auto symbol : globalSymbolTable) {
    if (symbol.second.type == symbolTable::symbolType::VARIABLE and
        symbol.second.def != symbolTable::defType::FALSE and
        symbol.second.link != symbolTable::linkage::NONE) {
      MAKE_SHARED(scar::scar_StaticVariable_Node, static_variable);
      MAKE_SHARED(scar::scar_Identifier_Node, identifier);
      identifier->set_value(symbol.second.name);
      static_variable->set_identifier(std::move(identifier));
      // FIXME
      static_variable->set_init(symbol.second.value[0]);
      if (symbol.second.link == symbolTable::linkage::INTERNAL) {
        static_variable->set_global(false);
      }
      auto top_level =
          std::static_pointer_cast<scar::scar_Top_Level_Node>(static_variable);
      top_level->set_type(scar::topLevelType::STATICVARIABLE);
      scar_program.add_elem(std::move(top_level));
    }
  }

  this->scar = scar_program;
}

} // namespace codegen
} // namespace scarlet
