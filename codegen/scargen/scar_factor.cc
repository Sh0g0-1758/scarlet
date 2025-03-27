#include <codegen/common.hh>

namespace scarlet {
namespace codegen {

void Codegen::gen_scar_factor(
    std::shared_ptr<ast::AST_factor_Node> factor,
    std::shared_ptr<scar::scar_Function_Node> scar_function) {

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
  } else if (factor->get_exp_node() != nullptr) {
    gen_scar_exp(factor->get_exp_node(), scar_function);
  } else if (factor->get_cast_type() != ast::ElemType::NONE) {
    gen_scar_factor(factor->get_child(), scar_function);
    
    if (factor->get_child()->get_type() != factor->get_cast_type()) {
      auto inner_type = factor->get_child()->get_type();
      auto typeCast_type =  factor->get_cast_type();
      MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction);
      MAKE_SHARED(scar::scar_Val_Node, scar_val_src);
      MAKE_SHARED(scar::scar_Val_Node, scar_val_dst);

      SETVARCONSTANTREG(scar_val_src)
      scar_instruction->set_src1(std::move(scar_val_src));

      scar_val_dst->set_type(scar::val_type::VAR);
      scar_val_dst->set_reg_name(get_reg_name(factor->get_cast_type()));
      scar_instruction->set_dst(std::move(scar_val_dst));

      if(getSizeType(typeCast_type)==getSizeType(inner_type)){
        scar_instruction->set_type(scar::instruction_type::COPY);
      }
      else if(getSizeType(typeCast_type) < getSizeType(inner_type)){
        scar_instruction->set_type(scar::instruction_type::TRUNCATE);
      }
      else if(inner_type==ast::ElemType::INT or inner_type==ast::ElemType::LONG){
        scar_instruction->set_type(scar::instruction_type::SIGN_EXTEND);
      }
      else{
        scar_instruction->set_type(scar::instruction_type::ZERO_EXTEND);
      }

      scar_function->add_instruction(std::move(scar_instruction));
    }
  } else if (factor->get_unop_node() != nullptr) {
    gen_scar_factor(factor->get_child(), scar_function);

    auto op = factor->get_unop_node()->get_op();
    MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction);

    if (op == unop::UNOP::PREDECREMENT or op == unop::UNOP::PREINCREMENT) {
      scar_instruction->set_type(scar::instruction_type::BINARY);
      if (op == unop::UNOP::PREINCREMENT) {
        scar_instruction->set_binop(binop::BINOP::ADD);
      } else {
        scar_instruction->set_binop(binop::BINOP::SUB);
      }
      MAKE_SHARED(scar::scar_Val_Node, scar_val_src1);
      MAKE_SHARED(scar::scar_Val_Node, scar_val_src2);
      MAKE_SHARED(scar::scar_Val_Node, scar_val_dst);

      scar_val_src1->set_type(scar::val_type::VAR);
      scar_val_src1->set_reg_name(variable_buffer);
      scar_instruction->set_src1(std::move(scar_val_src1));

      scar_val_dst->set_type(scar::val_type::VAR);
      scar_val_dst->set_reg_name(variable_buffer);
      scar_instruction->set_dst(std::move(scar_val_dst));

      scar_val_src2->set_type(scar::val_type::CONSTANT);
      constant::Constant one;
      one.set_type(constant::Type::INT);
      one.set_value({.i = 1});
      scar_val_src2->set_const_val(one);
      scar_instruction->set_src2(std::move(scar_val_src2));

      scar_function->add_instruction(std::move(scar_instruction));
      // propagate the updated value by not clearing the variable buffer

    } else if (op == unop::UNOP::POSTDECREMENT or
               op == unop::UNOP::POSTINCREMENT) {
      // copy the original value into a scar register
      MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction);
      scar_instruction->set_type(scar::instruction_type::COPY);
      MAKE_SHARED(scar::scar_Val_Node, scar_val_src1);
      MAKE_SHARED(scar::scar_Val_Node, scar_val_dst);
      scar_val_src1->set_type(scar::val_type::VAR);
      scar_val_src1->set_reg_name(variable_buffer);

      scar_instruction->set_src1(std::move(scar_val_src1));
      scar_val_dst->set_type(scar::val_type::VAR);
      scar_val_dst->set_reg_name(get_reg_name(factor->get_type()));
      scar_instruction->set_dst(std::move(scar_val_dst));
      scar_function->add_instruction(std::move(scar_instruction));

      // now do increment / decrement on the original value
      MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction2);
      scar_instruction2->set_type(scar::instruction_type::BINARY);
      if (op == unop::UNOP::POSTINCREMENT) {
        scar_instruction2->set_binop(binop::BINOP::ADD);
      } else {
        scar_instruction2->set_binop(binop::BINOP::SUB);
      }
      MAKE_SHARED(scar::scar_Val_Node, scar_val_src2);
      MAKE_SHARED(scar::scar_Val_Node, scar_val_src3);
      MAKE_SHARED(scar::scar_Val_Node, scar_val_dst2);

      scar_val_src2->set_type(scar::val_type::VAR);
      scar_val_src2->set_reg_name(variable_buffer);
      scar_instruction2->set_src1(std::move(scar_val_src2));

      scar_val_dst2->set_type(scar::val_type::VAR);
      scar_val_dst2->set_reg_name(variable_buffer);
      scar_instruction2->set_dst(std::move(scar_val_dst2));

      scar_val_src3->set_type(scar::val_type::CONSTANT);
      constant::Constant one;
      one.set_type(constant::Type::INT);
      one.set_value({.i = 1});
      scar_val_src3->set_const_val(one);
      scar_instruction2->set_src2(std::move(scar_val_src3));

      scar_function->add_instruction(std::move(scar_instruction2));
      variable_buffer.clear();
    } else {
      scar_instruction->set_type(scar::instruction_type::UNARY);
      scar_instruction->set_unop(op);

      MAKE_SHARED(scar::scar_Val_Node, scar_val_src);
      MAKE_SHARED(scar::scar_Val_Node, scar_val_dst);
      
      SETVARCONSTANTREG(scar_val_src)
      scar_instruction->set_src1(std::move(scar_val_src));

      scar_val_dst->set_type(scar::val_type::VAR);
      scar_val_dst->set_reg_name(get_reg_name(factor->get_type()));
      scar_instruction->set_dst(std::move(scar_val_dst));

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

  std::string dstReg = get_reg_name(factor->get_type());

  variable_buffer = dstReg;
  MAKE_SHARED(scar::scar_Val_Node, scar_val_dst);
  scar_val_dst->set_type(scar::val_type::VAR);
  scar_val_dst->set_reg_name(dstReg);
  scar_instruction->set_dst(scar_val_dst);

  scar_function->add_instruction(scar_instruction);
}

} // namespace codegen
} // namespace scarlet
