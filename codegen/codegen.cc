#include "codegen.hh"

#define MAKE_SHARED(a, b) std::shared_ptr<a> b = std::make_shared<a>()
namespace scarlet {
namespace codegen {

#define UNREACHABLE()                                                          \
  std::cout << "Unreachable code reached in " << __FILE__ << " at line "       \
            << __LINE__ << std::endl;                                          \
  __builtin_unreachable();

void Codegen::gen_scar_factor(
    std::shared_ptr<ast::AST_factor_Node> factor,
    std::shared_ptr<scar::scar_Function_Node> scar_function) {
  // firstly put all the unops (if they exist) in the unop buffer
  for (auto it : factor->get_unop_nodes()) {
    unop_buffer[curr_buff].emplace_back(it->get_op());
  }
  // if exp exists, parse that. If exp is null, it will simply return
  if (factor->get_exp_node() != nullptr) {
    curr_buff++;
    if (curr_buff >= (int)unop_buffer.size())
      unop_buffer.resize(curr_buff + 1);
    gen_scar_exp(factor->get_exp_node(), scar_function);
    curr_buff--;
  }
  // If we have an integer node and unops to operate on, proceed...
  if (!unop_buffer[curr_buff].empty()) {
    int num_unpos = unop_buffer[curr_buff].size();
    for (int i = num_unpos - 1; i >= 0; i--) {
      // SPECIAL CASE WHEN WE HAVE INCREMENT OR DECREMENT OPERATOR
      unop::UNOP op = unop_buffer[curr_buff][i];
      if (op == unop::UNOP::PREINCREMENT or op == unop::UNOP::PREDECREMENT) {
        MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction);
        scar_instruction->set_type(scar::instruction_type::BINARY);
        if (op == unop::UNOP::PREINCREMENT) {
          scar_instruction->set_binop(binop::BINOP::ADD);
        } else {
          scar_instruction->set_binop(binop::BINOP::SUB);
        }
        MAKE_SHARED(scar::scar_Val_Node, scar_val_src);
        MAKE_SHARED(scar::scar_Val_Node, scar_val_src2);
        MAKE_SHARED(scar::scar_Val_Node, scar_val_dst);

        scar_val_dst->set_type(scar::val_type::VAR);
        // The destination will always be a variable
        if (!variable_buffer.empty()) {
          scar_val_dst->set_reg_name(variable_buffer);
          variable_buffer.clear();
        } else {
          scar_val_dst->set_reg_name(
              factor->get_identifier_node()->get_value());
        }

        reg_name = scar_val_dst->get_reg();
        scar_instruction->set_dst(std::move(scar_val_dst));

        scar_val_src->set_type(scar::val_type::VAR);
        scar_val_src->set_reg_name(reg_name);
        scar_instruction->set_src1(std::move(scar_val_src));

        scar_val_src2->set_type(scar::val_type::CONSTANT);
        scar_val_src2->set_value("1");
        scar_instruction->set_src2(std::move(scar_val_src2));

        scar_function->add_instruction(std::move(scar_instruction));
        continue;
      } else if (op == unop::UNOP::POSTINCREMENT or
                 op == unop::UNOP::POSTDECREMENT) {
        // SPECIAL CASE WHEN WE HAVE POST INCREMENT OR DECREMENT
        // Since these return the original value and then increment or decrement
        // We first make a copy of the original value and then increment or
        // decrement

        // COPY THE ORIGINAL VALUE INTO A NEW SCAR REGISTER
        MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction);
        scar_instruction->set_type(scar::instruction_type::COPY);
        MAKE_SHARED(scar::scar_Val_Node, scar_val_src);
        MAKE_SHARED(scar::scar_Val_Node, scar_val_dst);
        scar_val_src->set_type(scar::val_type::VAR);
        if (!variable_buffer.empty()) {
          scar_val_src->set_reg_name(variable_buffer);
          variable_buffer.clear();
        } else {
          scar_val_src->set_reg_name(
              factor->get_identifier_node()->get_value());
        }
        std::string _variable = scar_val_src->get_reg();
        scar_instruction->set_src1(std::move(scar_val_src));
        scar_val_dst->set_type(scar::val_type::VAR);
        scar_val_dst->set_reg_name(get_reg_name());
        scar_instruction->set_dst(std::move(scar_val_dst));
        scar_function->add_instruction(std::move(scar_instruction));

        // NOW DO THE BINARY OPERATION
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
        scar_val_src2->set_reg_name(_variable);
        scar_instruction2->set_src1(std::move(scar_val_src2));

        scar_val_src3->set_type(scar::val_type::CONSTANT);
        scar_val_src3->set_value("1");
        scar_instruction2->set_src2(std::move(scar_val_src3));

        scar_val_dst2->set_type(scar::val_type::VAR);
        scar_val_dst2->set_reg_name(_variable);
        scar_instruction2->set_dst(std::move(scar_val_dst2));

        scar_function->add_instruction(std::move(scar_instruction2));
        continue;
      }
      // scar::scar_Instruction_Node scar_instruction;
      MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction);
      scar_instruction->set_type(scar::instruction_type::UNARY);
      scar_instruction->set_unop(op);

      MAKE_SHARED(scar::scar_Val_Node, scar_val_src);
      MAKE_SHARED(scar::scar_Val_Node, scar_val_dst);

      // deal with the source
      if (i == num_unpos - 1) {
        if (factor->get_int_node() != nullptr and
            !factor->get_int_node()->get_value().empty()) {
          scar_val_src->set_type(scar::val_type::CONSTANT);
          scar_val_src->set_value(factor->get_int_node()->get_value());
        } else if (factor->get_identifier_node() != nullptr and
                   !factor->get_identifier_node()->get_value().empty()) {
          scar_val_src->set_type(scar::val_type::VAR);
          scar_val_src->set_reg_name(
              factor->get_identifier_node()->get_value());
        } else if (!constant_buffer.empty()) {
          scar_val_src->set_type(scar::val_type::CONSTANT);
          scar_val_src->set_value(constant_buffer);
          constant_buffer.clear();
        } else if (!variable_buffer.empty()) {
          scar_val_src->set_type(scar::val_type::VAR);
          scar_val_src->set_reg_name(variable_buffer);
          variable_buffer.clear();
        } else {
          scar_val_src->set_type(scar::val_type::VAR);
          scar_val_src->set_reg_name(get_prev_reg_name());
        }
      } else {
        scar_val_src->set_type(scar::val_type::VAR);
        scar_val_src->set_reg_name(get_prev_reg_name());
      }
      scar_instruction->set_src1(std::move(scar_val_src));

      // deal with the destination
      scar_val_dst->set_type(scar::val_type::VAR);
      scar_val_dst->set_reg_name(get_reg_name());
      scar_instruction->set_dst(std::move(scar_val_dst));

      scar_function->add_instruction(std::move(scar_instruction));
    }
    // empty the unop buffer
    unop_buffer[curr_buff].clear();
  } else {
    // NOTE: It is guaranteed that the factor node will have either an int node
    // or an identifier node

    // save constant for later use
    if (factor->get_int_node() != nullptr and
        !factor->get_int_node()->get_value().empty()) {
      constant_buffer = factor->get_int_node()->get_value();
    }

    // save variable for later use
    if (factor->get_identifier_node() != nullptr and
        !factor->get_identifier_node()->get_value().empty()) {
      variable_buffer = factor->get_identifier_node()->get_value();
    }
  }
}

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
      return;
    }
    bool short_circuit = binop::short_circuit(exp->get_binop_node()->get_op());
    binop::BINOP sc_binop = exp->get_binop_node()->get_op();
    const binop::BINOP compound_binop = sc_binop;
    if (binop::is_compound(compound_binop)) {
      sc_binop = compound_to_base[compound_binop];
    }
    MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction);
    if (short_circuit) {
      if (sc_binop == binop::BINOP::LAND) {
        scar_instruction->set_type(scar::instruction_type::JUMP_IF_ZERO);
      } else if (sc_binop == binop::BINOP::LOR) {
        scar_instruction->set_type(scar::instruction_type::JUMP_IF_NOT_ZERO);
      } else if (sc_binop == binop::BINOP::TERNARY) {
        scar_instruction->set_type(scar::instruction_type::JUMP_IF_ZERO);
      }
    } else {
      scar_instruction->set_type(scar::instruction_type::BINARY);
      scar_instruction->set_binop(sc_binop);
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

    if (short_circuit) {
      // create a label to jump to
      MAKE_SHARED(scar::scar_Val_Node, scar_val_dst1);
      scar_val_dst1->set_type(scar::val_type::UNKNOWN);
      scar_val_dst1->set_value(get_fr_label_name());
      scar_instruction->set_dst(std::move(scar_val_dst1));
      scar_function->add_instruction(std::move(scar_instruction));
    }

    // TERNARY SPECIAL CASE
    if (sc_binop == binop::BINOP::TERNARY) {
      // generate the first expression
      gen_scar_exp(exp->get_middle(), scar_function);

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
      gen_scar_exp(exp->get_right(), scar_function);

      // copy this result in a new register
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

      // early return
      return;
    }
    gen_scar_exp(exp->get_right(), scar_function);
    SETVARCONSTANTREG(scar_val_src2);

    if (short_circuit) {
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
    } else {
      MAKE_SHARED(scar::scar_Val_Node, scar_val_dst);
      scar_val_dst->set_type(scar::val_type::VAR);
      if (binop::is_compound(compound_binop)) {
        scar_val_dst->set_reg_name(
            exp->get_factor_node()->get_identifier_node()->get_value());
        reg_name = scar_val_dst->get_reg();
      } else {
        scar_val_dst->set_reg_name(get_reg_name());
      }
      scar_instruction->set_src2(std::move(scar_val_src2));
      scar_instruction->set_dst(std::move(scar_val_dst));

      scar_function->add_instruction(std::move(scar_instruction));
    }
  } else {
    // When we do not have a binary operator, so only parse the factor node
    gen_scar_factor(exp->get_factor_node(), scar_function);
  }
}

void Codegen::gen_scar_statement(
    std::shared_ptr<ast::AST_Statement_Node> statement,
    std::shared_ptr<scar::scar_Function_Node> scar_function) {
  switch (statement->get_type()) {
  case ast::statementType::NULLSTMT:
    break;
  case ast::statementType::RETURN: {
    gen_scar_exp(statement->get_exps(), scar_function);
    MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction);
    scar_instruction->set_type(scar::instruction_type::RETURN);
    MAKE_SHARED(scar::scar_Val_Node, scar_val_src);
    SETVARCONSTANTREG(scar_val_src);
    scar_instruction->set_src1(scar_val_src);
    scar_function->add_instruction(scar_instruction);
  } break;
  case ast::statementType::EXP: {
    gen_scar_exp(statement->get_exps(), scar_function);
  } break;
  case ast::statementType::IF: {
    // <instructions for condition> (stored in exps)
    // c = <result of condition>
    // JumpIfZero(c, end)
    // <instructions for statement> (handled in the next iteration)
    // Label(end)            (Added during the end of if statement)
    auto if_statement =
        std::static_pointer_cast<ast::AST_if_else_statement_Node>(statement);
    gen_scar_exp(if_statement->get_exps(), scar_function);
    MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction);
    scar_instruction->set_type(scar::instruction_type::JUMP_IF_ZERO);
    MAKE_SHARED(scar::scar_Val_Node, scar_val_src);
    SETVARCONSTANTREG(scar_val_src);
    scar_instruction->set_src1(std::move(scar_val_src));
    MAKE_SHARED(scar::scar_Val_Node, scar_val_dst);
    scar_val_dst->set_type(scar::val_type::UNKNOWN);
    scar_val_dst->set_value(if_statement->get_labels().first->get_value());
    scar_instruction->set_dst(std::move(scar_val_dst));
    scar_function->add_instruction(std::move(scar_instruction));

    // generate scar for the statement
    gen_scar_statement(if_statement->get_stmt1(), scar_function);

    // generate the end label
    MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction2);
    scar_instruction2->set_type(scar::instruction_type::LABEL);
    MAKE_SHARED(scar::scar_Val_Node, scar_val_src2);
    scar_val_src2->set_type(scar::val_type::UNKNOWN);
    scar_val_src2->set_value(if_statement->get_labels().first->get_value());
    scar_instruction2->set_src1(std::move(scar_val_src2));
    scar_function->add_instruction(std::move(scar_instruction2));
  } break;
  case ast::statementType::IFELSE: {
    // <instructions for condition>
    // c = <result of condition>
    // JumpIfZero(c, else_label)
    // <instructions for statement1>
    // Jump(end)
    // Label(else_label)
    // <instructions for statement2>
    // Label(end)

    auto if_else_statement =
        std::static_pointer_cast<ast::AST_if_else_statement_Node>(statement);
    gen_scar_exp(if_else_statement->get_exps(), scar_function);

    // Jump if zero to else statement
    MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction);
    scar_instruction->set_type(scar::instruction_type::JUMP_IF_ZERO);
    MAKE_SHARED(scar::scar_Val_Node, scar_val_src);
    SETVARCONSTANTREG(scar_val_src);
    scar_instruction->set_src1(std::move(scar_val_src));
    MAKE_SHARED(scar::scar_Val_Node, scar_val_dst);
    scar_val_dst->set_type(scar::val_type::UNKNOWN);
    scar_val_dst->set_value(if_else_statement->get_labels().first->get_value());
    scar_instruction->set_dst(std::move(scar_val_dst));
    scar_function->add_instruction(std::move(scar_instruction));

    // generate scar for the if statement
    gen_scar_statement(if_else_statement->get_stmt1(), scar_function);

    // jump to the end
    MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction2);
    scar_instruction2->set_type(scar::instruction_type::JUMP);
    MAKE_SHARED(scar::scar_Val_Node, scar_val_src2);
    scar_val_src2->set_type(scar::val_type::UNKNOWN);
    scar_val_src2->set_value(
        if_else_statement->get_labels().second->get_value());
    scar_instruction2->set_src1(std::move(scar_val_src2));
    scar_function->add_instruction(std::move(scar_instruction2));

    // generate the else label
    MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction3);
    scar_instruction3->set_type(scar::instruction_type::LABEL);
    MAKE_SHARED(scar::scar_Val_Node, scar_val_src3);
    scar_val_src3->set_type(scar::val_type::UNKNOWN);
    scar_val_src3->set_value(
        if_else_statement->get_labels().first->get_value());
    scar_instruction3->set_src1(std::move(scar_val_src3));
    scar_function->add_instruction(std::move(scar_instruction3));

    // generate scar for the else statement
    gen_scar_statement(if_else_statement->get_stmt2(), scar_function);

    // generate the end label
    MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction4);
    scar_instruction4->set_type(scar::instruction_type::LABEL);
    MAKE_SHARED(scar::scar_Val_Node, scar_val_src4);
    scar_val_src4->set_type(scar::val_type::UNKNOWN);
    scar_val_src4->set_value(
        if_else_statement->get_labels().second->get_value());
    scar_instruction4->set_src1(std::move(scar_val_src4));
    scar_function->add_instruction(std::move(scar_instruction4));
  } break;

  case ast::statementType::GOTO:
  case ast::statementType::BREAK:
  case ast::statementType::CONTINUE: {
    MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction);
    scar_instruction->set_type(scar::instruction_type::JUMP);
    MAKE_SHARED(scar::scar_Val_Node, scar_val_src);
    scar_val_src->set_type(scar::val_type::UNKNOWN);
    scar_val_src->set_value(statement->get_labels().first->get_value());
    scar_instruction->set_src1(std::move(scar_val_src));
    scar_function->add_instruction(scar_instruction);
  } break;

  case ast::statementType::LABEL: {
    MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction);
    scar_instruction->set_type(scar::instruction_type::LABEL);
    MAKE_SHARED(scar::scar_Val_Node, scar_val_src);
    scar_val_src->set_type(scar::val_type::UNKNOWN);
    scar_val_src->set_value(statement->get_labels().first->get_value());
    scar_instruction->set_src1(std::move(scar_val_src));
    scar_function->add_instruction(scar_instruction);
  } break;

  case ast::statementType::BLOCK: {
    auto block_statement =
        std::static_pointer_cast<ast::AST_block_statement_node>(statement);
    gen_scar_block(block_statement->get_block(), scar_function);
  } break;

  case ast::statementType::WHILE: {
    auto while_statement =
        std::static_pointer_cast<ast::AST_while_statement_Node>(statement);

    // Label(start)
    // <instructions for condition>
    // c = <result of condition>
    // JumpIfZero(c, end)
    // <instructions for statement>
    // Jump(start)
    // Label(end)

    MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction);
    scar_instruction->set_type(scar::instruction_type::LABEL);
    MAKE_SHARED(scar::scar_Val_Node, scar_val_src);
    scar_val_src->set_type(scar::val_type::UNKNOWN);
    scar_val_src->set_value(while_statement->get_labels().first->get_value());
    scar_instruction->set_src1(std::move(scar_val_src));
    scar_function->add_instruction(scar_instruction);

    gen_scar_exp(while_statement->get_exps(), scar_function);

    MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction2);
    scar_instruction2->set_type(scar::instruction_type::JUMP_IF_ZERO);
    MAKE_SHARED(scar::scar_Val_Node, scar_val_src2);
    SETVARCONSTANTREG(scar_val_src2);
    scar_instruction2->set_src1(std::move(scar_val_src2));
    MAKE_SHARED(scar::scar_Val_Node, scar_val_dst2);
    scar_val_dst2->set_type(scar::val_type::UNKNOWN);
    scar_val_dst2->set_value(while_statement->get_labels().second->get_value());
    scar_instruction2->set_dst(std::move(scar_val_dst2));
    scar_function->add_instruction(scar_instruction2);

    gen_scar_statement(while_statement->get_stmt(), scar_function);

    MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction3);
    scar_instruction3->set_type(scar::instruction_type::JUMP);
    MAKE_SHARED(scar::scar_Val_Node, scar_val_src3);
    scar_val_src3->set_type(scar::val_type::UNKNOWN);
    scar_val_src3->set_value(while_statement->get_labels().first->get_value());
    scar_instruction3->set_src1(std::move(scar_val_src3));
    scar_function->add_instruction(scar_instruction3);

    MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction4);
    scar_instruction4->set_type(scar::instruction_type::LABEL);
    MAKE_SHARED(scar::scar_Val_Node, scar_val_src4);
    scar_val_src4->set_type(scar::val_type::UNKNOWN);
    scar_val_src4->set_value(while_statement->get_labels().second->get_value());
    scar_instruction4->set_src1(std::move(scar_val_src4));
    scar_function->add_instruction(scar_instruction4);
  } break;

  case ast::statementType::DO_WHILE: {
    auto do_while_statement =
        std::static_pointer_cast<ast::AST_while_statement_Node>(statement);

    // Label(start)
    // <instructions for statement>
    // <instructions for condition>
    // c = <result of condition>
    // JumpIfNotZero(c, start)
    // Label(end)

    MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction);
    scar_instruction->set_type(scar::instruction_type::LABEL);
    MAKE_SHARED(scar::scar_Val_Node, scar_val_src);
    scar_val_src->set_type(scar::val_type::UNKNOWN);
    scar_val_src->set_value(
        do_while_statement->get_labels().first->get_value());
    scar_instruction->set_src1(std::move(scar_val_src));
    scar_function->add_instruction(scar_instruction);

    gen_scar_statement(do_while_statement->get_stmt(), scar_function);

    gen_scar_exp(do_while_statement->get_exps(), scar_function);

    MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction2);
    scar_instruction2->set_type(scar::instruction_type::JUMP_IF_NOT_ZERO);
    MAKE_SHARED(scar::scar_Val_Node, scar_val_src2);
    SETVARCONSTANTREG(scar_val_src2);
    scar_instruction2->set_src1(std::move(scar_val_src2));
    MAKE_SHARED(scar::scar_Val_Node, scar_val_dst2);
    scar_val_dst2->set_type(scar::val_type::UNKNOWN);
    scar_val_dst2->set_value(
        do_while_statement->get_labels().first->get_value());
    scar_instruction2->set_dst(std::move(scar_val_dst2));
    scar_function->add_instruction(scar_instruction2);

    MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction3);
    scar_instruction3->set_type(scar::instruction_type::LABEL);
    MAKE_SHARED(scar::scar_Val_Node, scar_val_src3);
    scar_val_src3->set_type(scar::val_type::UNKNOWN);
    scar_val_src3->set_value(
        do_while_statement->get_labels().second->get_value());
    scar_instruction3->set_src1(std::move(scar_val_src3));
    scar_function->add_instruction(scar_instruction3);
  } break;

  case ast::statementType::FOR: {
    auto for_statement =
        std::static_pointer_cast<ast::AST_For_Statement_Node>(statement);

    // <instructions for init>
    // Label(Forstart)
    // <instructions for condition>
    // c = <result of condition>
    // JumpIfZero(c, end)
    // <instructions for statement>
    // Label(continue)
    // <instructions for post>
    // Jump(Forstart)
    // Label(end)

    if (for_statement->get_for_init() != nullptr) {
      if (for_statement->get_for_init()->get_declaration() != nullptr) {
        gen_scar_declaration(for_statement->get_for_init()->get_declaration(),
                             scar_function);
      } else {
        gen_scar_exp(for_statement->get_for_init()->get_exp(), scar_function);
      }
    }

    std::string FOR_START = get_for_label_name();

    MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction);
    scar_instruction->set_type(scar::instruction_type::LABEL);
    MAKE_SHARED(scar::scar_Val_Node, scar_val_src);
    scar_val_src->set_type(scar::val_type::UNKNOWN);
    scar_val_src->set_value(FOR_START);
    scar_instruction->set_src1(std::move(scar_val_src));
    scar_function->add_instruction(scar_instruction);

    gen_scar_exp(for_statement->get_exps(), scar_function);

    MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction2);
    scar_instruction2->set_type(scar::instruction_type::JUMP_IF_ZERO);
    MAKE_SHARED(scar::scar_Val_Node, scar_val_src2);
    SETVARCONSTANTREG(scar_val_src2);
    scar_instruction2->set_src1(std::move(scar_val_src2));
    MAKE_SHARED(scar::scar_Val_Node, scar_val_dst2);
    scar_val_dst2->set_type(scar::val_type::UNKNOWN);
    scar_val_dst2->set_value(for_statement->get_labels().second->get_value());
    scar_instruction2->set_dst(std::move(scar_val_dst2));
    scar_function->add_instruction(scar_instruction2);

    gen_scar_statement(for_statement->get_stmt(), scar_function);

    MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction3);
    scar_instruction3->set_type(scar::instruction_type::LABEL);
    MAKE_SHARED(scar::scar_Val_Node, scar_val_src3);
    scar_val_src3->set_type(scar::val_type::UNKNOWN);
    scar_val_src3->set_value(for_statement->get_labels().first->get_value());
    scar_instruction3->set_src1(std::move(scar_val_src3));
    scar_function->add_instruction(scar_instruction3);

    gen_scar_exp(for_statement->get_exp2(), scar_function);

    MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction4);
    scar_instruction4->set_type(scar::instruction_type::JUMP);
    MAKE_SHARED(scar::scar_Val_Node, scar_val_src4);
    scar_val_src4->set_type(scar::val_type::UNKNOWN);
    scar_val_src4->set_value(FOR_START);
    scar_instruction4->set_src1(std::move(scar_val_src4));
    scar_function->add_instruction(scar_instruction4);

    MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction5);
    scar_instruction5->set_type(scar::instruction_type::LABEL);
    MAKE_SHARED(scar::scar_Val_Node, scar_val_src5);
    scar_val_src5->set_type(scar::val_type::UNKNOWN);
    scar_val_src5->set_value(for_statement->get_labels().second->get_value());
    scar_instruction5->set_src1(std::move(scar_val_src5));
    scar_function->add_instruction(scar_instruction5);
  } break;
  case ast::statementType::UNKNOWN:
    UNREACHABLE()
  }
}
void Codegen::gen_scar_declaration(
    std::shared_ptr<ast::AST_Declaration_Node> declaration,
    std::shared_ptr<scar::scar_Function_Node> scar_function) {
  // if there is no definition, we ignore it
  if (declaration->get_exp() != nullptr) {
    gen_scar_exp(declaration->get_exp(), scar_function);
    MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction);
    scar_instruction->set_type(scar::instruction_type::COPY);
    MAKE_SHARED(scar::scar_Val_Node, scar_val_src);
    SETVARCONSTANTREG(scar_val_src);
    scar_instruction->set_src1(scar_val_src);
    MAKE_SHARED(scar::scar_Val_Node, scar_val_dst);
    scar_val_dst->set_type(scar::val_type::VAR);
    scar_val_dst->set_reg_name(declaration->get_identifier()->get_value());
    scar_instruction->set_dst(scar_val_dst);
    scar_function->add_instruction(scar_instruction);
  }
}

void Codegen::gen_scar_block(
    std::shared_ptr<ast::AST_Block_Node> block,
    std::shared_ptr<scar::scar_Function_Node> scar_function) {
  for (auto inst : block->get_blockItems()) {
    switch (inst->get_type()) {
    case ast::BlockItemType::STATEMENT:
      gen_scar_statement(inst->get_statement(), scar_function);
      break;
    case ast::BlockItemType::DECLARATION:
      gen_scar_declaration(inst->get_declaration(), scar_function);
      break;
    case ast::BlockItemType::UNKNOWN:
      UNREACHABLE()
    }
  }
}

void Codegen::gen_scar() {
  scar::scar_Program_Node scar_program;
  for (auto it : program.get_functions()) {
    MAKE_SHARED(scar::scar_Function_Node, scar_function);
    MAKE_SHARED(scar::scar_Identifier_Node, identifier);
    identifier->set_value(it->get_identifier()->get_value());
    scar_function->set_identifier(identifier);

    gen_scar_block(it->get_block(), scar_function);

    // Add a complementary return 0 at the end of the function
    // in case there is no return statement
    MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction);
    scar_instruction->set_type(scar::instruction_type::RETURN);
    MAKE_SHARED(scar::scar_Val_Node, scar_val_src);
    scar_val_src->set_type(scar::val_type::CONSTANT);
    scar_val_src->set_value("0");
    scar_instruction->set_src1(scar_val_src);
    scar_function->add_instruction(scar_instruction);

    scar_program.add_function(scar_function);
  }

  this->scar = scar_program;
}

void Codegen::pretty_print() {
  std::cout << "Program(" << std::endl;
  for (auto function : scar.get_functions()) {
    std::cout << "\tFunction(" << std::endl;
    std::cout << "\t\tname=\"" << function->get_identifier()->get_value()
              << "\"," << std::endl;
    std::cout << "\t\tbody=[" << std::endl;
    for (auto statement : function->get_instructions()) {
      std::cout << "\t\t\t" << to_string(statement->get_type()) << "(";
      if (statement->get_type() == scar::instruction_type::RETURN) {
        if (statement->get_src1()->get_type() == scar::val_type::CONSTANT) {
          std::cout << "Constant(" << statement->get_src1()->get_value() << ")";
        } else if (statement->get_src1()->get_type() == scar::val_type::VAR) {
          std::cout << "Var(" << statement->get_src1()->get_reg() << ")";
        }
        std::cout << ")" << std::endl;
      } else if (statement->get_type() == scar::instruction_type::UNARY) {
        std::cout << unop::to_string(statement->get_unop()) << ", ";
        if (statement->get_src1()->get_type() == scar::val_type::CONSTANT) {
          std::cout << "Constant(" << statement->get_src1()->get_value() << ")";
        } else if (statement->get_src1()->get_type() == scar::val_type::VAR) {
          std::cout << "Var(" << statement->get_src1()->get_reg() << ")";
        }
        std::cout << ", ";
        if (statement->get_dst()->get_type() == scar::val_type::VAR) {
          std::cout << "Var(" << statement->get_dst()->get_reg() << ")";
        } else if (statement->get_dst()->get_type() ==
                   scar::val_type::CONSTANT) {
          std::cout << "Constant(" << statement->get_dst()->get_value() << ")";
        }
        std::cout << ")" << std::endl;
      } else if (statement->get_type() == scar::instruction_type::BINARY) {
        std::cout << binop::to_string(statement->get_binop()) << ", ";
        if (statement->get_src1()->get_type() == scar::val_type::VAR) {
          std::cout << "Var(" << statement->get_src1()->get_reg() << ")";
        } else if (statement->get_src1()->get_type() ==
                   scar::val_type::CONSTANT) {
          std::cout << "Constant(" << statement->get_src1()->get_value() << ")";
        }
        std::cout << ", ";
        if (statement->get_src2()->get_type() == scar::val_type::VAR) {
          std::cout << "Var(" << statement->get_src2()->get_reg() << ")";
        } else if (statement->get_src2()->get_type() ==
                   scar::val_type::CONSTANT) {
          std::cout << "Constant(" << statement->get_src2()->get_value() << ")";
        }
        std::cout << ", ";
        if (statement->get_dst()->get_type() == scar::val_type::VAR) {
          std::cout << "Var(" << statement->get_dst()->get_reg() << ")";
        } else if (statement->get_dst()->get_type() ==
                   scar::val_type::CONSTANT) {
          std::cout << "Constant(" << statement->get_dst()->get_value() << ")";
        }
        std::cout << ")" << std::endl;
      } else if (statement->get_type() == scar::instruction_type::COPY) {
        if (statement->get_src1()->get_type() == scar::val_type::VAR) {
          std::cout << "Var(" << statement->get_src1()->get_reg() << ")";
        } else if (statement->get_src1()->get_type() ==
                   scar::val_type::CONSTANT) {
          std::cout << "Constant(" << statement->get_src1()->get_value() << ")";
        }
        std::cout << " ,";
        if (statement->get_dst()->get_type() == scar::val_type::VAR) {
          std::cout << "Var(" << statement->get_dst()->get_reg() << ")";
        }
        std::cout << ")" << std::endl;
      } else if (statement->get_type() == scar::instruction_type::JUMP or
                 statement->get_type() == scar::instruction_type::LABEL) {
        std::cout << statement->get_src1()->get_value() << ")" << std::endl;
      } else if (statement->get_type() ==
                     scar::instruction_type::JUMP_IF_ZERO or
                 statement->get_type() ==
                     scar::instruction_type::JUMP_IF_NOT_ZERO) {
        if (statement->get_src1()->get_type() == scar::val_type::VAR) {
          std::cout << "Var(" << statement->get_src1()->get_reg() << ")";
        } else if (statement->get_src1()->get_type() ==
                   scar::val_type::CONSTANT) {
          std::cout << "Constant(" << statement->get_src1()->get_value() << ")";
        }
        std::cout << ", ";
        std::cout << statement->get_dst()->get_value() << ")" << std::endl;
      }
    }
    std::cout << "\t\t]" << std::endl;
    std::cout << "\t)," << std::endl;
  }
  std::cout << ")" << std::endl;
}

#define SET_MOV_SOURCE()                                                       \
  switch (inst->get_src1()->get_type()) {                                      \
  case scar::val_type::VAR:                                                    \
    scasm_src->set_type(scasm::operand_type::PSEUDO);                          \
    scasm_src->set_identifier_stack(inst->get_src1()->get_reg());              \
    break;                                                                     \
  case scar::val_type::CONSTANT:                                               \
    scasm_src->set_type(scasm::operand_type::IMM);                             \
    scasm_src->set_imm(stoi(inst->get_src1()->get_value()));                   \
    break;                                                                     \
  default:                                                                     \
    break;                                                                     \
  }                                                                            \
  scasm_inst->set_src(std::move(scasm_src))

#define SET_DST(dst)                                                           \
  switch (inst->get_dst()->get_type()) {                                       \
  case scar::val_type::VAR:                                                    \
    dst->set_type(scasm::operand_type::PSEUDO);                                \
    dst->set_identifier_stack(inst->get_dst()->get_reg());                     \
    break;                                                                     \
  default:                                                                     \
    break;                                                                     \
  }

void Codegen::gen_scasm() {
  scasm::scasm_program scasm_program{};
  for (auto func : scar.get_functions()) {
    MAKE_SHARED(scasm::scasm_function, scasm_func);
    scasm_func->set_name(func->get_identifier()->get_value());
    for (auto inst : func->get_instructions()) {
      if (inst->get_type() == scar::instruction_type::RETURN) {
        MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
        scasm_inst->set_type(scasm::instruction_type::MOV);
        MAKE_SHARED(scasm::scasm_operand, scasm_src);
        SET_MOV_SOURCE();
        MAKE_SHARED(scasm::scasm_operand, scasm_dst);
        scasm_dst->set_type(scasm::operand_type::REG);
        scasm_dst->set_reg(scasm::register_type::AX);
        scasm_inst->set_dst(std::move(scasm_dst));
        scasm_func->add_instruction(std::move(scasm_inst));

        MAKE_SHARED(scasm::scasm_instruction, scasm_inst2);
        scasm_inst2->set_type(scasm::instruction_type::RET);
        scasm_func->add_instruction(std::move(scasm_inst2));

      } else if (inst->get_type() == scar::instruction_type::UNARY) {
        if (inst->get_unop() == unop::UNOP::NOT) {
          // Cmp(Imm(0), src)
          // Mov(Imm(0), dst)
          // SetCC(E, dst)

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
          scasm_inst->set_type(scasm::instruction_type::CMP);
          MAKE_SHARED(scasm::scasm_operand, scasm_src);
          scasm_src->set_type(scasm::operand_type::IMM);
          scasm_src->set_imm(0);
          scasm_inst->set_src(std::move(scasm_src));
          MAKE_SHARED(scasm::scasm_operand, scasm_dst);
          switch (inst->get_src1()->get_type()) {
          case scar::val_type::VAR:
            scasm_dst->set_type(scasm::operand_type::PSEUDO);
            scasm_dst->set_identifier_stack(inst->get_src1()->get_reg());
            break;
          case scar::val_type::CONSTANT:
            scasm_dst->set_type(scasm::operand_type::IMM);
            scasm_dst->set_imm(stoi(inst->get_src1()->get_value()));
            break;
          default:
            break;
          }
          scasm_inst->set_dst(std::move(scasm_dst));
          scasm_func->add_instruction(std::move(scasm_inst));

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst2);
          scasm_inst2->set_type(scasm::instruction_type::MOV);
          MAKE_SHARED(scasm::scasm_operand, scasm_src2);
          scasm_src2->set_type(scasm::operand_type::IMM);
          scasm_src2->set_imm(0);
          scasm_inst2->set_src(std::move(scasm_src2));
          MAKE_SHARED(scasm::scasm_operand, scasm_dst2);
          SET_DST(scasm_dst2);
          scasm_inst2->set_dst(std::move(scasm_dst2));
          scasm_func->add_instruction(std::move(scasm_inst2));

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst3);
          scasm_inst3->set_type(scasm::instruction_type::SETCC);
          MAKE_SHARED(scasm::scasm_operand, scasm_src3);
          scasm_src3->set_type(scasm::operand_type::COND);
          scasm_src3->set_cond(scasm::cond_code::E);
          scasm_inst3->set_src(std::move(scasm_src3));
          MAKE_SHARED(scasm::scasm_operand, scasm_dst3);
          switch (inst->get_dst()->get_type()) {
          case scar::val_type::VAR:
            scasm_dst3->set_type(scasm::operand_type::PSEUDO);
            scasm_dst3->set_identifier_stack(inst->get_dst()->get_reg());
            break;
          default:
            break;
          }
          scasm_inst3->set_dst(std::move(scasm_dst3));
          scasm_func->add_instruction(std::move(scasm_inst3));
        } else {
          MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
          scasm_inst->set_type(scasm::instruction_type::MOV);
          MAKE_SHARED(scasm::scasm_operand, scasm_src);
          SET_MOV_SOURCE();

          MAKE_SHARED(scasm::scasm_operand, scasm_dst);
          if (inst->get_dst()->get_type() == scar::val_type::VAR) {
            scasm_dst->set_type(scasm::operand_type::PSEUDO);
            scasm_dst->set_identifier_stack(inst->get_dst()->get_reg());
          }
          scasm_inst->set_dst(std::move(scasm_dst));

          scasm_func->add_instruction(std::move(scasm_inst));

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst2);
          scasm_inst2->set_type(scasm::instruction_type::UNARY);
          scasm_inst2->set_unop(inst->get_unop());

          MAKE_SHARED(scasm::scasm_operand, scasm_dst2);
          if (inst->get_dst()->get_type() == scar::val_type::VAR) {
            scasm_dst2->set_type(scasm::operand_type::PSEUDO);
            scasm_dst2->set_identifier_stack(inst->get_dst()->get_reg());
          }
          scasm_inst2->set_dst(std::move(scasm_dst2));

          scasm_func->add_instruction(std::move(scasm_inst2));
        }
      } else if (inst->get_type() == scar::instruction_type::BINARY) {
        if (binop::is_relational(inst->get_binop())) {
          // Cmp(src2, src1)
          // Mov(Imm(0), dst)
          // SetCC(conditional, dst)
          MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
          scasm_inst->set_type(scasm::instruction_type::CMP);
          MAKE_SHARED(scasm::scasm_operand, scasm_src);
          switch (inst->get_src2()->get_type()) {
          case scar::val_type::VAR:
            scasm_src->set_type(scasm::operand_type::PSEUDO);
            scasm_src->set_identifier_stack(inst->get_src2()->get_reg());
            break;
          case scar::val_type::CONSTANT:
            scasm_src->set_type(scasm::operand_type::IMM);
            scasm_src->set_imm(stoi(inst->get_src2()->get_value()));
            break;
          default:
            break;
          }
          scasm_inst->set_src(std::move(scasm_src));
          MAKE_SHARED(scasm::scasm_operand, scasm_dst);
          switch (inst->get_src1()->get_type()) {
          case scar::val_type::VAR:
            scasm_dst->set_type(scasm::operand_type::PSEUDO);
            scasm_dst->set_identifier_stack(inst->get_src1()->get_reg());
            break;
          case scar::val_type::CONSTANT:
            scasm_dst->set_type(scasm::operand_type::IMM);
            scasm_dst->set_imm(stoi(inst->get_src1()->get_value()));
            break;
          default:
            break;
          }
          scasm_inst->set_dst(std::move(scasm_dst));
          scasm_func->add_instruction(std::move(scasm_inst));

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst2);
          scasm_inst2->set_type(scasm::instruction_type::MOV);
          MAKE_SHARED(scasm::scasm_operand, scasm_src2);
          scasm_src2->set_type(scasm::operand_type::IMM);
          scasm_src2->set_imm(0);
          scasm_inst2->set_src(std::move(scasm_src2));
          MAKE_SHARED(scasm::scasm_operand, scasm_dst2);
          SET_DST(scasm_dst2);
          scasm_inst2->set_dst(std::move(scasm_dst2));
          scasm_func->add_instruction(std::move(scasm_inst2));

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst3);
          scasm_inst3->set_type(scasm::instruction_type::SETCC);
          MAKE_SHARED(scasm::scasm_operand, scasm_src3);
          scasm_src3->set_type(scasm::operand_type::COND);
          switch (inst->get_binop()) {
          case binop::BINOP::EQUAL:
            scasm_src3->set_cond(scasm::cond_code::E);
            break;
          case binop::BINOP::NOTEQUAL:
            scasm_src3->set_cond(scasm::cond_code::NE);
            break;
          case binop::BINOP::LESSTHAN:
            scasm_src3->set_cond(scasm::cond_code::L);
            break;
          case binop::BINOP::LESSTHANEQUAL:
            scasm_src3->set_cond(scasm::cond_code::LE);
            break;
          case binop::BINOP::GREATERTHAN:
            scasm_src3->set_cond(scasm::cond_code::G);
            break;
          case binop::BINOP::GREATERTHANEQUAL:
            scasm_src3->set_cond(scasm::cond_code::GE);
            break;
          default:
            break;
          }
          scasm_inst3->set_src(std::move(scasm_src3));
          MAKE_SHARED(scasm::scasm_operand, scasm_dst3);
          SET_DST(scasm_dst3);
          scasm_inst3->set_dst(std::move(scasm_dst3));
          scasm_func->add_instruction(std::move(scasm_inst3));
        } else if (inst->get_binop() == binop::BINOP::DIV or
                   inst->get_binop() == binop::BINOP::MOD) {
          // Mov(src1, Reg(AX))
          // Cdq
          // Idiv(src2)
          // Mov(Reg(AX), dst) | Mov(Reg(DX), dst)

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
          scasm_inst->set_type(scasm::instruction_type::MOV);
          MAKE_SHARED(scasm::scasm_operand, scasm_src);
          SET_MOV_SOURCE();
          MAKE_SHARED(scasm::scasm_operand, scasm_dst);
          scasm_dst->set_type(scasm::operand_type::REG);
          scasm_dst->set_reg(scasm::register_type::AX);
          scasm_inst->set_dst(std::move(scasm_dst));
          scasm_func->add_instruction(std::move(scasm_inst));

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst2);
          scasm_inst2->set_type(scasm::instruction_type::CDQ);
          scasm_func->add_instruction(std::move(scasm_inst2));

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst3);
          scasm_inst3->set_type(scasm::instruction_type::IDIV);
          MAKE_SHARED(scasm::scasm_operand, scasm_src2);
          if (inst->get_src2()->get_type() == scar::val_type::VAR) {
            scasm_src2->set_type(scasm::operand_type::PSEUDO);
            scasm_src2->set_identifier_stack(inst->get_src2()->get_reg());
          } else if (inst->get_src2()->get_type() == scar::val_type::CONSTANT) {
            scasm_src2->set_type(scasm::operand_type::IMM);
            scasm_src2->set_imm(stoi(inst->get_src2()->get_value()));
          }
          scasm_inst3->set_src(std::move(scasm_src2));
          scasm_func->add_instruction(std::move(scasm_inst3));

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst4);
          scasm_inst4->set_type(scasm::instruction_type::MOV);
          MAKE_SHARED(scasm::scasm_operand, scasm_dst2);
          SET_DST(scasm_dst2);
          scasm_inst4->set_dst(std::move(scasm_dst2));
          MAKE_SHARED(scasm::scasm_operand, scasm_src3);
          scasm_src3->set_type(scasm::operand_type::REG);
          if (inst->get_binop() == binop::BINOP::DIV) {
            scasm_src3->set_reg(scasm::register_type::AX);
          } else {
            scasm_src3->set_reg(scasm::register_type::DX);
          }
          scasm_inst4->set_src(std::move(scasm_src3));
          scasm_func->add_instruction(std::move(scasm_inst4));
        } else if (inst->get_binop() == binop::BINOP::LEFT_SHIFT or
                   inst->get_binop() == binop::BINOP::RIGHT_SHIFT) {
          // Mov(src1,dst)
          // Mov(src2, Reg(CX))
          // Binary(binary operand, CL, dst)

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
          scasm_inst->set_type(scasm::instruction_type::MOV);
          MAKE_SHARED(scasm::scasm_operand, scasm_src);
          SET_MOV_SOURCE();
          MAKE_SHARED(scasm::scasm_operand, scasm_dst);
          if (inst->get_dst()->get_type() == scar::val_type::VAR) {
            scasm_dst->set_type(scasm::operand_type::PSEUDO);
            scasm_dst->set_identifier_stack(inst->get_dst()->get_reg());
          }
          scasm_inst->set_dst(std::move(scasm_dst));
          scasm_func->add_instruction(std::move(scasm_inst));

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst2);
          scasm_inst2->set_type(scasm::instruction_type::MOV);
          MAKE_SHARED(scasm::scasm_operand, scasm_src2);
          if (inst->get_src2()->get_type() == scar::val_type::CONSTANT) {
            scasm_src2->set_type(scasm::operand_type::IMM);
            scasm_src2->set_imm(stoi(inst->get_src2()->get_value()));
          } else if (inst->get_src2()->get_type() == scar::val_type::VAR) {
            scasm_src2->set_type(scasm::operand_type::PSEUDO);
            scasm_src2->set_identifier_stack(inst->get_src2()->get_reg());
          }
          scasm_inst2->set_src(std::move(scasm_src2));
          MAKE_SHARED(scasm::scasm_operand, scasm_dst2);
          scasm_dst2->set_type(scasm::operand_type::REG);
          scasm_dst2->set_reg(scasm::register_type::CX);
          scasm_inst2->set_dst(std::move(scasm_dst2));
          scasm_func->add_instruction(std::move(scasm_inst2));

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst3);
          scasm_inst3->set_type(scasm::instruction_type::BINARY);
          scasm_inst3->set_binop(
              scasm::scar_binop_to_scasm_binop(inst->get_binop()));
          MAKE_SHARED(scasm::scasm_operand, scasm_src3);
          scasm_src3->set_type(scasm::operand_type::REG);
          scasm_src3->set_reg(scasm::register_type::CL);
          scasm_inst3->set_src(std::move(scasm_src3));
          MAKE_SHARED(scasm::scasm_operand, scasm_dst3);
          scasm_dst3->set_type(scasm::operand_type::PSEUDO);
          scasm_dst3->set_identifier_stack(inst->get_dst()->get_reg());
          scasm_inst3->set_dst(std::move(scasm_dst3));
          scasm_func->add_instruction(std::move(scasm_inst3));

        } else {
          // Mov(src1, dst)
          // Binary(binary_operator, src2, dst)

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
          scasm_inst->set_type(scasm::instruction_type::MOV);
          MAKE_SHARED(scasm::scasm_operand, scasm_src);
          SET_MOV_SOURCE();
          MAKE_SHARED(scasm::scasm_operand, scasm_dst);
          if (inst->get_dst()->get_type() == scar::val_type::VAR) {
            scasm_dst->set_type(scasm::operand_type::PSEUDO);
            scasm_dst->set_identifier_stack(inst->get_dst()->get_reg());
          }
          scasm_inst->set_dst(std::move(scasm_dst));
          scasm_func->add_instruction(std::move(scasm_inst));

          MAKE_SHARED(scasm::scasm_instruction, scasm_inst2);
          scasm_inst2->set_type(scasm::instruction_type::BINARY);
          scasm_inst2->set_binop(
              scasm::scar_binop_to_scasm_binop(inst->get_binop()));
          MAKE_SHARED(scasm::scasm_operand, scasm_src2);
          if (inst->get_src2()->get_type() == scar::val_type::VAR) {
            scasm_src2->set_type(scasm::operand_type::PSEUDO);
            scasm_src2->set_identifier_stack(inst->get_src2()->get_reg());
          } else if (inst->get_src2()->get_type() == scar::val_type::CONSTANT) {
            scasm_src2->set_type(scasm::operand_type::IMM);
            scasm_src2->set_imm(stoi(inst->get_src2()->get_value()));
          }
          scasm_inst2->set_src(std::move(scasm_src2));
          MAKE_SHARED(scasm::scasm_operand, scasm_dst2);
          if (inst->get_dst()->get_type() == scar::val_type::VAR) {
            scasm_dst2->set_type(scasm::operand_type::PSEUDO);
            scasm_dst2->set_identifier_stack(inst->get_dst()->get_reg());
          }
          scasm_inst2->set_dst(std::move(scasm_dst2));
          scasm_func->add_instruction(std::move(scasm_inst2));
        }
      } else if (inst->get_type() == scar::instruction_type::COPY) {
        MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
        scasm_inst->set_type(scasm::instruction_type::MOV);
        MAKE_SHARED(scasm::scasm_operand, scasm_src);
        SET_MOV_SOURCE();
        MAKE_SHARED(scasm::scasm_operand, scasm_dst);
        SET_DST(scasm_dst);
        scasm_inst->set_dst(std::move(scasm_dst));
        scasm_func->add_instruction(std::move(scasm_inst));
      } else if (inst->get_type() == scar::instruction_type::LABEL) {
        MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
        scasm_inst->set_type(scasm::instruction_type::LABEL);
        MAKE_SHARED(scasm::scasm_operand, scasm_src);
        scasm_src->set_type(scasm::operand_type::LABEL);
        scasm_src->set_identifier_stack(inst->get_src1()->get_value());
        scasm_inst->set_src(std::move(scasm_src));
        scasm_func->add_instruction(std::move(scasm_inst));
      } else if (inst->get_type() == scar::instruction_type::JUMP) {
        MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
        scasm_inst->set_type(scasm::instruction_type::JMP);
        MAKE_SHARED(scasm::scasm_operand, scasm_src);
        scasm_src->set_type(scasm::operand_type::LABEL);
        scasm_src->set_identifier_stack(inst->get_src1()->get_value());
        scasm_inst->set_src(std::move(scasm_src));
        scasm_func->add_instruction(std::move(scasm_inst));
      } else if (inst->get_type() == scar::instruction_type::JUMP_IF_ZERO or
                 inst->get_type() == scar::instruction_type::JUMP_IF_NOT_ZERO) {
        // Cmp(Imm(0), condition)
        // JmpCC(E, label) | JmpCC(NE, label)
        MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
        scasm_inst->set_type(scasm::instruction_type::CMP);
        MAKE_SHARED(scasm::scasm_operand, scasm_src);
        scasm_src->set_type(scasm::operand_type::IMM);
        scasm_src->set_imm(0);
        scasm_inst->set_src(std::move(scasm_src));
        MAKE_SHARED(scasm::scasm_operand, scasm_dst);
        switch (inst->get_src1()->get_type()) {
        case scar::val_type::VAR:
          scasm_dst->set_type(scasm::operand_type::PSEUDO);
          scasm_dst->set_identifier_stack(inst->get_src1()->get_reg());
          break;
        case scar::val_type::CONSTANT:
          scasm_dst->set_type(scasm::operand_type::IMM);
          scasm_dst->set_imm(stoi(inst->get_src1()->get_value()));
          break;
        default:
          break;
        }
        scasm_inst->set_dst(std::move(scasm_dst));
        scasm_func->add_instruction(std::move(scasm_inst));

        MAKE_SHARED(scasm::scasm_instruction, scasm_inst2);
        scasm_inst2->set_type(scasm::instruction_type::JMPCC);
        MAKE_SHARED(scasm::scasm_operand, scasm_src2);
        scasm_src2->set_type(scasm::operand_type::COND);
        if (inst->get_type() == scar::instruction_type::JUMP_IF_ZERO) {
          scasm_src2->set_cond(scasm::cond_code::E);
        } else {
          scasm_src2->set_cond(scasm::cond_code::NE);
        }
        scasm_inst2->set_src(std::move(scasm_src2));
        MAKE_SHARED(scasm::scasm_operand, scasm_dst2);
        scasm_dst2->set_type(scasm::operand_type::LABEL);
        scasm_dst2->set_identifier_stack(inst->get_dst()->get_value());
        scasm_inst2->set_dst(std::move(scasm_dst2));
        scasm_func->add_instruction(std::move(scasm_inst2));
      }
    }
    scasm_program.add_function(std::move(scasm_func));
  }

  this->scasm = scasm_program;
}

#define FIX_PSEUDO(target)                                                     \
  if (NOTNULL(inst->get_##target()) &&                                         \
      inst->get_##target()->get_type() == scasm::operand_type::PSEUDO) {       \
    if (pseduo_registers.find(inst->get_##target()->get_identifier_stack()) != \
        pseduo_registers.end()) {                                              \
      inst->get_##target()->set_identifier_stack(                              \
          pseduo_registers[inst->get_##target()->get_identifier_stack()]);     \
    } else {                                                                   \
      std::string temp = inst->get_##target()->get_identifier_stack();         \
      inst->get_##target()->set_identifier_stack(                              \
          "-" + std::to_string(offset * 4) + "(%rbp)");                        \
      pseduo_registers[temp] = inst->get_##target()->get_identifier_stack();   \
      offset++;                                                                \
    }                                                                          \
    inst->get_##target()->set_type(scasm::operand_type::STACK);                \
  }

void Codegen::fix_pseudo_registers() {
  int offset = 1;
  for (auto &funcs : scasm.get_functions()) {
    for (auto &inst : funcs->get_instructions()) {
      FIX_PSEUDO(src);
      FIX_PSEUDO(dst);
    }
  }
  stack_offset = 4 * (offset - 1);
}

void Codegen::fix_instructions() {
  MAKE_SHARED(scasm::scasm_instruction, scasm_stack_instr);
  scasm_stack_instr->set_type(scasm::instruction_type::ALLOCATE_STACK);
  MAKE_SHARED(scasm::scasm_operand, val);
  val->set_type(scasm::operand_type::IMM);
  val->set_imm(stack_offset);
  scasm_stack_instr->set_src(std::move(val));

  scasm.get_functions()[0]->get_instructions().insert(
      scasm.get_functions()[0]->get_instructions().begin(),
      std::move(scasm_stack_instr));

  for (auto &funcs : scasm.get_functions()) {
    for (auto it = funcs->get_instructions().begin();
         it != funcs->get_instructions().end(); it++) {
      if (NOTNULL((*it)->get_src()) && NOTNULL((*it)->get_dst()) &&
          (*it)->get_src()->get_type() == scasm::operand_type::STACK &&
          (*it)->get_dst()->get_type() == scasm::operand_type::STACK) {
        MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
        scasm_inst->set_type(scasm::instruction_type::MOV);
        scasm_inst->set_src((*it)->get_src());

        // fixing up stack to stack move
        MAKE_SHARED(scasm::scasm_operand, dst);
        dst->set_type(scasm::operand_type::REG);
        dst->set_reg(scasm::register_type::R10);
        scasm_inst->set_dst(dst);

        (*it)->set_src(std::move(dst));
        it = funcs->get_instructions().insert(it, std::move(scasm_inst));
        it++;
      }
    }
  }

  // case when DIV uses a constant as an operand
  // case when MUL have dst as a stack value
  for (auto &funcs : scasm.get_functions()) {
    for (auto it = funcs->get_instructions().begin();
         it != funcs->get_instructions().end(); it++) {

      if ((*it)->get_type() == scasm::instruction_type::CMP and
          (*it)->get_dst()->get_type() == scasm::operand_type::IMM) {
        // cmpl stack/reg, $5
        //       |
        //       v
        // movl $5, %r11d
        // cmpl stack/reg, %r11d
        MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
        scasm_inst->set_type(scasm::instruction_type::MOV);
        scasm_inst->set_src((*it)->get_dst());
        MAKE_SHARED(scasm::scasm_operand, dst);
        dst->set_type(scasm::operand_type::REG);
        dst->set_reg(scasm::register_type::R11);
        scasm_inst->set_dst(dst);
        (*it)->set_dst(std::move(dst));
        it = funcs->get_instructions().insert(it, std::move(scasm_inst));
        it++;
      }
      if ((*it)->get_type() == scasm::instruction_type::IDIV and
          (*it)->get_src()->get_type() == scasm::operand_type::IMM) {
        // idivl $3
        //   |
        //   v
        // movl $3, %r10d
        // idivl %r10d
        MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
        scasm_inst->set_type(scasm::instruction_type::MOV);
        scasm_inst->set_src((*it)->get_src());
        MAKE_SHARED(scasm::scasm_operand, dst);
        dst->set_type(scasm::operand_type::REG);
        dst->set_reg(scasm::register_type::R10);
        scasm_inst->set_dst(dst);
        (*it)->set_src(std::move(dst));
        it = funcs->get_instructions().insert(it, std::move(scasm_inst));
        it++;
      } else if ((*it)->get_type() == scasm::instruction_type::BINARY and
                 (*it)->get_binop() == scasm::Binop::MUL and
                 (*it)->get_dst()->get_type() == scasm::operand_type::STACK) {
        // imull $3, STACK
        //        |
        //        v
        // movl STACK, %r11d
        // imull $3, %r11d
        // movl %r11d, STACK

        MAKE_SHARED(scasm::scasm_operand, dst);
        dst->set_type(scasm::operand_type::REG);
        dst->set_reg(scasm::register_type::R11);

        MAKE_SHARED(scasm::scasm_instruction, scasm_inst);
        scasm_inst->set_type(scasm::instruction_type::MOV);
        scasm_inst->set_src((*it)->get_dst());
        scasm_inst->set_dst(dst);

        MAKE_SHARED(scasm::scasm_instruction, scasm_inst2);
        scasm_inst2->set_type(scasm::instruction_type::MOV);
        scasm_inst2->set_src(dst);
        scasm_inst2->set_dst((*it)->get_dst());

        (*it)->set_dst(std::move(dst));

        it = funcs->get_instructions().insert(it, std::move(scasm_inst));
        it++;
        it = funcs->get_instructions().insert(it + 1, std::move(scasm_inst2));
      }
    }
  }
}

#define CODEGEN_SRC_DST()                                                      \
  if (instr->get_src()->get_type() == scasm::operand_type::IMM) {              \
    assembly << "$" << instr->get_src()->get_imm();                            \
  } else if (instr->get_src()->get_type() == scasm::operand_type::STACK) {     \
    assembly << instr->get_src()->get_identifier_stack();                      \
  } else if (instr->get_src()->get_type() == scasm::operand_type::REG) {       \
    assembly << scasm::to_string(instr->get_src()->get_reg());                 \
  }                                                                            \
  assembly << ", ";                                                            \
  if (instr->get_dst()->get_type() == scasm::operand_type::STACK) {            \
    assembly << instr->get_dst()->get_identifier_stack();                      \
  } else if (instr->get_dst()->get_type() == scasm::operand_type::REG) {       \
    assembly << scasm::to_string(instr->get_dst()->get_reg());                 \
  }

void Codegen::codegen() {
  // ###########################
  gen_scasm();
  fix_pseudo_registers();
  fix_instructions();
  // ###########################
  std::stringstream assembly;

  for (auto funcs : scasm.get_functions()) {
#ifdef __APPLE__
    if (funcs->get_name() == "main") {
      assembly << "\t.globl " << "_main" << "\n";
      assembly << "_main" << ":\n";
    } else {
      assembly << "\t.globl " << funcs->get_name() << "\n";
      assembly << funcs->get_name() << ":\n";
    }
#else
    assembly << "\t.globl " << funcs->get_name() << "\n";
    assembly << funcs->get_name() << ":\n";
#endif
    assembly << "\tpushq %rbp\n";
    assembly << "\tmovq %rsp, %rbp\n";
    for (auto instr : funcs->get_instructions()) {
      if (instr->get_type() == scasm::instruction_type::ALLOCATE_STACK) {
        assembly << "\tsubq $" << instr->get_src()->get_imm() << ", %rsp\n";
      } else if (instr->get_type() == scasm::instruction_type::RET) {
        assembly << "\tmovq %rbp, %rsp\n";
        assembly << "\tpopq %rbp\n";
        assembly << "\tret\n";
      } else if (instr->get_type() == scasm::instruction_type::MOV) {
        assembly << "\tmovl ";
        CODEGEN_SRC_DST();
        assembly << "\n";
      } else if (instr->get_type() == scasm::instruction_type::UNARY) {
        assembly << "\t";
        assembly << scasm::to_string(instr->get_unop()) << " ";
        if (instr->get_dst()->get_type() == scasm::operand_type::STACK) {
          assembly << instr->get_dst()->get_identifier_stack();
        } else if (instr->get_dst()->get_type() == scasm::operand_type::REG) {
          assembly << scasm::to_string(instr->get_src()->get_reg());
        }
        assembly << "\n";
      } else if (instr->get_type() == scasm::instruction_type::CDQ) {
        assembly << "\tcdq\n";
      } else if (instr->get_type() == scasm::instruction_type::IDIV) {
        assembly << "\tidivl ";
        if (instr->get_src()->get_type() == scasm::operand_type::STACK) {
          assembly << instr->get_src()->get_identifier_stack();
        } else if (instr->get_src()->get_type() == scasm::operand_type::REG) {
          assembly << scasm::to_string(instr->get_src()->get_reg());
        }
        assembly << "\n";
      } else if (instr->get_type() == scasm::instruction_type::BINARY) {
        assembly << "\t";
        assembly << scasm::to_string(instr->get_binop()) << " ";
        CODEGEN_SRC_DST();
        assembly << "\n";
      } else if (instr->get_type() == scasm::instruction_type::JMP) {
#ifdef __APPLE__
        assembly << "\tjmp " << "L" << instr->get_src()->get_identifier_stack()
                 << "\n";
#else
        assembly << "\tjmp " << ".L" << instr->get_src()->get_identifier_stack()
                 << "\n";
#endif
      } else if (instr->get_type() == scasm::instruction_type::LABEL) {
#ifdef __APPLE__
        assembly << "L" << instr->get_src()->get_identifier_stack() << ":\n";
#else
        assembly << ".L" << instr->get_src()->get_identifier_stack() << ":\n";
#endif
      } else if (instr->get_type() == scasm::instruction_type::CMP) {
        assembly << "\tcmpl ";
        CODEGEN_SRC_DST();
        assembly << "\n";
      } else if (instr->get_type() == scasm::instruction_type::JMPCC) {
        assembly << "\tj";
        assembly << scasm::to_string(instr->get_src()->get_cond()) << " ";
#ifdef __APPLE__
        assembly << "L" << instr->get_dst()->get_identifier_stack() << "\n";
#else
        assembly << ".L" << instr->get_dst()->get_identifier_stack() << "\n";
#endif
      } else if (instr->get_type() == scasm::instruction_type::SETCC) {
        assembly << "\tset";
        assembly << scasm::to_string(instr->get_src()->get_cond()) << " ";
        if (instr->get_dst()->get_type() == scasm::operand_type::STACK) {
          assembly << instr->get_dst()->get_identifier_stack();
        } else if (instr->get_dst()->get_type() == scasm::operand_type::REG) {
          assembly << scasm::to_string(instr->get_dst()->get_reg(), true);
        }
        assembly << "\n";
      }
    }
  }
#ifndef __APPLE__
  assembly << "\t.section    .note.GNU-stack,\"\",@progbits\n";
#endif

  std::ofstream file(file_name);
  if (file.is_open()) {
    file << assembly.str();
    file.close();
  } else {
    std::cerr << "[ERROR]: Unable to open file" << std::endl;
    success = false;
  }
}
} // namespace codegen
} // namespace scarlet
