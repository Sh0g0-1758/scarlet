#include <codegen/common.hh>

namespace scarlet {
namespace codegen {

#define SETUPLANDLOR(num)                                                      \
  if (exp->get_binop_node()->get_op() == binop::BINOP::LAND) {                 \
    /* Logical and */                                                          \
    scar_instruction##num->set_type(scar::instruction_type::JUMP_IF_ZERO);     \
  } else {                                                                     \
    /* Logical or */                                                           \
    scar_instruction##num->set_type(scar::instruction_type::JUMP_IF_NOT_ZERO); \
  }                                                                            \
                                                                               \
  if (exp->get_left() == nullptr) {                                            \
    if (num == 1)                                                              \
      gen_scar_factor(exp->get_factor_node(), scar_function);                  \
    SETVARCONSTANTREG(scar_val_src##num);                                      \
  } else {                                                                     \
    scar_val_src##num->set_type(scar::val_type::VAR);                          \
    scar_val_src##num->set_reg_name(get_prev_reg_name());                      \
  }                                                                            \
                                                                               \
  scar_instruction##num->set_src1(std::move(scar_val_src##num));               \
  scar_val_dst##num->set_type(scar::val_type::UNKNOWN);

void Codegen::gen_scar_assign_exp(
    std::shared_ptr<ast::AST_exp_Node> exp,
    std::shared_ptr<scar::scar_Function_Node> scar_function) {
  MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction);
  scar_instruction->set_type(scar::instruction_type::COPY);
  MAKE_SHARED(scar::scar_Val_Node, scar_val_src);
  MAKE_SHARED(scar::scar_Val_Node, scar_val_dst);
  // the semantic analyis phase should have ensured that the left child
  // is null, factor is an identifier with no unops and the right child
  // is an expression
  gen_scar_exp(exp->get_right(), scar_function);
  SETVARCONSTANTREG(scar_val_src);
  scar_instruction->set_src1(std::move(scar_val_src));

  scar_val_dst->set_type(scar::val_type::VAR);
  scar_val_dst->set_reg_name(
      exp->get_factor_node()->get_identifier_node()->get_value());
  // NOTE: we update the current scar register name to use the variable
  // since we can have expressions like:
  // int b = a = 5;
  reg_name = scar_val_dst->get_reg();
  scar_instruction->set_dst(std::move(scar_val_dst));

  scar_function->add_instruction(std::move(scar_instruction));
}

void Codegen::gen_scar_short_circuit_exp(
    std::shared_ptr<ast::AST_exp_Node> exp,
    std::shared_ptr<scar::scar_Function_Node> scar_function) {
  binop::BINOP sc_binop = exp->get_binop_node()->get_op();
  MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction);
  if (sc_binop == binop::BINOP::LAND) {
    scar_instruction->set_type(scar::instruction_type::JUMP_IF_ZERO);
  } else {
    scar_instruction->set_type(scar::instruction_type::JUMP_IF_NOT_ZERO);
  }
  MAKE_SHARED(scar::scar_Val_Node, scar_val_src1);
  MAKE_SHARED(scar::scar_Val_Node, scar_val_src2);

  if (exp->get_left() == nullptr) {
    gen_scar_factor(exp->get_factor_node(), scar_function);
    SETVARCONSTANTREG(scar_val_src1);
  } else {
    scar_val_src1->set_type(scar::val_type::VAR);
    scar_val_src1->set_reg_name(get_prev_reg_name());
  }

  scar_instruction->set_src1(std::move(scar_val_src1));

  // create a label to jump to
  MAKE_SHARED(scar::scar_Val_Node, scar_val_dst1);
  scar_val_dst1->set_type(scar::val_type::UNKNOWN);
  scar_val_dst1->set_value(get_fr_label_name());
  scar_instruction->set_dst(std::move(scar_val_dst1));
  scar_function->add_instruction(std::move(scar_instruction));

  gen_scar_exp(exp->get_right(), scar_function);
  SETVARCONSTANTREG(scar_val_src2);

  MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction2);
  if (sc_binop == binop::BINOP::LAND) {
    scar_instruction2->set_type(scar::instruction_type::JUMP_IF_ZERO);
  } else {
    scar_instruction2->set_type(scar::instruction_type::JUMP_IF_NOT_ZERO);
  }
  scar_instruction2->set_src1(std::move(scar_val_src2));
  MAKE_SHARED(scar::scar_Val_Node, scar_val_dst2);
  scar_val_dst2->set_type(scar::val_type::UNKNOWN);
  scar_val_dst2->set_value(get_last_fr_label_name());
  scar_instruction2->set_dst(std::move(scar_val_dst2));
  scar_function->add_instruction(std::move(scar_instruction2));

  // now copy 1(LAND) / 0(LOR) into a scar register
  MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction3);
  MAKE_SHARED(scar::scar_Val_Node, scar_val_src3);
  MAKE_SHARED(scar::scar_Val_Node, scar_val_dst3);
  scar_instruction3->set_type(scar::instruction_type::COPY);
  scar_val_src3->set_type(scar::val_type::CONSTANT);
  if (sc_binop == binop::BINOP::LAND) {
    scar_val_src3->set_value(std::to_string(1));
  } else {
    scar_val_src3->set_value(std::to_string(0));
  }
  scar_val_dst3->set_type(scar::val_type::VAR);
  scar_val_dst3->set_reg_name(get_reg_name());
  scar_instruction3->set_src1(std::move(scar_val_src3));
  scar_instruction3->set_dst(std::move(scar_val_dst3));
  scar_function->add_instruction(std::move(scar_instruction3));

  // now jump to the end
  MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction4);
  MAKE_SHARED(scar::scar_Val_Node, scar_val_src4);
  scar_instruction4->set_type(scar::instruction_type::JUMP);
  scar_val_src4->set_type(scar::val_type::UNKNOWN);
  scar_val_src4->set_value(get_res_label_name());
  scar_instruction4->set_src1(std::move(scar_val_src4));
  scar_function->add_instruction(std::move(scar_instruction4));

  // Now generate the intermediate label
  MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction5);
  MAKE_SHARED(scar::scar_Val_Node, scar_val_src5);
  scar_instruction5->set_type(scar::instruction_type::LABEL);
  scar_val_src5->set_type(scar::val_type::UNKNOWN);
  scar_val_src5->set_value(get_last_fr_label_name(true));
  scar_instruction5->set_src1(std::move(scar_val_src5));
  scar_function->add_instruction(std::move(scar_instruction5));

  // now copy 0(LAND) / 1(LOR) into a scar register
  MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction6);
  MAKE_SHARED(scar::scar_Val_Node, scar_val_src6);
  MAKE_SHARED(scar::scar_Val_Node, scar_val_dst6);
  scar_instruction6->set_type(scar::instruction_type::COPY);
  scar_val_src6->set_type(scar::val_type::CONSTANT);
  if (sc_binop == binop::BINOP::LAND) {
    scar_val_src6->set_value(std::to_string(0));
  } else {
    scar_val_src6->set_value(std::to_string(1));
  }
  scar_val_dst6->set_type(scar::val_type::VAR);
  scar_val_dst6->set_reg_name(get_prev_reg_name());
  scar_instruction6->set_src1(std::move(scar_val_src6));
  scar_instruction6->set_dst(std::move(scar_val_dst6));
  scar_function->add_instruction(std::move(scar_instruction6));

  // now generate the final label
  MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction7);
  MAKE_SHARED(scar::scar_Val_Node, scar_val_src7);
  scar_instruction7->set_type(scar::instruction_type::LABEL);
  scar_val_src7->set_type(scar::val_type::UNKNOWN);
  scar_val_src7->set_value(get_last_res_label_name());
  scar_instruction7->set_src1(std::move(scar_val_src7));
  scar_function->add_instruction(std::move(scar_instruction7));
}

void Codegen::gen_scar_ternary_exp(
    std::shared_ptr<ast::AST_exp_Node> exp,
    std::shared_ptr<scar::scar_Function_Node> scar_function) {

  auto texp = std::static_pointer_cast<ast::AST_ternary_exp_Node>(exp);

  MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction);
  scar_instruction->set_type(scar::instruction_type::JUMP_IF_ZERO);
  MAKE_SHARED(scar::scar_Val_Node, scar_val_src1);

  if (texp->get_left() == nullptr) {
    gen_scar_factor(texp->get_factor_node(), scar_function);
    SETVARCONSTANTREG(scar_val_src1);
  } else {
    scar_val_src1->set_type(scar::val_type::VAR);
    scar_val_src1->set_reg_name(get_prev_reg_name());
  }

  scar_instruction->set_src1(std::move(scar_val_src1));

  // create a label to jump to
  MAKE_SHARED(scar::scar_Val_Node, scar_val_dst1);
  scar_val_dst1->set_type(scar::val_type::UNKNOWN);
  scar_val_dst1->set_value(get_fr_label_name());
  scar_instruction->set_dst(std::move(scar_val_dst1));
  scar_function->add_instruction(std::move(scar_instruction));

  // generate the first expression
  gen_scar_exp(texp->get_middle(), scar_function);

  // copy this result in a new register
  MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction2);
  MAKE_SHARED(scar::scar_Val_Node, scar_val_src2);
  MAKE_SHARED(scar::scar_Val_Node, scar_val_dst2);
  scar_instruction2->set_type(scar::instruction_type::COPY);
  SETVARCONSTANTREG(scar_val_src2);
  scar_instruction2->set_src1(std::move(scar_val_src2));
  scar_val_dst2->set_type(scar::val_type::VAR);
  // Doing this so that we can use the same scar register to
  // store the result of the second expression as well.
  // We cannot simply use get_prev_reg_name() since we need to
  // parse the second expression first and that will change the
  // register counter
  std::string result = get_reg_name();
  scar_val_dst2->set_reg_name(result);
  scar_instruction2->set_dst(std::move(scar_val_dst2));
  scar_function->add_instruction(std::move(scar_instruction2));

  // jump to the end
  MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction3);
  MAKE_SHARED(scar::scar_Val_Node, scar_val_src3);
  scar_instruction3->set_type(scar::instruction_type::JUMP);
  scar_val_src3->set_type(scar::val_type::UNKNOWN);
  scar_val_src3->set_value(get_res_label_name());
  scar_instruction3->set_src1(std::move(scar_val_src3));
  scar_function->add_instruction(std::move(scar_instruction3));

  // generate the intermediate label
  MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction4);
  MAKE_SHARED(scar::scar_Val_Node, scar_val_src4);
  scar_instruction4->set_type(scar::instruction_type::LABEL);
  scar_val_src4->set_type(scar::val_type::UNKNOWN);
  scar_val_src4->set_value(get_last_fr_label_name(true));
  scar_instruction4->set_src1(std::move(scar_val_src4));
  scar_function->add_instruction(std::move(scar_instruction4));

  // generate the second expression
  gen_scar_exp(texp->get_right(), scar_function);

  // copy this result in the register we used for the first expression
  MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction5);
  MAKE_SHARED(scar::scar_Val_Node, scar_val_src5);
  MAKE_SHARED(scar::scar_Val_Node, scar_val_dst5);
  scar_instruction5->set_type(scar::instruction_type::COPY);
  SETVARCONSTANTREG(scar_val_src5);
  scar_instruction5->set_src1(std::move(scar_val_src5));
  scar_val_dst5->set_type(scar::val_type::VAR);
  scar_val_dst5->set_reg_name(result);
  scar_instruction5->set_dst(std::move(scar_val_dst5));
  scar_function->add_instruction(std::move(scar_instruction5));

  // generate the final label
  MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction6);
  MAKE_SHARED(scar::scar_Val_Node, scar_val_src6);
  scar_instruction6->set_type(scar::instruction_type::LABEL);
  scar_val_src6->set_type(scar::val_type::UNKNOWN);
  scar_val_src6->set_value(get_last_res_label_name());
  scar_instruction6->set_src1(std::move(scar_val_src6));
  scar_function->add_instruction(std::move(scar_instruction6));
}

void Codegen::gen_scar_exp(
    std::shared_ptr<ast::AST_exp_Node> exp,
    std::shared_ptr<scar::scar_Function_Node> scar_function) {
  if (exp == nullptr)
    return;
  gen_scar_exp(exp->get_left(), scar_function);
  if (exp->get_binop_node() != nullptr and
      exp->get_binop_node()->get_op() != binop::BINOP::UNKNOWN) {
    // when we have a binary operator
    // deal with && and || separately using jmpif(not)zero, labels and copy
    // operations since we need to apply short circuit for them
    //
    // There will be a separate case for the assignment operator as that
    // will be represented by a simple copy operation
    //
    // There will be a separate case for the ternary operator as well
    // So first we need to check whether it is the first expression
    // or the second expression that will be returned using jmpifzero
    // and copy the result to a new register. Ternary is actually a special
    // case of short circuiting
    if (exp->get_binop_node()->get_op() == binop::BINOP::ASSIGN) {
      gen_scar_assign_exp(exp, scar_function);
      return;
    } else if (exp->get_binop_node()->get_op() == binop::BINOP::TERNARY) {
      gen_scar_ternary_exp(exp, scar_function);
      return;
    } else if (binop::short_circuit(exp->get_binop_node()->get_op())) {
      gen_scar_short_circuit_exp(exp, scar_function);
      return;
    }

    binop::BINOP binop = exp->get_binop_node()->get_op();
    MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction);
    scar_instruction->set_type(scar::instruction_type::BINARY);
    MAKE_SHARED(scar::scar_Val_Node, scar_val_src1);
    MAKE_SHARED(scar::scar_Val_Node, scar_val_src2);

    if (exp->get_left() == nullptr) {
      gen_scar_factor(exp->get_factor_node(), scar_function);
      SETVARCONSTANTREG(scar_val_src1);
    } else {
      scar_val_src1->set_type(scar::val_type::VAR);
      scar_val_src1->set_reg_name(get_prev_reg_name());
    }

    scar_instruction->set_src1(std::move(scar_val_src1));

    gen_scar_exp(exp->get_right(), scar_function);
    SETVARCONSTANTREG(scar_val_src2);

    MAKE_SHARED(scar::scar_Val_Node, scar_val_dst);
    scar_val_dst->set_type(scar::val_type::VAR);
    if (binop::is_compound(binop)) {
      binop = compound_to_base[binop];
      scar_val_dst->set_reg_name(
          exp->get_factor_node()->get_identifier_node()->get_value());
      reg_name = scar_val_dst->get_reg();
    } else {
      scar_val_dst->set_reg_name(get_reg_name());
    }
    scar_instruction->set_src2(std::move(scar_val_src2));
    scar_instruction->set_dst(std::move(scar_val_dst));
    scar_instruction->set_binop(binop);

    scar_function->add_instruction(std::move(scar_instruction));
  } else {
    // When we do not have a binary operator, so only parse the factor node
    gen_scar_factor(exp->get_factor_node(), scar_function);
  }
}

} // namespace codegen
} // namespace scarlet
