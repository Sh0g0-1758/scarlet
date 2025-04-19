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
    result.set_value({.c = (char)(src1.get_value().c op src2.get_value().c)}); \
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
    result.set_value({.c = (char)(src1.get_value().c op src2.get_value().c)}); \
    break;                                                                     \
  case constant::Type::UCHAR:                                                  \
    result.set_value(                                                          \
        {.uc = (unsigned char)(src1.get_value().uc op src2.get_value().uc)});  \
    break;                                                                     \
  default:                                                                     \
    break;                                                                     \
  }

#define CALC_BINOP_CMP(src1, src2, op)                                         \
  result.set_type(constant::Type::INT);                                        \
  switch (src1.get_type()) {                                                   \
  case constant::Type::INT:                                                    \
    result.set_value({.i = src1.get_value().i op src2.get_value().i});         \
    break;                                                                     \
  case constant::Type::UINT:                                                   \
    result.set_value({.i = src1.get_value().ui op src2.get_value().ui});       \
    break;                                                                     \
  case constant::Type::LONG:                                                   \
    result.set_value({.i = src1.get_value().l op src2.get_value().l});         \
    break;                                                                     \
  case constant::Type::ULONG:                                                  \
    result.set_value({.i = src1.get_value().ul op src2.get_value().ul});       \
    break;                                                                     \
  case constant::Type::DOUBLE:                                                 \
    result.set_value({.i = src1.get_value().d op src2.get_value().d});         \
    break;                                                                     \
  case constant::Type::CHAR:                                                   \
    result.set_value({.i = src1.get_value().c op src2.get_value().c});         \
    break;                                                                     \
  case constant::Type::UCHAR:                                                  \
    result.set_value({.i = src1.get_value().uc op src2.get_value().uc});       \
    break;                                                                     \
  default:                                                                     \
    break;                                                                     \
  }

#define IS_ZERO(src, flag)                                                     \
  switch (src.get_type()) {                                                    \
  case constant::Type::INT:                                                    \
    if (src.get_value().i == 0)                                                \
      flag = true;                                                             \
    break;                                                                     \
  case constant::Type::UINT:                                                   \
    if (src.get_value().ui == 0)                                               \
      flag = true;                                                             \
    break;                                                                     \
  case constant::Type::LONG:                                                   \
    if (src.get_value().l == 0)                                                \
      flag = true;                                                             \
    break;                                                                     \
  case constant::Type::ULONG:                                                  \
    if (src.get_value().ul == 0)                                               \
      flag = true;                                                             \
    break;                                                                     \
  case constant::Type::DOUBLE:                                                 \
    if (src.get_value().d == 0.0 or src.get_value().d == -0.0)                 \
      flag = true;                                                             \
    break;                                                                     \
  case constant::Type::CHAR:                                                   \
    if (src.get_value().c == 0)                                                \
      flag = true;                                                             \
    break;                                                                     \
  case constant::Type::UCHAR:                                                  \
    if (src.get_value().uc == 0)                                               \
      flag = true;                                                             \
    break;                                                                     \
  default:                                                                     \
    break;                                                                     \
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

#define CALC_UNOP_NOT(src, op)                                                 \
  result.set_type(constant::Type::INT);                                        \
  switch (src.get_type()) {                                                    \
  case constant::Type::INT:                                                    \
    result.set_value({.i = op src.get_value().i});                             \
    break;                                                                     \
  case constant::Type::UINT:                                                   \
    result.set_value({.i = op src.get_value().ui});                            \
    break;                                                                     \
  case constant::Type::LONG:                                                   \
    result.set_value({.i = op src.get_value().l});                             \
    break;                                                                     \
  case constant::Type::ULONG:                                                  \
    result.set_value({.i = op src.get_value().ul});                            \
    break;                                                                     \
  case constant::Type::DOUBLE:                                                 \
    result.set_value({.i = op src.get_value().d});                             \
    break;                                                                     \
  case constant::Type::CHAR:                                                   \
    result.set_value({.i = op src.get_value().c});                             \
    break;                                                                     \
  case constant::Type::UCHAR:                                                  \
    result.set_value({.i = op src.get_value().uc});                            \
    break;                                                                     \
  default:                                                                     \
    break;                                                                     \
  }

#define CALC_TYPECAST(src, result, type, t)                                    \
  switch (src.get_type()) {                                                    \
  case constant::Type::INT:                                                    \
    result.set_value({.t = (type)src.get_value().i});                          \
    break;                                                                     \
  case constant::Type::UINT:                                                   \
    result.set_value({.t = (type)src.get_value().ui});                         \
    break;                                                                     \
  case constant::Type::LONG:                                                   \
    result.set_value({.t = (type)src.get_value().l});                          \
    break;                                                                     \
  case constant::Type::ULONG:                                                  \
    result.set_value({.t = (type)src.get_value().ul});                         \
    break;                                                                     \
  case constant::Type::DOUBLE:                                                 \
    result.set_value({.t = (type)src.get_value().d});                          \
    break;                                                                     \
  case constant::Type::CHAR:                                                   \
    result.set_value({.t = (type)src.get_value().c});                          \
    break;                                                                     \
  case constant::Type::UCHAR:                                                  \
    result.set_value({.t = (type)src.get_value().uc});                         \
    break;                                                                     \
  default:                                                                     \
    break;                                                                     \
  }

void Codegen::fold_binop(constant::Constant src1, constant::Constant src2,
                         constant::Constant &result, binop::BINOP op) {
  result.set_type(src1.get_type());
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
  case binop::BINOP::DIV: {
    bool divisionByZero{};
    IS_ZERO(src2, divisionByZero);
    if (divisionByZero) {
      result.set_value({.i = 0});
    } else {
      CALC_BINOP(src1, src2, /);
    }
  } break;
  case binop::BINOP::MOD: {
    bool ModByZero{};
    IS_ZERO(src2, ModByZero);
    if (ModByZero) {
      result.set_value({.i = 0});
    } else {
      CALC_BINOP_INT(src1, src2, %);
    }
  } break;
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
  case binop::BINOP::EQUAL:
    CALC_BINOP_CMP(src1, src2, ==);
    break;
  case binop::BINOP::NOTEQUAL:
    CALC_BINOP_CMP(src1, src2, !=);
    break;
  case binop::BINOP::LESSTHAN:
    CALC_BINOP_CMP(src1, src2, <);
    break;
  case binop::BINOP::GREATERTHAN:
    CALC_BINOP_CMP(src1, src2, >);
    break;
  case binop::BINOP::LESSTHANEQUAL:
    CALC_BINOP_CMP(src1, src2, <=);
    break;
  case binop::BINOP::GREATERTHANEQUAL:
    CALC_BINOP_CMP(src1, src2, >=);
    break;
  default:
    break;
  }
}

void Codegen::fold_unop(constant::Constant src, constant::Constant &result,
                        unop::UNOP op) {
  result.set_type(src.get_type());
  switch (op) {
  case unop::UNOP::NEGATE:
    CALC_UNOP(src, -);
    break;
  case unop::UNOP::NOT:
    result.set_type(constant::Type::INT);
    CALC_UNOP_NOT(src, !);
    break;
  case unop::UNOP::COMPLEMENT:
    CALC_UNOP_INT(src, ~);
    break;
  default:
    break;
  }
}

void Codegen::fold_typecast(constant::Constant src,
                            constant::Constant &result) {
  switch (result.get_type()) {
  case constant::Type::DOUBLE:
    CALC_TYPECAST(src, result, double, d);
    break;
  case constant::Type::INT:
    CALC_TYPECAST(src, result, int, i);
    break;
  case constant::Type::UINT:
    CALC_TYPECAST(src, result, unsigned int, ui);
    break;
  case constant::Type::LONG:
    CALC_TYPECAST(src, result, long, l);
    break;
  case constant::Type::ULONG:
    CALC_TYPECAST(src, result, unsigned long, ul);
    break;
  case constant::Type::CHAR:
    CALC_TYPECAST(src, result, char, c);
    break;
  case constant::Type::UCHAR:
    CALC_TYPECAST(src, result, unsigned char, uc);
    break;
  default:
    break;
  }
}

bool Codegen::constant_folding(std::vector<cfg::node> &cfg) {
  bool isChanged{};
  for (auto block = cfg.begin(); block != cfg.end(); ++block) {
    if (block->is_empty())
      continue;
    for (auto inst = block->get_body().begin(); inst != block->get_body().end();
         ++inst) {
      auto instType = (*inst)->get_type();
      if (instType == scar::instruction_type::BINARY) {
        if (IS_CONSTANT((*inst)->get_src1()) and
            IS_CONSTANT((*inst)->get_src2())) {
          isChanged = true;
          constant::Constant result;
          fold_binop((*inst)->get_src1()->get_const_val(),
                     (*inst)->get_src2()->get_const_val(), result,
                     (*inst)->get_binop());
          (*inst)->set_type(scar::instruction_type::COPY);
          (*inst)->get_src1()->set_const_val(result);
          (*inst)->set_src2(nullptr);
        }
      } else if (instType == scar::instruction_type::UNARY) {
        if (IS_CONSTANT((*inst)->get_src1())) {
          isChanged = true;
          constant::Constant result;
          fold_unop((*inst)->get_src1()->get_const_val(), result,
                    (*inst)->get_unop());
          (*inst)->set_type(scar::instruction_type::COPY);
          (*inst)->get_src1()->set_const_val(result);
        }
      } else if (instType == scar::instruction_type::JUMP_IF_ZERO) {
        if (IS_CONSTANT((*inst)->get_src1())) {
          isChanged = true;
          bool modify{};
          IS_ZERO((*inst)->get_src1()->get_const_val(), modify);
          if (modify) {
            (*inst)->set_type(scar::instruction_type::JUMP);
            (*inst)->set_src1((*inst)->get_dst());
            for (auto succID : block->get_succ()) {
              getNodeFromID(cfg, succID).remove_pred(block->get_id());
            }
            block->get_succ().clear();
            block->add_succ(NodeLabelToId[(*inst)->get_src1()->get_label()]);
          } else {
            inst = block->get_body().erase(inst);
            --inst;
            if (!block->get_body().empty()) {
              for (auto succID : block->get_succ()) {
                getNodeFromID(cfg, succID).remove_pred(block->get_id());
              }
              block->get_succ().clear();
              block->add_succ((block + 1)->get_id());
            }
          }
        }
      } else if (instType == scar::instruction_type::JUMP_IF_NOT_ZERO) {
        if (IS_CONSTANT((*inst)->get_src1())) {
          isChanged = true;
          bool erase{};
          IS_ZERO((*inst)->get_src1()->get_const_val(), erase);
          if (erase) {
            inst = block->get_body().erase(inst);
            --inst;
            if (!block->get_body().empty()) {
              for (auto succID : block->get_succ()) {
                getNodeFromID(cfg, succID).remove_pred(block->get_id());
              }
              block->get_succ().clear();
              block->add_succ((block + 1)->get_id());
            }
          } else {
            (*inst)->set_type(scar::instruction_type::JUMP);
            (*inst)->set_src1((*inst)->get_dst());
            for (auto succID : block->get_succ()) {
              getNodeFromID(cfg, succID).remove_pred(block->get_id());
            }
            block->get_succ().clear();
            block->add_succ(NodeLabelToId[(*inst)->get_src1()->get_label()]);
          }
        }
      } else if (scar::is_type_cast(instType)) {
        if (IS_CONSTANT((*inst)->get_src1())) {
          isChanged = true;
          constant::Constant result;
          result.set_type(scarValTypeToConstType((*inst)->get_dst()));
          fold_typecast((*inst)->get_src1()->get_const_val(), result);
          (*inst)->set_type(scar::instruction_type::COPY);
          (*inst)->get_src1()->set_const_val(result);
        }
      } else if (instType == scar::instruction_type::COPY) {
        auto dstType = scarValTypeToConstType((*inst)->get_dst());
        if (IS_CONSTANT((*inst)->get_src1()) and
            dstType != (*inst)->get_src1()->get_const_val().get_type()) {
          isChanged = true;
          (*inst)->get_src1()->get_const_val().set_type(dstType);
        }
      }
    }
    if (block->get_body().empty()) {
      REMOVE_BLOCK();
    }
  }
  return isChanged;
}

} // namespace codegen
} // namespace scarlet
