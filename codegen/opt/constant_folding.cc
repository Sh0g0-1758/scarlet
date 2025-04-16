#include <codegen/common.hh>

namespace scarlet {
namespace codegen {

#define IS_CONSTANT(operand) operand->get_type() == scar::val_type::CONSTANT

#define CALC(src1, src2, op)                                                   \
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

#define CALC_INT(src1, src2, op)                                               \
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

void Codegen::constant_folding(
    std::vector<std::shared_ptr<scar::scar_Instruction_Node>> &funcBody) {
  for (auto &inst : funcBody) {
    if (inst->get_type() == scar::instruction_type::BINARY) {
      if (IS_CONSTANT(inst->get_src1()) and IS_CONSTANT(inst->get_src2())) {
        auto src1 = inst->get_src1()->get_const_val();
        auto src2 = inst->get_src2()->get_const_val();
        constant::Constant result;
        switch (inst->get_binop()) {
        case binop::BINOP::ADD:
          CALC(src1, src2, +);
          break;
        case binop::BINOP::SUB:
          CALC(src1, src2, -);
          break;
        case binop::BINOP::MUL:
          CALC(src1, src2, *);
          break;
        case binop::BINOP::DIV:
          CALC(src1, src2, /);
          break;
        case binop::BINOP::MOD:
          CALC_INT(src1, src2, %);
          break;
        case binop::BINOP::AAND:
          CALC_INT(src1, src2, &);
          break;
        case binop::BINOP::AOR:
          CALC_INT(src1, src2, |);
          break;
        case binop::BINOP::XOR:
          CALC_INT(src1, src2, ^);
          break;
        case binop::BINOP::LOGICAL_LEFT_SHIFT:
        case binop::BINOP::LEFT_SHIFT:
          CALC_INT(src1, src2, <<);
          break;
        case binop::BINOP::LOGICAL_RIGHT_SHIFT:
        case binop::BINOP::RIGHT_SHIFT:
          CALC_INT(src1, src2, >>);
          break;
        case binop::BINOP::LAND:
          CALC_INT(src1, src2, &&);
          break;
        case binop::BINOP::LOR:
          CALC_INT(src1, src2, ||);
          break;
        case binop::BINOP::EQUAL:
          CALC_INT(src1, src2, ==);
          break;
        case binop::BINOP::NOTEQUAL:
          CALC_INT(src1, src2, !=);
          break;
        case binop::BINOP::LESSTHAN:
          CALC_INT(src1, src2, <);
          break;
        case binop::BINOP::GREATERTHAN:
          CALC_INT(src1, src2, >);
          break;
        case binop::BINOP::LESSTHANEQUAL:
          CALC_INT(src1, src2, <=);
          break;
        case binop::BINOP::GREATERTHANEQUAL:
          CALC_INT(src1, src2, >=);
          break;
        default:
          break;
        }
        inst->set_type(scar::instruction_type::COPY);
        inst->get_src1()->set_const_val(result);
        inst->set_src2(nullptr);
      }
    }
  }
}

} // namespace codegen
} // namespace scarlet
