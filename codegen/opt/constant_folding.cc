#include <codegen/common.hh>

namespace scarlet {
namespace codegen {

#define IS_CONSTANT(operand) operand->get_type() == scar::val_type::CONSTANT

#define CALC_BINOP(src1, src2, op)                                             \
  switch (src1.get_type()) {                                                   \
  case constant::Type::INT:                                                    \
    result.set_value({.i = src1.get_value().i op src2.get_value().i});         \
    break;                                                                     \
  case constant::Type::UINT:                                                   \
    result.set_value({.ui = src1.get_value().ui op src2.get_value().ui});      \
    break;                                                                     \
  case constant::Type::LONG:                                                   \
    result.set_value({.l = src1.get_value().l op src2.get_value().l});         \
    break;                                                                     \
  case constant::Type::ULONG:                                                  \
    result.set_value({.ul = src1.get_value().ul op src2.get_value().ul});      \
    break;                                                                     \
  case constant::Type::DOUBLE:                                                 \
    result.set_value({.d = src1.get_value().d op src2.get_value().d});         \
    break;                                                                     \
  case constant::Type::CHAR:                                                   \
    result.set_value(                                                          \
        {.c = (char)(src1.get_value().c op(int) src2.get_value().c)});         \
    break;                                                                     \
  case constant::Type::UCHAR:                                                  \
    result.set_value(                                                          \
        {.uc = (unsigned char)(src1.get_value().uc op src2.get_value().uc)});  \
    break;                                                                     \
  default:                                                                     \
    break;                                                                     \
  }

#define CALC_BINOP_INT(src1, src2, op)                                         \
  switch (src1.get_type()) {                                                   \
  case constant::Type::INT:                                                    \
    result.set_value({.i = src1.get_value().i op src2.get_value().i});         \
    break;                                                                     \
  case constant::Type::UINT:                                                   \
    result.set_value({.ui = src1.get_value().ui op src2.get_value().ui});      \
    break;                                                                     \
  case constant::Type::LONG:                                                   \
    result.set_value({.l = src1.get_value().l op src2.get_value().l});         \
    break;                                                                     \
  case constant::Type::ULONG:                                                  \
    result.set_value({.ul = src1.get_value().ul op src2.get_value().ul});      \
    break;                                                                     \
  case constant::Type::CHAR:                                                   \
    result.set_value(                                                          \
        {.c = (char)(src1.get_value().c op(int) src2.get_value().c)});         \
    break;                                                                     \
  case constant::Type::UCHAR:                                                  \
    result.set_value(                                                          \
        {.uc = (unsigned char)(src1.get_value().uc op src2.get_value().uc)});  \
    break;                                                                     \
  default:                                                                     \
    break;                                                                     \
  }

void Codegen::fold_binop(constant::Constant src1, constant::Constant src2,
                         constant::Constant &result, binop::BINOP op) {
  switch (op) {
  case binop::BINOP::ADD:
    CALC_BINOP(src1, src2, +);
    break;
  case binop::BINOP::SUB:
    CALC_BINOP(src1, src2, -);
    break;
  case binop::BINOP::MUL:
    CALC_BINOP(src1, src2, *);
    break;
  case binop::BINOP::DIV:
    CALC_BINOP(src1, src2, /);
    break;
  case binop::BINOP::MOD:
    CALC_BINOP_INT(src1, src2, %);
    break;
  case binop::BINOP::AAND:
    CALC_BINOP_INT(src1, src2, &);
    break;
  case binop::BINOP::AOR:
    CALC_BINOP_INT(src1, src2, |);
    break;
  case binop::BINOP::XOR:
    CALC_BINOP_INT(src1, src2, ^);
    break;
  case binop::BINOP::LOGICAL_LEFT_SHIFT:
  case binop::BINOP::LEFT_SHIFT:
    CALC_BINOP_INT(src1, src2, <<);
    break;
  case binop::BINOP::LOGICAL_RIGHT_SHIFT:
  case binop::BINOP::RIGHT_SHIFT:
    CALC_BINOP_INT(src1, src2, >>);
    break;
  case binop::BINOP::LAND:
    CALC_BINOP_INT(src1, src2, &&);
    break;
  case binop::BINOP::LOR:
    CALC_BINOP_INT(src1, src2, ||);
    break;
  case binop::BINOP::EQUAL:
    CALC_BINOP_INT(src1, src2, ==);
    break;
  case binop::BINOP::NOTEQUAL:
    CALC_BINOP_INT(src1, src2, !=);
    break;
  case binop::BINOP::LESSTHAN:
    CALC_BINOP_INT(src1, src2, <);
    break;
  case binop::BINOP::GREATERTHAN:
    CALC_BINOP_INT(src1, src2, >);
    break;
  case binop::BINOP::LESSTHANEQUAL:
    CALC_BINOP_INT(src1, src2, <=);
    break;
  case binop::BINOP::GREATERTHANEQUAL:
    CALC_BINOP_INT(src1, src2, >=);
    break;
  default:
    break;
  }
}

#define CALC_UNOP(src, op)                                                     \
  switch (src.get_type()) {                                                    \
  case constant::Type::INT:                                                    \
    result.set_value({.i = op src.get_value().i});                             \
    break;                                                                     \
  case constant::Type::UINT:                                                   \
    result.set_value({.ui = op src.get_value().ui});                           \
    break;                                                                     \
  case constant::Type::LONG:                                                   \
    result.set_value({.l = op src.get_value().l});                             \
    break;                                                                     \
  case constant::Type::ULONG:                                                  \
    result.set_value({.ul = op src.get_value().ul});                           \
    break;                                                                     \
  case constant::Type::DOUBLE:                                                 \
    result.set_value({.d = op src.get_value().d});                             \
    break;                                                                     \
  case constant::Type::CHAR:                                                   \
    result.set_value({.c = (char)(op src.get_value().c)});                     \
    break;                                                                     \
  case constant::Type::UCHAR:                                                  \
    result.set_value({.uc = (unsigned char)(op src.get_value().uc)});          \
    break;                                                                     \
  default:                                                                     \
    break;                                                                     \
  }

#define CALC_UNOP_INT(src, op)                                                 \
  switch (src.get_type()) {                                                    \
  case constant::Type::INT:                                                    \
    result.set_value({.i = op src.get_value().i});                             \
    break;                                                                     \
  case constant::Type::UINT:                                                   \
    result.set_value({.ui = op src.get_value().ui});                           \
    break;                                                                     \
  case constant::Type::LONG:                                                   \
    result.set_value({.l = op src.get_value().l});                             \
    break;                                                                     \
  case constant::Type::ULONG:                                                  \
    result.set_value({.ul = op src.get_value().ul});                           \
    break;                                                                     \
  case constant::Type::CHAR:                                                   \
    result.set_value({.c = (char)(op src.get_value().c)});                     \
    break;                                                                     \
  case constant::Type::UCHAR:                                                  \
    result.set_value({.uc = (unsigned char)(op src.get_value().uc)});          \
    break;                                                                     \
  default:                                                                     \
    break;                                                                     \
  }

void Codegen::fold_unop(constant::Constant src, constant::Constant &result,
                        unop::UNOP op) {
  switch (op) {
  case unop::UNOP::NEGATE:
    CALC_UNOP(src, -);
    break;
  case unop::UNOP::NOT:
    CALC_UNOP_INT(src, !);
    break;
  case unop::UNOP::COMPLEMENT:
    CALC_UNOP_INT(src, ~);
    break;
  default:
    break;
  }
}

void Codegen::constant_folding(
    std::vector<std::shared_ptr<scar::scar_Instruction_Node>> &funcBody) {
  for (auto &inst : funcBody) {
    if (inst->get_type() == scar::instruction_type::BINARY) {
      if (IS_CONSTANT(inst->get_src1()) and IS_CONSTANT(inst->get_src2())) {
        constant::Constant result;
        fold_binop(inst->get_src1()->get_const_val(),
                   inst->get_src2()->get_const_val(), result,
                   inst->get_binop());
        inst->set_type(scar::instruction_type::COPY);
        inst->get_src1()->set_const_val(result);
        inst->set_src2(nullptr);
      }
    } else if (inst->get_type() == scar::instruction_type::UNARY) {
      if (IS_CONSTANT(inst->get_src1())) {
        constant::Constant result;
        fold_unop(inst->get_src1()->get_const_val(), result, inst->get_unop());
        inst->set_type(scar::instruction_type::COPY);
        inst->get_src1()->set_const_val(result);
      }
    } else if (inst->get_type() == scar::instruction_type::JUMP_IF_ZERO) {
      if (IS_CONSTANT(inst->get_src1())) {
        bool modify = false;
        auto src = inst->get_src1()->get_const_val();
        switch (src.get_type()) {
        case constant::Type::INT:
          modify = src.get_value().i == 0;
          break;
        case constant::Type::UINT:
          modify = src.get_value().ui == 0;
          break;
        case constant::Type::LONG:
          modify = src.get_value().l == 0;
          break;
        case constant::Type::ULONG:
          modify = src.get_value().ul == 0;
          break;
        case constant::Type::DOUBLE:
          modify = src.get_value().d == 0;
          break;
        case constant::Type::CHAR:
          modify = src.get_value().c == 0;
          break;
        case constant::Type::UCHAR:
          modify = src.get_value().uc == 0;
          break;
        default:
          break;
        }
        if (modify) {
          inst->set_type(scar::instruction_type::JUMP);
          inst->set_src1(inst->get_dst());
        }
      }
    } else if (inst->get_type() == scar::instruction_type::JUMP_IF_NOT_ZERO) {
      if (IS_CONSTANT(inst->get_src1())) {
        bool modify = false;
        auto src = inst->get_src1()->get_const_val();
        switch (src.get_type()) {
        case constant::Type::INT:
          modify = src.get_value().i != 0;
          break;
        case constant::Type::UINT:
          modify = src.get_value().ui != 0;
          break;
        case constant::Type::LONG:
          modify = src.get_value().l != 0;
          break;
        case constant::Type::ULONG:
          modify = src.get_value().ul != 0;
          break;
        case constant::Type::DOUBLE:
          modify = src.get_value().d != 0;
          break;
        case constant::Type::CHAR:
          modify = src.get_value().c != 0;
          break;
        case constant::Type::UCHAR:
          modify = src.get_value().uc != 0;
          break;
        default:
          break;
        }
        if (modify) {
          inst->set_type(scar::instruction_type::JUMP);
          inst->set_src1(inst->get_dst());
        }
      }
    }
  }
}

} // namespace codegen
} // namespace scarlet
