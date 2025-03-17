#include <codegen/codegen.hh>

namespace scarlet {
namespace codegen {

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
          if (factor->get_type() == ast::FactorType::FUNCTION_CALL) {
            gen_scar_factor_function_call(
                std::static_pointer_cast<ast::AST_factor_function_call_Node>(
                    factor),
                scar_function);
            scar_val_dst->set_reg_name(variable_buffer);
            variable_buffer.clear();
          } else {
            scar_val_dst->set_reg_name(
                factor->get_identifier_node()->get_value());
          }
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
          if (factor->get_type() == ast::FactorType::FUNCTION_CALL) {
            gen_scar_factor_function_call(
                std::static_pointer_cast<ast::AST_factor_function_call_Node>(
                    factor),
                scar_function);
            scar_val_src->set_reg_name(variable_buffer);
            variable_buffer.clear();
          } else {
            scar_val_src->set_reg_name(
                factor->get_identifier_node()->get_value());
          }
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
          if (factor->get_type() == ast::FactorType::FUNCTION_CALL) {
            gen_scar_factor_function_call(
                std::static_pointer_cast<ast::AST_factor_function_call_Node>(
                    factor),
                scar_function);
            scar_val_src->set_reg_name(variable_buffer);
            variable_buffer.clear();
          } else {
            scar_val_src->set_reg_name(
                factor->get_identifier_node()->get_value());
          }
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
    // NOTE: It is guaranteed that the factor node will have either an int node,
    //       an identifier node or a function call

    // save constant for later use
    if (factor->get_int_node() != nullptr and
        !factor->get_int_node()->get_value().empty()) {
      constant_buffer = factor->get_int_node()->get_value();
    }

    // save variable for later use
    if (factor->get_identifier_node() != nullptr and
        !factor->get_identifier_node()->get_value().empty()) {
      if (factor->get_type() == ast::FactorType::FUNCTION_CALL) {
        gen_scar_factor_function_call(
            std::static_pointer_cast<ast::AST_factor_function_call_Node>(
                factor),
            scar_function);
      } else {
        variable_buffer = factor->get_identifier_node()->get_value();
      }
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
      scar_val->set_value(constant_buffer);
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

  std::string dstReg = get_reg_name();

  variable_buffer = dstReg;
  MAKE_SHARED(scar::scar_Val_Node, scar_val_dst);
  scar_val_dst->set_type(scar::val_type::VAR);
  scar_val_dst->set_reg_name(dstReg);
  scar_instruction->set_dst(scar_val_dst);

  scar_function->add_instruction(scar_instruction);
}

} // namespace codegen
} // namespace scarlet
