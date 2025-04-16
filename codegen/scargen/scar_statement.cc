#include <codegen/common.hh>

namespace scarlet {
namespace codegen {

void Codegen::gen_scar_statement(
    std::shared_ptr<ast::AST_Statement_Node> statement,
    std::shared_ptr<scar::scar_Function_Node> scar_function) {
  switch (statement->get_type()) {
  case ast::statementType::NULLSTMT:
    break;
  case ast::statementType::SWITCH: {
    auto switch_statement =
        std::static_pointer_cast<ast::AST_switch_statement_Node>(statement);
    MAKE_SHARED(scar::scar_Instruction_Node, default_jump);
    gen_scar_exp(switch_statement->get_exps(), scar_function);
    MAKE_SHARED(scar::scar_Val_Node, scar_val_src);
    SETVARCONSTANTREG(scar_val_src);
    for (auto case_exp_label : switch_statement->get_case_exp_label()) {
      // copy the switch exp to be useable later
      auto switch_exp_pointer = scar_val_src;
      if (case_exp_label.first != nullptr) {
        MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction);
        scar_instruction->set_type(scar::instruction_type::BINARY);
        scar_instruction->set_src1(switch_exp_pointer);
        MAKE_SHARED(scar::scar_Val_Node, scar_val_src2);
        gen_scar_exp(case_exp_label.first, scar_function);
        SETVARCONSTANTREG(scar_val_src2);
        scar_instruction->set_src2(scar_val_src2);
        scar_instruction->set_binop(binop::EQUAL);
        MAKE_SHARED(scar::scar_Val_Node, scar_val_dst);
        scar_val_dst->set_type(scar::val_type::VAR);
        scar_val_dst->set_reg_name(get_reg_name(ast::ElemType::INT, {}));
        scar_instruction->set_dst(scar_val_dst);
        scar_function->add_instruction(scar_instruction);

        MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction2);
        scar_instruction2->set_type(scar::instruction_type::JUMP_IF_NOT_ZERO);
        MAKE_SHARED(scar::scar_Val_Node, scar_val_src3);
        scar_val_src3->set_type(scar::val_type::VAR);
        scar_val_src3->set_reg_name(get_prev_reg_name());
        scar_instruction2->set_src1(scar_val_src3);
        MAKE_SHARED(scar::scar_Val_Node, scar_val_dst2);
        scar_val_dst2->set_type(scar::val_type::LABEL);
        scar_val_dst2->set_label(case_exp_label.second->get_value());
        scar_instruction2->set_dst(scar_val_dst2);
        scar_function->add_instruction(scar_instruction2);
      } else {
        default_jump->set_type(scar::instruction_type::JUMP);
        MAKE_SHARED(scar::scar_Val_Node, scar_val_src2);
        scar_val_src2->set_type(scar::val_type::LABEL);
        scar_val_src2->set_label(case_exp_label.second->get_value());
        default_jump->set_src1(scar_val_src2);
      }
    }
    if (switch_statement->get_has_default_case()) {
      scar_function->add_instruction(default_jump);
    }
    // jump to the end
    MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction3);
    scar_instruction3->set_type(scar::instruction_type::JUMP);
    MAKE_SHARED(scar::scar_Val_Node, scar_val_src3);
    scar_val_src3->set_type(scar::val_type::LABEL);
    scar_val_src3->set_label(
        switch_statement->get_labels().second->get_value());
    scar_instruction3->set_src1(scar_val_src3);
    scar_function->add_instruction(scar_instruction3);

    gen_scar_statement(switch_statement->get_stmt(), scar_function);

    // generate the end label
    MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction4);
    scar_instruction4->set_type(scar::instruction_type::LABEL);
    MAKE_SHARED(scar::scar_Val_Node, scar_val_src4);
    scar_val_src4->set_type(scar::val_type::LABEL);
    scar_val_src4->set_label(
        switch_statement->get_labels().second->get_value());
    scar_instruction4->set_src1(scar_val_src4);
    scar_function->add_instruction(scar_instruction4);

  } break;
  case ast::statementType::RETURN: {
    gen_scar_exp(statement->get_exps(), scar_function);
    MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction);
    scar_instruction->set_type(scar::instruction_type::RETURN);
    if (statement->get_exps() != nullptr) {
      MAKE_SHARED(scar::scar_Val_Node, scar_val_src);
      SETVARCONSTANTREG(scar_val_src);
      scar_instruction->set_src1(scar_val_src);
    }
    scar_function->add_instruction(scar_instruction);
  } break;
  case ast::statementType::EXP: {
    gen_scar_exp(statement->get_exps(), scar_function);
    variable_buffer.clear();
    constant_buffer.clear();
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
    scar_val_dst->set_type(scar::val_type::LABEL);
    scar_val_dst->set_label(if_statement->get_labels().first->get_value());
    scar_instruction->set_dst(std::move(scar_val_dst));
    scar_function->add_instruction(std::move(scar_instruction));

    // generate scar for the statement
    gen_scar_statement(if_statement->get_stmt1(), scar_function);

    // generate the end label
    MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction2);
    scar_instruction2->set_type(scar::instruction_type::LABEL);
    MAKE_SHARED(scar::scar_Val_Node, scar_val_src2);
    scar_val_src2->set_type(scar::val_type::LABEL);
    scar_val_src2->set_label(if_statement->get_labels().first->get_value());
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
    scar_val_dst->set_type(scar::val_type::LABEL);
    scar_val_dst->set_label(if_else_statement->get_labels().first->get_value());
    scar_instruction->set_dst(std::move(scar_val_dst));
    scar_function->add_instruction(std::move(scar_instruction));

    // generate scar for the if statement
    gen_scar_statement(if_else_statement->get_stmt1(), scar_function);

    // jump to the end
    MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction2);
    scar_instruction2->set_type(scar::instruction_type::JUMP);
    MAKE_SHARED(scar::scar_Val_Node, scar_val_src2);
    scar_val_src2->set_type(scar::val_type::LABEL);
    scar_val_src2->set_label(
        if_else_statement->get_labels().second->get_value());
    scar_instruction2->set_src1(std::move(scar_val_src2));
    scar_function->add_instruction(std::move(scar_instruction2));

    // generate the else label
    MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction3);
    scar_instruction3->set_type(scar::instruction_type::LABEL);
    MAKE_SHARED(scar::scar_Val_Node, scar_val_src3);
    scar_val_src3->set_type(scar::val_type::LABEL);
    scar_val_src3->set_label(
        if_else_statement->get_labels().first->get_value());
    scar_instruction3->set_src1(std::move(scar_val_src3));
    scar_function->add_instruction(std::move(scar_instruction3));

    // generate scar for the else statement
    gen_scar_statement(if_else_statement->get_stmt2(), scar_function);

    // generate the end label
    MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction4);
    scar_instruction4->set_type(scar::instruction_type::LABEL);
    MAKE_SHARED(scar::scar_Val_Node, scar_val_src4);
    scar_val_src4->set_type(scar::val_type::LABEL);
    scar_val_src4->set_label(
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
    scar_val_src->set_type(scar::val_type::LABEL);
    scar_val_src->set_label(statement->get_labels().first->get_value());
    scar_instruction->set_src1(std::move(scar_val_src));
    scar_function->add_instruction(scar_instruction);
  } break;
  case ast::statementType::CASE:
  case ast::statementType::DEFAULT_CASE:
  case ast::statementType::LABEL: {
    MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction);
    scar_instruction->set_type(scar::instruction_type::LABEL);
    MAKE_SHARED(scar::scar_Val_Node, scar_val_src);
    scar_val_src->set_type(scar::val_type::LABEL);
    scar_val_src->set_label(statement->get_labels().first->get_value());
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
    scar_val_src->set_type(scar::val_type::LABEL);
    scar_val_src->set_label(while_statement->get_labels().first->get_value());
    scar_instruction->set_src1(std::move(scar_val_src));
    scar_function->add_instruction(scar_instruction);

    gen_scar_exp(while_statement->get_exps(), scar_function);

    MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction2);
    scar_instruction2->set_type(scar::instruction_type::JUMP_IF_ZERO);
    MAKE_SHARED(scar::scar_Val_Node, scar_val_src2);
    SETVARCONSTANTREG(scar_val_src2);
    scar_instruction2->set_src1(std::move(scar_val_src2));
    MAKE_SHARED(scar::scar_Val_Node, scar_val_dst2);
    scar_val_dst2->set_type(scar::val_type::LABEL);
    scar_val_dst2->set_label(while_statement->get_labels().second->get_value());
    scar_instruction2->set_dst(std::move(scar_val_dst2));
    scar_function->add_instruction(scar_instruction2);

    gen_scar_statement(while_statement->get_stmt(), scar_function);

    MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction3);
    scar_instruction3->set_type(scar::instruction_type::JUMP);
    MAKE_SHARED(scar::scar_Val_Node, scar_val_src3);
    scar_val_src3->set_type(scar::val_type::LABEL);
    scar_val_src3->set_label(while_statement->get_labels().first->get_value());
    scar_instruction3->set_src1(std::move(scar_val_src3));
    scar_function->add_instruction(scar_instruction3);

    MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction4);
    scar_instruction4->set_type(scar::instruction_type::LABEL);
    MAKE_SHARED(scar::scar_Val_Node, scar_val_src4);
    scar_val_src4->set_type(scar::val_type::LABEL);
    scar_val_src4->set_label(while_statement->get_labels().second->get_value());
    scar_instruction4->set_src1(std::move(scar_val_src4));
    scar_function->add_instruction(scar_instruction4);
  } break;

  case ast::statementType::DO_WHILE: {
    auto do_while_statement =
        std::static_pointer_cast<ast::AST_while_statement_Node>(statement);

    // Label(start)
    // <instructions for statement>
    // Label(continue)
    // <instructions for condition>
    // c = <result of condition>
    // JumpIfNotZero(c, start)
    // Label(end)

    MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction);
    scar_instruction->set_type(scar::instruction_type::LABEL);
    MAKE_SHARED(scar::scar_Val_Node, scar_val_src);
    scar_val_src->set_type(scar::val_type::LABEL);
    scar_val_src->set_label(do_while_statement->get_start_label());
    scar_instruction->set_src1(std::move(scar_val_src));
    scar_function->add_instruction(scar_instruction);

    gen_scar_statement(do_while_statement->get_stmt(), scar_function);

    MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction1);
    scar_instruction1->set_type(scar::instruction_type::LABEL);
    MAKE_SHARED(scar::scar_Val_Node, scar_val_src1);
    scar_val_src1->set_type(scar::val_type::LABEL);
    scar_val_src1->set_label(
        do_while_statement->get_labels().first->get_value());
    scar_instruction1->set_src1(std::move(scar_val_src1));
    scar_function->add_instruction(scar_instruction1);

    gen_scar_exp(do_while_statement->get_exps(), scar_function);

    MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction2);
    scar_instruction2->set_type(scar::instruction_type::JUMP_IF_NOT_ZERO);
    MAKE_SHARED(scar::scar_Val_Node, scar_val_src2);
    SETVARCONSTANTREG(scar_val_src2);
    scar_instruction2->set_src1(std::move(scar_val_src2));
    MAKE_SHARED(scar::scar_Val_Node, scar_val_dst2);
    scar_val_dst2->set_type(scar::val_type::LABEL);
    scar_val_dst2->set_label(do_while_statement->get_start_label());
    scar_instruction2->set_dst(std::move(scar_val_dst2));
    scar_function->add_instruction(scar_instruction2);

    MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction3);
    scar_instruction3->set_type(scar::instruction_type::LABEL);
    MAKE_SHARED(scar::scar_Val_Node, scar_val_src3);
    scar_val_src3->set_type(scar::val_type::LABEL);
    scar_val_src3->set_label(
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

    MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction);
    scar_instruction->set_type(scar::instruction_type::LABEL);
    MAKE_SHARED(scar::scar_Val_Node, scar_val_src);
    scar_val_src->set_type(scar::val_type::LABEL);
    scar_val_src->set_label(for_statement->get_start_label());
    scar_instruction->set_src1(std::move(scar_val_src));
    scar_function->add_instruction(scar_instruction);

    gen_scar_exp(for_statement->get_exps(), scar_function);

    if (for_statement->get_exps() != nullptr) {
      MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction2);
      scar_instruction2->set_type(scar::instruction_type::JUMP_IF_ZERO);
      MAKE_SHARED(scar::scar_Val_Node, scar_val_src2);
      SETVARCONSTANTREG(scar_val_src2);
      scar_instruction2->set_src1(std::move(scar_val_src2));
      MAKE_SHARED(scar::scar_Val_Node, scar_val_dst2);
      scar_val_dst2->set_type(scar::val_type::LABEL);
      scar_val_dst2->set_label(for_statement->get_labels().second->get_value());
      scar_instruction2->set_dst(std::move(scar_val_dst2));
      scar_function->add_instruction(scar_instruction2);
    }

    gen_scar_statement(for_statement->get_stmt(), scar_function);

    MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction3);
    scar_instruction3->set_type(scar::instruction_type::LABEL);
    MAKE_SHARED(scar::scar_Val_Node, scar_val_src3);
    scar_val_src3->set_type(scar::val_type::LABEL);
    scar_val_src3->set_label(for_statement->get_labels().first->get_value());
    scar_instruction3->set_src1(std::move(scar_val_src3));
    scar_function->add_instruction(scar_instruction3);

    gen_scar_exp(for_statement->get_exp2(), scar_function);

    MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction4);
    scar_instruction4->set_type(scar::instruction_type::JUMP);
    MAKE_SHARED(scar::scar_Val_Node, scar_val_src4);
    scar_val_src4->set_type(scar::val_type::LABEL);
    scar_val_src4->set_label(for_statement->get_start_label());
    scar_instruction4->set_src1(std::move(scar_val_src4));
    scar_function->add_instruction(scar_instruction4);

    MAKE_SHARED(scar::scar_Instruction_Node, scar_instruction5);
    scar_instruction5->set_type(scar::instruction_type::LABEL);
    MAKE_SHARED(scar::scar_Val_Node, scar_val_src5);
    scar_val_src5->set_type(scar::val_type::LABEL);
    scar_val_src5->set_label(for_statement->get_labels().second->get_value());
    scar_instruction5->set_src1(std::move(scar_val_src5));
    scar_function->add_instruction(scar_instruction5);
  } break;
  case ast::statementType::UNKNOWN:
    UNREACHABLE()
  }
}

} // namespace codegen
} // namespace scarlet
