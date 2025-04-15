#include <codegen/common.hh>

namespace scarlet {
namespace codegen {

void Codegen::gen_scar_factor(
    std::shared_ptr<ast::AST_factor_Node> factor,
    std::shared_ptr<scar::scar_Function_Node> scar_function) {

  if (factor == nullptr) {
    return;
  }

  if (factor->get_const_node() != nullptr) {
    constant_buffer = factor->get_const_node()->get_constant();
  } else if (factor->get_identifier_node() != nullptr) {
    if (factor->get_factor_type() == ast::FactorType::FUNCTION_CALL) {
      gen_scar_factor_function_call(
          std::static_pointer_cast<ast::AST_factor_function_call_Node>(factor),
          scar_function);
    } else {
      variable_buffer = factor->get_identifier_node()->get_value();
    }
  } else if (factor->get_unop_node() != nullptr) {
    // if the factor is a dereference and the child is an addrof, then we can
    // simply omit the instructions for both of them. the same is true when the
    // factor is an addrof and the child is a dereference.
    if (factor->get_child() != nullptr and
        factor->get_child()->get_unop_node() != nullptr) {
      auto prev_op = factor->get_child()->get_unop_node()->get_op();
      auto curr_op = factor->get_unop_node()->get_op();
      if (curr_op == unop::UNOP::DEREFERENCE and
          prev_op == unop::UNOP::ADDROF) {
        gen_scar_factor(factor->get_child()->get_child(), scar_function);
        return;
      }
      if (curr_op == unop::UNOP::ADDROF and
          prev_op == unop::UNOP::DEREFERENCE) {
        gen_scar_factor(factor->get_child()->get_child(), scar_function);
        return;
      }
      if (curr_op == unop::UNOP::DEREFERENCE and prev_op == unop::UNOP::DECAY) {
        factor->get_child()->get_child()->set_type(factor->get_type());
        factor->get_child()->get_child()->set_derived_type(
            factor->get_derived_type());
        gen_scar_factor(factor->get_child()->get_child(), scar_function);
        return;
      }
      if (curr_op == unop::UNOP::DECAY and prev_op == unop::UNOP::DEREFERENCE) {
        factor->get_child()->get_child()->set_type(factor->get_type());
        factor->get_child()->get_child()->set_derived_type(
            factor->get_derived_type());
        gen_scar_factor(factor->get_child()->get_child(), scar_function);
        return;
      }
    }

    auto op = factor->get_unop_node()->get_op();
    MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction);
    if (op == unop::UNOP::SIZEOF) {
      constant::Constant sortc;
      sortc.set_type(constant::Type::ULONG);
      ast::ElemType baseType{};
      std::vector<long> derivedType;
      if (factor->get_exp_node() != nullptr) {
        baseType = factor->get_exp_node()->get_type();
        derivedType = factor->get_exp_node()->get_derived_type();
      } else if (factor->get_child() != nullptr) {
        baseType = factor->get_child()->get_type();
        derivedType = factor->get_child()->get_derived_type();
      } else {
        baseType = factor->get_cast_type();
        ast::unroll_derived_type(factor->get_cast_declarator(), derivedType);
        if (derivedType.size() != 0) {
          derivedType.push_back((long)baseType);
          baseType = ast::ElemType::DERIVED;
        }
      }
      if (derivedType.size() == 0) {
        sortc.set_value(
            {.ul = (unsigned long)ast::getSizeOfTypeOnArch(baseType)});
      } else {
        if (derivedType[0] == (long)ast::ElemType::POINTER) {
          sortc.set_value({.ul = 8});
        } else if (derivedType[0] > 0) {
          sortc.set_value({.ul = ast::getSizeOfArrayTypeOnArch(derivedType)});
        }
      }
      constant_buffer = sortc;
      return;
    }

    gen_scar_factor(factor->get_child(), scar_function);

    if (op == unop::UNOP::POSTDECREMENT or op == unop::UNOP::POSTINCREMENT) {
      // copy the original value into a scar register
      scar_instruction->set_type(scar::instruction_type::COPY);
      MAKE_SHARED(scar::scar_Val_Node, scar_val_src);
      MAKE_SHARED(scar::scar_Val_Node, scar_val_dst);
      SETVARCONSTANTREG(scar_val_src);
      scar_instruction->set_src1(std::move(scar_val_src));
      scar_val_dst->set_type(scar::val_type::VAR);
      scar_val_dst->set_reg_name(
          get_reg_name(factor->get_type(), factor->get_derived_type()));
      scar_instruction->set_dst(std::move(scar_val_dst));
      scar_function->add_instruction(std::move(scar_instruction));
      std::string retval = get_prev_reg_name();

      // now do increment / decrement on the original value
      gen_scar_exp(factor->get_exp_node(), scar_function);

      // propagate the original value
      variable_buffer.clear();
      constant_buffer.clear();
      reg_name = retval;
    } else if (op == unop::UNOP::DEREFERENCE) {
      scar_instruction->set_type(scar::instruction_type::LOAD);

      MAKE_SHARED(scar::scar_Val_Node, scar_val_src);
      SETVARCONSTANTREG(scar_val_src);
      scar_instruction->set_src1(std::move(scar_val_src));

      MAKE_SHARED(scar::scar_Val_Node, scar_val_dst);
      scar_val_dst->set_type(scar::val_type::VAR);
      scar_val_dst->set_reg_name(
          get_reg_name(factor->get_type(), factor->get_derived_type()));
      scar_instruction->set_dst(std::move(scar_val_dst));

      scar_function->add_instruction(std::move(scar_instruction));
    } else if (op == unop::UNOP::ADDROF or op == unop::UNOP::DECAY) {
      MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction);
      scar_instruction->set_type(scar::instruction_type::GET_ADDRESS);
      MAKE_SHARED(scar::scar_Val_Node, scar_val_src);
      MAKE_SHARED(scar::scar_Val_Node, scar_val_dst);
      SETVARCONSTANTREG(scar_val_src);
      scar_instruction->set_src1(std::move(scar_val_src));
      scar_val_dst->set_type(scar::val_type::VAR);
      scar_val_dst->set_reg_name(
          get_reg_name(factor->get_type(), factor->get_derived_type()));
      scar_instruction->set_dst(std::move(scar_val_dst));
      scar_function->add_instruction(std::move(scar_instruction));
    } else {
      scar_instruction->set_type(scar::instruction_type::UNARY);
      scar_instruction->set_unop(op);

      MAKE_SHARED(scar::scar_Val_Node, scar_val_src);
      MAKE_SHARED(scar::scar_Val_Node, scar_val_dst);

      SETVARCONSTANTREG(scar_val_src)
      scar_instruction->set_src1(std::move(scar_val_src));

      scar_val_dst->set_type(scar::val_type::VAR);
      scar_val_dst->set_reg_name(
          get_reg_name(factor->get_type(), factor->get_derived_type()));
      scar_instruction->set_dst(std::move(scar_val_dst));

      scar_function->add_instruction(std::move(scar_instruction));
    }
  } else if (factor->get_exp_node() != nullptr) {
    gen_scar_exp(factor->get_exp_node(), scar_function);
  } else if (factor->get_cast_type() != ast::ElemType::NONE) {
    gen_scar_factor(factor->get_child(), scar_function);

    auto inner_type = factor->get_child()->get_type();
    auto inner_derived_type = factor->get_child()->get_derived_type();
    auto typeCast_type = factor->get_type();
    auto typeCast_derived_type = factor->get_derived_type();

    if (typeCast_type == ast::ElemType::VOID)
      return;

    if (inner_type != typeCast_type or
        inner_derived_type != typeCast_derived_type) {
      MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction);
      MAKE_SHARED(scar::scar_Val_Node, scar_val_src);
      MAKE_SHARED(scar::scar_Val_Node, scar_val_dst);

      SETVARCONSTANTREG(scar_val_src)
      scar_instruction->set_src1(std::move(scar_val_src));

      scar_val_dst->set_type(scar::val_type::VAR);
      scar_val_dst->set_reg_name(
          get_reg_name(typeCast_type, typeCast_derived_type));
      scar_instruction->set_dst(std::move(scar_val_dst));

      // Treat derived type(pointer) as ulong
      if (typeCast_type == ast::ElemType::DERIVED and
          typeCast_derived_type[0] == (long)ast::ElemType::POINTER) {
        typeCast_type = ast::ElemType::ULONG;
      }

      if (inner_type == ast::ElemType::DERIVED and
          inner_derived_type[0] == (long)ast::ElemType::POINTER) {
        inner_type = ast::ElemType::ULONG;
      }

      if (typeCast_type == ast::ElemType::DOUBLE) {
        if (inner_type == ast::ElemType::INT or
            inner_type == ast::ElemType::LONG) {
          scar_instruction->set_type(scar::instruction_type::INT_TO_DOUBLE);
        } else {
          scar_instruction->set_type(scar::instruction_type::UINT_TO_DOUBLE);
        }
      } else if (inner_type == ast::ElemType::DOUBLE) {
        if (typeCast_type == ast::ElemType::INT or
            typeCast_type == ast::ElemType::LONG) {
          scar_instruction->set_type(scar::instruction_type::DOUBLE_TO_INT);
        } else {
          scar_instruction->set_type(scar::instruction_type::DOUBLE_TO_UINT);
        }
      } else {
        if (ast::getSizeOfTypeOnArch(typeCast_type) ==
            ast::getSizeOfTypeOnArch(inner_type)) {
          scar_instruction->set_type(scar::instruction_type::COPY);
        } else if (ast::getSizeOfTypeOnArch(typeCast_type) <
                   ast::getSizeOfTypeOnArch(inner_type)) {
          scar_instruction->set_type(scar::instruction_type::TRUNCATE);
        } else if (inner_type == ast::ElemType::INT or
                   inner_type == ast::ElemType::LONG) {
          scar_instruction->set_type(scar::instruction_type::SIGN_EXTEND);
        } else {
          scar_instruction->set_type(scar::instruction_type::ZERO_EXTEND);
        }
      }

      scar_function->add_instruction(std::move(scar_instruction));
    }
  }
}

void Codegen::gen_scar_factor_function_call(
    std::shared_ptr<ast::AST_factor_function_call_Node> factor,
    std::shared_ptr<scar::scar_Function_Node> scar_function) {

  MAKE_SHARED(scar::scar_FunctionCall_Instruction_Node, scar_instruction);
  scar_instruction->set_type(scar::instruction_type::CALL);
  MAKE_SHARED(scar::scar_Identifier_Node, scar_identifier);
  scar_identifier->set_value(factor->get_identifier_node()->get_value());
  scar_instruction->set_name(scar_identifier);

  for (auto it : factor->get_arguments()) {
    gen_scar_exp(it, scar_function);
    if (!constant_buffer.empty()) {
      MAKE_SHARED(scar::scar_Val_Node, scar_val);
      scar_val->set_type(scar::val_type::CONSTANT);
      scar_val->set_const_val(constant_buffer);
      scar_instruction->add_arg(scar_val);
      constant_buffer.clear();
    } else if (!variable_buffer.empty()) {
      MAKE_SHARED(scar::scar_Val_Node, scar_val);
      scar_val->set_type(scar::val_type::VAR);
      scar_val->set_reg_name(variable_buffer);
      scar_instruction->add_arg(scar_val);
      variable_buffer.clear();
    } else {
      MAKE_SHARED(scar::scar_Val_Node, scar_val);
      scar_val->set_type(scar::val_type::VAR);
      scar_val->set_reg_name(get_prev_reg_name());
      scar_instruction->add_arg(scar_val);
    }
  }

  if (globalSymbolTable[factor->get_identifier_node()->get_value()]
          .typeDef[0] == ast::ElemType::VOID) {
    scar_function->add_instruction(scar_instruction);
    return;
  }

  std::string dstReg =
      get_reg_name(factor->get_type(), factor->get_derived_type());

  variable_buffer = dstReg;
  MAKE_SHARED(scar::scar_Val_Node, scar_val_dst);
  scar_val_dst->set_type(scar::val_type::VAR);
  scar_val_dst->set_reg_name(dstReg);
  scar_instruction->set_dst(scar_val_dst);

  scar_function->add_instruction(scar_instruction);
}

} // namespace codegen
} // namespace scarlet
