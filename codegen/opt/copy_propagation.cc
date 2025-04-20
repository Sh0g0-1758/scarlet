#include <codegen/common.hh>

namespace scarlet {
namespace codegen {

#define SET_COPY_FROM_VAL(src, copy)                                           \
  if (src->get_type() == scar::val_type::VAR) {                                \
    copy.set_reg_name(src->get_reg());                                         \
    copy.set_copy_type(scar::val_type::VAR);                                   \
  } else if (src->get_type() == scar::val_type::CONSTANT) {                    \
    copy.set_const_val(src->get_const_val());                                  \
    copy.set_copy_type(scar::val_type::CONSTANT);                              \
  }

/*******************************************************************************
 * NOTE: We make a new scar_Val_Node instead of changing the original one
 *       because we have made extensive reuse of scar nodes in scargen. So
 *       changes here can have unintended affects in other scar instructions.
 ******************************************************************************/
#define SET_VAL_FROM_COPY(src, set_src)                                        \
  if (src != nullptr and src->get_type() == scar::val_type::VAR and            \
      copy_map.find(src->get_reg()) != copy_map.end()) {                       \
    ran_copy_propagation = true;                                               \
    auto newVal = copy_map[src->get_reg()];                                    \
    MAKE_SHARED(scar::scar_Val_Node, newSrc);                                  \
    if (newVal.get_copy_type() == scar::val_type::VAR) {                       \
      newSrc->set_type(scar::val_type::VAR);                                   \
      newSrc->set_reg_name(newVal.get_reg_name());                             \
    } else if (newVal.get_copy_type() == scar::val_type::CONSTANT) {           \
      newSrc->set_type(scar::val_type::CONSTANT);                              \
      newSrc->set_const_val(newVal.get_const_val());                           \
    }                                                                          \
    (*it)->set_src(newSrc);                                                    \
  }

#define INVALIDATE_COPY_IF_DST(copy_map)                                       \
  {                                                                            \
    /* If x = y in copy map and instruction is op(...|x), remove x = y */      \
    if (copy_map.find(dst->get_reg()) != copy_map.end()) {                     \
      copy_map.erase(dst->get_reg());                                          \
    }                                                                          \
                                                                               \
    /* If x = y in copy map and instruction is op(...|y), remove x = y */      \
    for (auto cpy = copy_map.begin(); cpy != copy_map.end();) {                \
      if (cpy->second.get_copy_type() == scar::val_type::VAR and               \
          cpy->second.get_reg_name() == dst->get_reg()) {                      \
        cpy = copy_map.erase(cpy);                                             \
      } else {                                                                 \
        cpy++;                                                                 \
      }                                                                        \
    }                                                                          \
  }

void Codegen::transfer_copies(cfg::node &block) {
  for (auto instr : block.get_body()) {
    auto src = instr->get_src1();
    auto dst = instr->get_dst();
    if (instr->get_type() == scar::instruction_type::COPY) {
      // If x = y in copy map and instruction is COPY(x, y), ignore
      if (src->get_type() == scar::val_type::VAR and
          block.copy_map.find(src->get_reg()) != block.copy_map.end()) {
        if (block.copy_map[src->get_reg()].get_copy_type() ==
                scar::val_type::VAR and
            block.copy_map[src->get_reg()].get_reg_name() == dst->get_reg()) {
          continue;
        }
      }

      // If x = y in copy map and instruction is COPY(y, x), ignore
      if (block.copy_map.find(dst->get_reg()) != block.copy_map.end()) {
        auto copySrc = block.copy_map[dst->get_reg()];
        if ((src->get_type() == scar::val_type::CONSTANT and
             copySrc.get_copy_type() == scar::val_type::CONSTANT and
             src->get_const_val() == copySrc.get_const_val()) or
            (src->get_type() == scar::val_type::VAR and
             copySrc.get_copy_type() == scar::val_type::VAR and
             src->get_reg() == copySrc.get_reg_name())) {
          continue;
        }
      }

      // If x = y in copy map and instruction is COPY(.|x), remove x = y
      if (block.copy_map.find(dst->get_reg()) != block.copy_map.end()) {
        block.copy_map.erase(dst->get_reg());
      }

      // If x = y in copy map and instruction is COPY(.|y), remove x = y
      for (auto cpy = block.copy_map.begin(); cpy != block.copy_map.end();) {
        if (cpy->second.get_copy_type() == scar::val_type::VAR and
            cpy->second.get_reg_name() == dst->get_reg()) {
          cpy = block.copy_map.erase(cpy);
        } else {
          cpy++;
        }
      }

      // add current copy
      // make sure to add copies only when src and dst have same signedness
      if (scarValTypeToConstType(src) == scarValTypeToConstType(dst)) {
        cfg::copy_info copy_val;
        SET_COPY_FROM_VAL(src, copy_val);
        block.copy_map[dst->get_reg()] = copy_val;
      }
    } else if (instr->get_type() == scar::instruction_type::CALL) {
      // remove copies with aliased variables
      for (auto cpy = block.copy_map.begin(); cpy != block.copy_map.end();) {
        if (aliased_vars[cpy->first]) {
          cpy = block.copy_map.erase(cpy);
        } else if (cpy->second.get_copy_type() == scar::val_type::VAR and
                   aliased_vars[cpy->second.get_reg_name()]) {
          cpy = block.copy_map.erase(cpy);
        } else {
          cpy++;
        }
      }

      if (dst != nullptr)
        INVALIDATE_COPY_IF_DST(block.copy_map);
    } else if (instr->get_type() == scar::instruction_type::STORE) {
      // invalidate copies that contain aliased variables
      for (auto cpy = block.copy_map.begin(); cpy != block.copy_map.end();) {
        if (aliased_vars[cpy->first]) {
          cpy = block.copy_map.erase(cpy);
        } else if (cpy->second.get_copy_type() == scar::val_type::VAR and
                   aliased_vars[cpy->second.get_reg_name()]) {
          cpy = block.copy_map.erase(cpy);
        } else {
          cpy++;
        }
      }
    } else {
      if (dst != nullptr)
        INVALIDATE_COPY_IF_DST(block.copy_map);
    }
  }
}

std::map<std::string, cfg::copy_info>
Codegen::merge_copies(std::vector<cfg::node> &cfg, cfg::node &block) {
  if (block.get_pred().empty()) {
    return {};
  }

  auto initMap = getNodeFromID(cfg, block.get_pred()[0]).copy_map;
  for (int i = 1; i < (int)block.get_pred().size(); i++) {
    auto predMap = getNodeFromID(cfg, block.get_pred()[i]).copy_map;
    for (auto it = initMap.begin(); it != initMap.end();) {
      if (predMap.find(it->first) != predMap.end() and
          it->second == predMap[it->first]) {
        it++;
      } else {
        it = initMap.erase(it);
      }
    }
  }
  return initMap;
}

bool Codegen::copy_propagation(std::vector<cfg::node> &cfg) {
  bool ran_copy_propagation{};
  // store all copies in the cfg as a provisional result to be able to
  // process blocks whose predecessors have not been processed yet.
  std::map<std::string, cfg::copy_info> all_copies;
  for (auto block : cfg) {
    if (block.is_empty())
      continue;
    for (auto instr : block.get_body()) {
      if (instr->get_type() == scar::instruction_type::COPY) {
        cfg::copy_info copy_val;
        // make sure to add copies only when src and dst have same signedness
        if (scarValTypeToConstType(instr->get_src1()) ==
            scarValTypeToConstType(instr->get_dst())) {
          SET_COPY_FROM_VAL(instr->get_src1(), copy_val);
          all_copies[instr->get_dst()->get_reg()] = copy_val;
        }
      }
    }
  }

  std::queue<unsigned int> worklist;
  std::map<unsigned int, bool> worklistMap;
  for (auto &block : cfg) {
    if (block.is_empty())
      continue;
    block.copy_map = all_copies;
    worklist.push(block.get_id());
    worklistMap[block.get_id()] = true;
  }

  // Reaching Copies Analysis
  while (!worklist.empty()) {
    auto &block = getNodeFromID(cfg, worklist.front());
    worklistMap[worklist.front()] = false;
    worklist.pop();
    auto old_copy_map = block.copy_map;
    block.copy_map = merge_copies(cfg, block);
    transfer_copies(block);
    auto new_copy_map = block.copy_map;
    if (old_copy_map != new_copy_map) {
      for (auto succID : block.get_succ()) {
        if (succID == cfg.size() - 1)
          continue;
        if (worklistMap[succID])
          continue;
        worklist.push(succID);
        worklistMap[succID] = true;
      }
    }
  }

  // propagate copies
  for (auto block = cfg.begin(); block != cfg.end(); ++block) {
    if (block->is_empty())
      continue;
    auto copy_map = merge_copies(cfg, *block);
    for (auto it = block->get_body().begin(); it != block->get_body().end();) {
      auto src = (*it)->get_src1();
      auto dst = (*it)->get_dst();
      if ((*it)->get_type() == scar::instruction_type::COPY) {
        // if x = y in copy map and instruction is COPY(x, y), remove
        // instruction
        if (src->get_type() == scar::val_type::VAR and
            copy_map.find(src->get_reg()) != copy_map.end()) {
          if (copy_map[src->get_reg()].get_copy_type() ==
                  scar::val_type::VAR and
              copy_map[src->get_reg()].get_reg_name() == dst->get_reg()) {
            ran_copy_propagation = true;
            it = block->get_body().erase(it);
            continue;
          }
        }

        // If x = y in copy map and instruction is COPY(y, x), remove
        // instruction
        if (copy_map.find(dst->get_reg()) != copy_map.end()) {
          auto copySrc = copy_map[dst->get_reg()];
          if ((src->get_type() == scar::val_type::CONSTANT and
               copySrc.get_copy_type() == scar::val_type::CONSTANT and
               src->get_const_val() == copySrc.get_const_val()) or
              (src->get_type() == scar::val_type::VAR and
               copySrc.get_copy_type() == scar::val_type::VAR and
               src->get_reg() == copySrc.get_reg_name())) {
            ran_copy_propagation = true;
            it = block->get_body().erase(it);
            continue;
          }
        }

        // If instruction is COPY(x,x), remove instruction
        if (src->get_type() == scar::val_type::VAR and
            dst->get_type() == scar::val_type::VAR and
            src->get_reg() == dst->get_reg()) {
          ran_copy_propagation = true;
          it = block->get_body().erase(it);
          continue;
        }

        // if x = y in copy map and instruction is COPY(x|.), replace x with y
        SET_VAL_FROM_COPY(src, set_src1);

        // If x = y in copy map and instruction is COPY(.|x), remove x = y
        if (copy_map.find(dst->get_reg()) != copy_map.end()) {
          copy_map.erase(dst->get_reg());
        }

        // If x = y in copy map and instruction is COPY(.|y), remove x = y
        for (auto cpy = copy_map.begin(); cpy != copy_map.end();) {
          if (cpy->second.get_copy_type() == scar::val_type::VAR and
              cpy->second.get_reg_name() == dst->get_reg()) {
            cpy = copy_map.erase(cpy);
          } else {
            cpy++;
          }
        }

        // add current copy
        // make sure to add copies only when src and dst have same signedness
        if (scarValTypeToConstType(src) == scarValTypeToConstType(dst)) {
          cfg::copy_info copy_val;
          SET_COPY_FROM_VAL(src, copy_val);
          copy_map[dst->get_reg()] = copy_val;
        }
      } else if ((*it)->get_type() == scar::instruction_type::CALL) {
        // if x = y in copy map and instruction is funcall(..|x|..), x -> y
        auto funCall =
            std::static_pointer_cast<scar::scar_FunctionCall_Instruction_Node>(
                *it);
        for (int i = 0; i < (int)funCall->get_args().size(); i++) {
          auto arg = funCall->get_args()[i];
          if (arg != nullptr and arg->get_type() == scar::val_type::VAR and
              copy_map.find(arg->get_reg()) != copy_map.end()) {
            ran_copy_propagation = true;
            auto newVal = copy_map[arg->get_reg()];
            MAKE_SHARED(scar::scar_Val_Node, newSrc);
            if (newVal.get_copy_type() == scar::val_type::VAR) {
              newSrc->set_type(scar::val_type::VAR);
              newSrc->set_reg_name(newVal.get_reg_name());
            } else if (newVal.get_copy_type() == scar::val_type::CONSTANT) {
              newSrc->set_type(scar::val_type::CONSTANT);
              newSrc->set_const_val(newVal.get_const_val());
            }
            funCall->get_args()[i] = newSrc;
          }
        }

        // remove copies with aliased variables
        for (auto cpy = copy_map.begin(); cpy != copy_map.end();) {
          if (aliased_vars[cpy->first]) {
            cpy = copy_map.erase(cpy);
          } else if (cpy->second.get_copy_type() == scar::val_type::VAR and
                     aliased_vars[cpy->second.get_reg_name()]) {
            cpy = copy_map.erase(cpy);
          } else {
            cpy++;
          }
        }

        if (dst != nullptr)
          INVALIDATE_COPY_IF_DST(copy_map);
      } else if ((*it)->get_type() == scar::instruction_type::STORE) {
        // invalidate copies that contain aliased variables
        for (auto cpy = copy_map.begin(); cpy != copy_map.end();) {
          if (aliased_vars[cpy->first]) {
            cpy = copy_map.erase(cpy);
          } else if (cpy->second.get_copy_type() == scar::val_type::VAR and
                     aliased_vars[cpy->second.get_reg_name()]) {
            cpy = copy_map.erase(cpy);
          } else {
            cpy++;
          }
        }

        // If x = y in copy map and instruction is op(x|.), replace x with y
        SET_VAL_FROM_COPY(src, set_src1);

        // If x = y in copy map and instruction is op(.|x), replace x with y
        SET_VAL_FROM_COPY(dst, set_dst);
      } else if ((*it)->get_type() == scar::instruction_type::GET_ADDRESS) {
        // NOTE: Do not tamper with the source of get address because
        // COPY(0, a)
        // GETADDRESS(a, b)
        // can get changed to GETADDRESS(0, b)
        if (dst != nullptr)
          INVALIDATE_COPY_IF_DST(copy_map);
      } else {
        // If x = y in copy map and instruction is op(x|...), replace x with y
        SET_VAL_FROM_COPY(src, set_src1);

        // If x = y in copy map and instruction is op(.|x|.), replace x with y
        auto src2 = (*it)->get_src2();
        SET_VAL_FROM_COPY(src2, set_src2);

        if (dst != nullptr)
          INVALIDATE_COPY_IF_DST(copy_map);
      }
      it++;
    }
    if (block->get_body().empty()) {
      REMOVE_BLOCK();
    }
  }
  return ran_copy_propagation;
}

} // namespace codegen
} // namespace scarlet
