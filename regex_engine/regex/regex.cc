#include "regex.hh"
#include "../ast/ast.hh"
#include "../lexer/lexer.hh"
#include <algorithm>
#include <bits/stdc++.h>
#include <cassert>
#include <deque>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

using namespace std;

// a function to print the children of an AST in a more visual manner
void RegexEngine::printChildren(const shared_ptr<ASTNode> &node) {
  if (auto reNode = dynamic_pointer_cast<RE>(node)) {
    cout << "RE Node Children: " << reNode->children.size() << endl;
    for (const auto &child : reNode->children) {
      cout << "  Child Type: " << typeid(*child).name() << endl;
    }
  } else if (auto groupNode = dynamic_pointer_cast<GroupNode>(node)) {
    cout << "GroupNode Children: " << groupNode->children.size() << endl;
    for (const auto &child : groupNode->children) {
      cout << "  Child Type: " << typeid(*child).name() << endl;
    }
  } else if (auto orNode = dynamic_pointer_cast<OrNode>(node)) {
    cout << "OrNode Children: " << orNode->children.size() << endl;
    for (const auto &child : orNode->children) {
      cout << "  Child Type: " << typeid(*child).name() << endl;
    }
  } else if (auto leafNode = dynamic_pointer_cast<LeafNode>(node)) {
    cout << "LeafNode has no children." << endl;
  } else {
    cout << "Unknown node type." << endl;
  }
}

// a function to print  an AST in a more visual manner
void RegexEngine::printAST(const shared_ptr<ASTNode> &node, int depth) {
  if (!node)
    return;

  string indent(depth * 2, ' ');

  cout << indent << "Node Type: " << typeid(*node).name() << endl;

  if (auto ele = dynamic_cast<Element *>(node.get())) {
    cout << indent << "  Match: " << ele->match << endl;

    if (holds_alternative<int>(ele->min)) {
      cout << indent << "  Min (int): " << get<int>(ele->min) << endl;
    } else if (holds_alternative<double>(ele->min)) {
      cout << indent << "  Min (double): " << get<double>(ele->min) << endl;
    }

    if (holds_alternative<int>(ele->max)) {
      cout << indent << "  Max (int): " << get<int>(ele->max) << endl;
    } else if (holds_alternative<double>(ele->max)) {
      cout << indent << "  Max (double): " << get<double>(ele->max) << endl;
    }
    return;

  } else if (auto group = dynamic_cast<GroupNode *>(node.get())) {
    cout << indent << "  Group Name: " << group->group_name << endl;
    cout << indent << "  Group ID: " << group->group_id << endl;
    cout << indent << "  Capturing: " << (group->is_capturing() ? "Yes" : "No")
         << endl;

    if (holds_alternative<int>(group->min)) {
      cout << indent << "  Min (int): " << get<int>(group->min) << endl;
    } else if (holds_alternative<double>(group->min)) {
      cout << indent << "  Min (double): " << get<double>(group->min) << endl;
    }

    if (holds_alternative<int>(group->max)) {
      cout << indent << "  Max (int): " << get<int>(group->max) << endl;
    } else if (holds_alternative<double>(group->max)) {
      cout << indent << "  Max (double): " << get<double>(group->max) << endl;
    }

  } else if (auto range = dynamic_cast<RangeElement *>(node.get())) {
    cout << indent << "  Match Range: " << range->match << endl;
    cout << indent
         << "  Positive Logic: " << (range->is_positive_logic ? "Yes" : "No")
         << endl;

    if (holds_alternative<int>(range->min)) {
      cout << indent << "  Min (int): " << get<int>(range->min) << endl;
    } else if (holds_alternative<double>(range->min)) {
      cout << indent << "  Min (double): " << get<double>(range->min) << endl;
    }

    if (holds_alternative<int>(range->max)) {
      cout << indent << "  Max (int): " << get<int>(range->max) << endl;
    } else if (holds_alternative<double>(range->max)) {
      cout << indent << "  Max (double): " << get<double>(range->max) << endl;
    }

  } else if (auto reNode = dynamic_cast<RE *>(node.get())) {
    cout << indent << "  Group Name: " << reNode->group_name << endl;
    cout << indent << "  Capturing: " << (reNode->is_capturing() ? "Yes" : "No")
         << endl;
  } else if (auto orNode = dc<OrNode *>(node.get())) {
    if (holds_alternative<int>(orNode->min)) {
      cout << indent << "  Min (int): " << get<int>(orNode->min) << endl;
    } else if (holds_alternative<double>(orNode->min)) {
      cout << indent << "  Min (double): " << get<double>(orNode->min) << endl;
    }

    if (holds_alternative<int>(orNode->max)) {
      cout << indent << "  Max (int): " << get<int>(orNode->max) << endl;
    } else if (holds_alternative<double>(orNode->max)) {
      cout << indent << "  Max (double): " << get<double>(orNode->max) << endl;
    }
  }

  // Recursively print children for nodes with children
  if (auto group = dynamic_cast<GroupNode *>(node.get())) {
    if (!group->children.empty()) {
      cout << indent << "  Children:" << endl;
      for (const auto &child : group->children) {
        printAST(child, depth + 1);
      }
    }
  } else if (auto orNode = dynamic_cast<OrNode *>(node.get())) {
    cout << indent << "  Children:" << endl;
    if (!orNode->children.empty()) {
      for (const auto &child : orNode->children) {
        printAST(child, depth + 1);
      }
    }

  } else if (auto reNode = dynamic_cast<RE *>(node.get())) {
    if (!reNode->children.empty()) {
      cout << indent << "  Children:" << endl;
      for (const auto &child : reNode->children) {
        printAST(child, depth + 1);
      }
    }
  } else {
    cout << indent << "  No children for this node type." << endl;
  }
}

//******************************************************************************************************************************************************
//

tuple<bool, int, vector<deque<Match>>>
RegexEngine::match(const string &re, const string &input_string,
                   bool return_matches, bool continue_after_match) {
  /**
   * Searches a regular expression within a test string.
   *
   * This method searches for the specified regular expression within the
   * provided test string and returns the result. It can be customized to alter
   * the search behavior and the returned value.
   *
   *
   * @param re The regular expression to search (string).
   * @param test_string The string to be searched (string).
   * @param return_matches If `true`, returns a data structure containing the
   *                       matches, including the whole match and any matched
   *                       groups or subgroups. Default is `false`.
   * @param continue_after_match If `true`, the search continues after the first
   *                             match until the entire input string is
   * consumed. Default is `false`.
   *
   * @return A `variant` containing one of the following tuples:
   *         - `(bool, int)`:
   *           - `bool`: Indicates whether a match was found.
   *           - `int`: The index of the last matched character.
   *         - `(bool, int, vector<deque<Match>>)`:
   *           - `bool`: Indicates whether a match was found.
   *           - `int`: The index of the last matched character.
   *           - `vector<deque<Match>>`: Contains all matches:
   *               - The first position represents the whole match.
   *               - Subsequent positions represent all matched groups and
   *                 subgroups.
   *
   * @throws runtime_error If an error occurs during regex compilation or
   *         execution.
   */

  // An inline lambda function to return in the necessary format
  auto give_back = [](bool result, int chars_consumed,
                      const vector<deque<Match>> &all_matches,
                      bool return_matches) {
    if (return_matches) {

      return make_tuple(result, chars_consumed, all_matches);

    } else {

      return make_tuple(result, chars_consumed, vector<deque<Match>>());
    }
  };

  // cout<<"match: Starting match function"<<endl;
  string normalized_re = re;
  string normalized_string = input_string;

  Parser parser;
  // cout<<"match: Parsing regex into AST"<<endl;

  shared_ptr<RE> ast = (prev_re != re) ? parser.parse(normalized_re) : prev_ast;
  // cout<<"match: AST parsed successfully"<<endl;
  // printAST(ast);
  prev_re = re;
  prev_ast = ast;
  // Uses cached results to avoid re-parsing if the regex hasn't changed.

  // cout<<"match: Starting initial match attempt"<<endl;
  auto [result, chars_consumed, matches] =
      matchInternal(ast, normalized_string, 0);
  // cout<<"match: Initial match result: res= "<<result<< "chars_consumed =
  // "<<chars_consumed<<endl;
  int highest_matched_idx = 0;
  vector<deque<Match>> all_matches;

  if (result) {
    // cout<<"match: Initial match successful"<<endl;
    highest_matched_idx = chars_consumed;
    all_matches.push_back(matches);
  } else {
    // cout<<"match: Initial match failed"<<endl;
    return give_back(result, highest_matched_idx, all_matches, return_matches);
  }

  if (!continue_after_match || chars_consumed <= 0) {
    // cout<<"match: Stopping after initial match (continue_after_match is False
    // or no characters consumed)"<<endl;
    return give_back(result, highest_matched_idx, all_matches, return_matches);
  }

  // cout<<"match: continue_after_match is True, continuing to match"<<endl;

  while (true) {
    // replaced 0 with consumed below, as now we start checking from last
    // consumed index

    // cout<<"match: Starting new match attempt from index
    // "<<chars_consumed<<endl;
    auto [new_result, new_chars_consumed, new_matches] =
        matchInternal(ast, normalized_string, chars_consumed);
    // cout<<"match: Match result: res= "<< result<<" , chars_consumed=
    // "<<chars_consumed<<endl;

    // if consumed is not grrater than highest_matched_idx this means the new
    // match consumed 0 characters, so there is really nothing more to match
    if (new_result && new_chars_consumed > highest_matched_idx) {
      // cout<<"match: New match found, updating highest_matched_idx to
      // "<<chars_consumed<<endl;
      highest_matched_idx = new_chars_consumed;
      all_matches.push_back(new_matches);
      chars_consumed = new_chars_consumed;
    } else {
      // If no new match, exit the loop and return results.
      // cout<<"match: No new match found, stopping"<<endl;
      return give_back(true, highest_matched_idx, all_matches, return_matches);
    }
  }
}

//*******************************************************************************************************************************************************//

tuple<bool, int, deque<Match>>
RegexEngine::matchInternal(shared_ptr<RE> ast, const string &input_string,
                           int start_str_i) {
  /* A more specific match function which works in core */

  deque<Match> matches;
  Match *last_match =
      nullptr; // used to restore the left match of a or-node (if necessary)

  // `max_matched_idx` represents the "upper limit" of the match.
  // It is essential when backtracking in cases with nested quantifiers,
  // as it provides a mechanism to signal the group that is causing the
  // failure due to excessive greediness to stop earlier if possible.
  int max_matched_idx = -1;

  // `str_i` represents the characters matched so far. It is initialized to the
  // value of the input parameter `start_str_i` because the match may need to
  // start at an index other than 0. For example, this can occur when the
  // function is invoked to search for a second match in the test string.
  int str_i = start_str_i;

  //*****************************************************************************************************************
  //*/ a returm function just like above
  auto give_back = [&matches](bool result, int str_i) {
    return make_tuple(result, str_i, matches);
  };

  //*****************************************************************************************************************
  //*/
  /**
   * This lambda function saves matches for capturing groups during regex
   * evaluation. It invokes the `match_group` function to attempt a match for
   * the specified group (`ast`) in the input string starting from the given
   * index. If the match is successful and the group is a capturing group, it
   * checks for existing matches with the same group ID in the `matches` list,
   * removes them if found, and adds a new `Match` object with the group ID,
   * start and end indices, matched substring, and group name. The function
   * ensures that the most recent match for each group is saved. It returns a
   * tuple containing a boolean indicating match success and the ending index of
   * the match. Non-capturing groups do not modify the `matches` list, and
   * `max_matched_idx` optionally limits the match extent for backtracking or
   * nested quantifiers.
   * */

  auto save_matches =
      [&matches, &last_match](
          const function<tuple<bool, int>(shared_ptr<ASTNode>, const string &,
                                          int)> &match_group,
          shared_ptr<ASTNode> ast, const string &input_string, int start_idx,
          int max_matched_idx =
              -1) { // cout<<"save_matches: save_matches called with ast = "<<
                    // typeid(*ast).name() <<" , start_idx = "<<start_idx<<" ,
                    // max_matched_idx "<<max_matched_idx<<endl;
        auto [result, end_idx] =
            match_group(ast, input_string, max_matched_idx);
        // cout << "save_matches: match_group result: res=" << result << ",
        // end_idx=" << end_idx << endl;
        if (dc<GroupNode *>(ast.get()) &&
            dc<GroupNode *>(ast.get())->is_capturing() && result) {
          // cout << "save_matches: Capturing group (ID=" <<
          // dc<GroupNode*>(ast.get())->group_id<< ", Name=" <<
          // dc<GroupNode*>(ast.get())->group_name<< ") matched successfully."
          // << endl;
          for (auto it = matches.begin(); it != matches.end(); ++it) {
            if (it->group_id == dc<GroupNode *>(ast.get())->group_id) {
              // cout << "save_matches: Found existing match with group ID "<<
              // dc<GroupNode*>(ast.get())->group_id << ". Removing it." <<
              // endl;
              last_match = &(*it);
              // cout << "save_matches: Removed match: group_id=" <<
              // last_match->group_id<<endl;
              matches.erase(it);
              break;
            }
          }
          auto gn = dc<GroupNode *>(ast.get());
          // cout<<"save_matches: "<<input_string<<endl;
          matches.push_front(Match(gn->group_id, start_idx, end_idx,
                                   input_string, gn->group_name));
          // cout << "save_matches: Added new match to the front: group_id=" <<
          // gn->group_id<<endl;
        }
        // cout<<"not in if statement"<<endl;
        return make_tuple(result, end_idx);
      };

  //*****************************************************************************************************************
  //*/

  auto remove_from_left = [&matches]() { matches.pop_front(); };

  //*****************************************************************************************************************
  //*/

  auto add_to_left = [&matches, &last_match]() {
    matches.push_front(*last_match);
  };

  //*****************************************************************************************************************
  //*/

  function<tuple<bool, int>(shared_ptr<ASTNode>, const string &, int)>
      match_group;

  match_group = [&](shared_ptr<ASTNode> ast, const string &input_string,
                    int max_matched_idx = -1) {
    auto re_node = dynamic_cast<RE *>(ast.get());
    shared_ptr<ASTNode> curr_node = (re_node && !re_node->children.empty())
                                        ? re_node->children[0]
                                        : nullptr;

    int i = 0;

    //*****************************************************************************************************************
    //*/
    vector<tuple<int, int, int, vector<int>>> backtrack_stack;
    // (child_index, min_matches, matched_times, consumed_lengths):
    // - `child_index`: Index of the current child node being processed.
    // - `min_matches`: Minimum number of matches required for this node.
    // - `matched_times`: Number of times this node has successfully matched so
    // far.
    // - `consumed_lengths`: List of character counts consumed during each
    // match.

    function<tuple<bool, int, int>(int, int, bool)> backtrack;

    backtrack = [&](int str_i, int curr_child_i,
                    bool recursive = false) -> tuple<bool, int, int> {
      /**
       * Determines whether backtracking is possible and provides the state to
       * backtrack to.
       *
       * This function evaluates the current state of the engine to decide if
       * backtracking can occur. It returns a flag indicating backtracking
       * feasibility and, if possible, the updated string index and child node
       * index to backtrack to.
       *
       * @param str_i The current index being considered in the test string.
       * @param curr_child_i The index of the `GroupNode` child currently being
       * evaluated.
       *
       * @return A tuple containing:
       *         - `bool`: True if backtracking is possible.
       *         - `int`: The new string index to backtrack to (valid only if
       * backtracking is possible).
       *         - `int`: The new child node index to backtrack to (valid only
       * if backtracking is possible).
       */

      // cout << "backtrack: Called with str_i = " << str_i << ", curr_child_i =
      // " << curr_child_i << ", recursive = " << recursive << endl;
      if (backtrack_stack.empty()) {
        // cout << "backtrack: Backtracking not possible. Stack is empty." <<
        // endl;
        return make_tuple(false, str_i, curr_child_i);
      }

      auto [popped_child_i, min_, matched_times, chars_consumed_list] =
          backtrack_stack.back();
      backtrack_stack.pop_back();
      // cout << "backtrack: Popped state from stack: popped_child_i = " <<
      // popped_child_i<< ", min_ = " << min_ << ", matched_times = " <<
      // matched_times<<endl;

      if (matched_times == min_) {
        // cout << "backtrack: Node has matched the minimum number of times.
        // matched_times = " << matched_times << endl;
        int before_str_i = str_i;
        for (int consumption : chars_consumed_list) {
          str_i -= consumption;
        }
        // cout << "backtrack: Updated str_i after subtracting chars_consumed
        // lengths = " << str_i << endl;
        ASTNode *childNode = nullptr;

        if (auto groupNode = dynamic_cast<GroupNode *>(ast.get())) {
          childNode = groupNode->children[popped_child_i].get();
        } else if (auto reNode = dynamic_cast<RE *>(ast.get())) {
          childNode = reNode->children[popped_child_i].get();
        } else if (auto orNode = dynamic_cast<OrNode *>(ast.get())) {
          childNode = orNode->children[popped_child_i].get();
        }
        bool isLeafNode = (dynamic_cast<LeafNode *>(childNode) != nullptr);

        // cout<<(before_str_i == str_i)<<endl;
        if (max_matched_idx == -1 || before_str_i == str_i || isLeafNode) {
          // cout << "backtrack: Recursing with str_i = " << str_i << ",
          // popped_child_i = " << popped_child_i << endl;
          return backtrack(str_i, popped_child_i, true);
        } else {
          // cout << "backtrack: Handling nested quantifier case. Returning not
          // recursive." << endl;
          return make_tuple(!recursive, str_i, popped_child_i);
        }
      } else {
        // cout << "backtrack: Node was matched more times than its min.
        // matched_times = " << matched_times << endl;
        int last_chars_consumed = chars_consumed_list.back();
        chars_consumed_list.pop_back();
        int new_str_i = str_i - last_chars_consumed;
        // cout << "backtrack: Updated state after removing last consumption:
        // new_str_i = " << new_str_i<< ", matched_times = " << matched_times -
        // 1 <<endl;

        ASTNode *childNode = nullptr;

        if (auto groupNode = dynamic_cast<GroupNode *>(ast.get())) {
          childNode = groupNode->children[popped_child_i].get();
        } else if (auto reNode = dynamic_cast<RE *>(ast.get())) {
          childNode = reNode->children[popped_child_i].get();
        } else if (auto orNode = dynamic_cast<OrNode *>(ast.get())) {
          childNode = orNode->children[popped_child_i].get();
        }
        bool isLeafNode = (dynamic_cast<LeafNode *>(childNode) != nullptr);

        if (max_matched_idx == -1 || isLeafNode) {
          backtrack_stack.emplace_back(popped_child_i, min_, matched_times - 1,
                                       chars_consumed_list);
          // cout << "backtrack: Appending updated state to stack:
          // popped_child_i = " << popped_child_i<< ", min_ = " << min_ << ",
          // matched_times = " << matched_times - 1<<endl;
          return make_tuple(true, new_str_i, curr_child_i);
        } else {
          // cout << "backtrack: Handling nested quantifier case. Returning not
          // recursive." << endl;
          return make_tuple(!recursive, new_str_i, popped_child_i);
        }
      }
    };

    auto pop = [&](int curr_child_i, int str_i) {
      if (backtrack_stack.empty())
        return str_i;
      auto [popped_child_i, min_, matched_times, chars_consumed_list] =
          backtrack_stack.back();
      backtrack_stack.pop_back();
      if (popped_child_i == curr_child_i) {
        for (int consumption : chars_consumed_list) {
          str_i -= consumption;
        }
      } else {
        backtrack_stack.emplace_back(popped_child_i, min_, matched_times,
                                     chars_consumed_list);
      }
      return str_i;
    };

    if (dc<OrNode *>(ast.get())) {
      int tmp_str_i = str_i;
      auto [result_left, str_i_left] = save_matches(
          match_group, curr_node, input_string, str_i, max_matched_idx);
      if (!result_left) {
        str_i = tmp_str_i;
        curr_node = dc<OrNode *>(ast.get())->right;
        auto [result_right, str_i_right] = save_matches(
            match_group, curr_node, input_string, str_i, max_matched_idx);
        str_i = str_i_right;
        return make_tuple(result_right, str_i);
      }
      str_i = str_i_left;
      return make_tuple(result_left, str_i);
    }

    while (i < (dynamic_pointer_cast<RE>(ast)
                    ? dynamic_pointer_cast<RE>(ast)->children.size()
                : dynamic_pointer_cast<GroupNode>(ast)
                    ? dynamic_pointer_cast<GroupNode>(ast)->children.size()
                : dynamic_pointer_cast<OrNode>(ast)
                    ? dynamic_pointer_cast<OrNode>(ast)->children.size()
                    : 0)) {
      // cout<< "size"<<(dynamic_pointer_cast<RE>(ast) ?
      // dynamic_pointer_cast<RE>(ast)->children.size() :
      //  dynamic_pointer_cast<GroupNode>(ast) ?
      //  dynamic_pointer_cast<GroupNode>(ast)->children.size() :
      //  dynamic_pointer_cast<OrNode>(ast) ?
      //  dynamic_pointer_cast<OrNode>(ast)->children.size() : 0)<<endl;
      // cout << "Inside the main while loop, i = " << i << ", str_i = " <<
      // str_i<< " and "<< input_string[str_i]<<endl;
      // curr_node = dc<GroupNode*>(ast.get()) ?
      // dc<GroupNode*>(ast.get())->children[i] :
      // dc<RE*>(ast.get())->children[i]; cout << "Processing node type:  " <<
      // typeid(*curr_node).name() << endl; printChildren(ast);

      if (auto reNode = dynamic_pointer_cast<RE>(ast)) {
        curr_node = reNode->children[i];
      } else if (auto groupNode = dynamic_pointer_cast<GroupNode>(ast)) {
        curr_node = groupNode->children[i];
      } else if (auto orNode = dynamic_pointer_cast<OrNode>(ast)) {
        curr_node = orNode->children[i];
      }

      if (dc<OrNode *>(curr_node.get())) {
        int before_str_i = str_i;
        auto orn = dc<OrNode *>(curr_node.get());
        // int min_ = get<int>(or-> min);
        // int max_ = get<int>(or-> max);
        int min_ = 0;
        int max_ = 0;

        if (holds_alternative<int>(orn->min)) {
          min_ = get<int>(orn->min);
          // cout << "isOrNode: min is (int) " << min_ << endl;
        } else if (holds_alternative<double>(orn->min)) {
          double min_double = get<double>(orn->min);
          // cout << "isOrNode: min is (double) " << min_double << endl;
        } else {
          cout << "Unexpected type in min!" << endl;
        }

        if (holds_alternative<int>(orn->max)) {
          max_ = get<int>(orn->max);
          // cout << "isOrNode: max is (int) " << max_ << endl;
        } else if (holds_alternative<double>(orn->max)) {
          double max_double = get<double>(orn->max);
          if (isinf(max_double)) {
            max_ = INT_MAX;
            // cout << "isOrNode: max is infinite (treated as INT_MAX)" << endl;
          } else {
            max_ = static_cast<int>(max_double);
            // cout << "isOrNode: max is (double) " << max_ << endl;
          }
        } else {
          cout << "Unexpected type in max!" << endl;
        }

        int j = 0;
        vector<int> chars_consumed_list;

        bool backtracking = false;

        while (j < max_) {
          int tmp_str_i = str_i;
          bool save_match_left =
              dc<GroupNode *>(dc<OrNode *>(curr_node.get())->children[0].get());
          auto [result_left, str_i_left] =
              save_match_left
                  ? save_matches(match_group,
                                 dc<OrNode *>(curr_node.get())->children[0],
                                 input_string, str_i, max_matched_idx)
                  : match_group(dc<OrNode *>(curr_node.get())->children[0],
                                input_string, max_matched_idx);

          str_i = tmp_str_i;

          bool save_match_right =
              dc<GroupNode *>(dc<OrNode *>(curr_node.get())->children[1].get());
          auto [result_right, str_i_right] =
              save_match_right
                  ? save_matches(match_group,
                                 dc<OrNode *>(curr_node.get())->children[1],
                                 input_string, str_i, max_matched_idx)
                  : match_group(
                        dynamic_cast<OrNode *>(curr_node.get())->children[1],
                        input_string, max_matched_idx);

          if (result_left && result_right) {
            bool chose_left = (str_i_left >= str_i_right);
            str_i = chose_left ? str_i_left : str_i_right;
            if (max_matched_idx != -1 && str_i > max_matched_idx) {
              str_i = chose_left ? str_i_right : str_i_left;
            }
            if (chose_left) {
              if (save_match_right)
                remove_from_left();
              if (save_match_left)
                add_to_left();
            } else {
              if (save_match_left && !save_match_right)
                remove_from_left();
            }
          } else if (result_left && !result_right) {
            str_i = str_i_left;
          } else if (!result_left && result_right) {
            str_i = str_i_right;
          }

          bool result = (result_left || result_right);

          if (result && (max_matched_idx == -1 || str_i <= max_matched_idx)) {
            if ((str_i - tmp_str_i == 0) && j >= min_) {
              max_matched_idx = -1;
              break;
            }
            chars_consumed_list.push_back(str_i - tmp_str_i);
          } else {
            if (min_ <= j) {
              max_matched_idx = -1;
              break;
            }
            if (i > 0 && !dynamic_cast<LeafNode *>(
                             dynamic_cast<GroupNode *>(ast.get())
                                 ? dynamic_cast<GroupNode *>(ast.get())
                                       ->children[i - 1]
                                       .get()
                                 : dynamic_cast<RE *>(ast.get())
                                       ->children[i - 1]
                                       .get())) {
              str_i = pop(i, str_i);
              if (str_i == start_str_i)
                return make_tuple(false, str_i);
              max_matched_idx =
                  (max_matched_idx == -1) ? str_i - 1 : max_matched_idx - 1;
            }
            auto [can_bt, bt_str_i, bt_i] = backtrack(str_i, i, false);
            if (can_bt) {
              i = bt_i;
              str_i = bt_str_i;
              backtracking = true;
              break;
            } else {
              return make_tuple(false, str_i);
            }
          }
          j++;
        }
        if (!backtracking) {
          backtrack_stack.emplace_back(i, min_, j, chars_consumed_list);
          max_matched_idx = -1;
          i++;
        }
        continue;
      } else if (dc<GroupNode *>(curr_node.get())) {
        // cout << "isGroupNode: Inside Group Node, i = " << i << endl;
        auto gn = dc<GroupNode *>(curr_node.get());
        // int min_ = get<int>(gn-> min);
        // int max_ = get<int>(gn-> max);

        int min_ = 0;
        int max_ = 0;

        if (holds_alternative<int>(gn->min)) {
          min_ = get<int>(gn->min);
          // cout << "isGroupNode: min is (int) " << min_ << endl;
        } else if (holds_alternative<double>(gn->min)) {
          double min_double = get<double>(gn->min);
          // cout << "isGroupNode: min is (double) " << min_double << endl;
        } else {
          cout << "Unexpected type in min!" << endl;
        }

        if (holds_alternative<int>(gn->max)) {
          max_ = get<int>(gn->max);
          // cout << "isGroupNode: max is (int) " << max_ << endl;
        } else if (holds_alternative<double>(gn->max)) {
          double max_double = get<double>(gn->max);
          if (isinf(max_double)) {
            max_ = INT_MAX;
            // cout << "isGroupNode: max is infinite (treated as INT_MAX)" <<
            // endl;
          } else {
            max_ = static_cast<int>(max_double);
            // cout << "isGroupNode: max is (double) " << max_ << endl;
          }
        } else {
          cout << "Unexpected type in max!" << endl;
        }

        int j = 0;
        vector<int> chars_consumed_list;
        int before_str_i = str_i;
        bool backtracking = false;

        // cout << "isGroupNode: Starting inner while loop. j = " << j << ",
        // str_i = " << str_i << endl;

        while (j < max_) {
          // cout << "isGroupNode: Inner while loop iteration. j = " << j << ",
          // str_i = " << str_i << endl;
          int tmp_str_i = str_i;
          auto [result, new_str_i] = save_matches(
              match_group, curr_node, input_string, str_i, max_matched_idx);
          // cout << "isGroupNode: Match result: " << result << ", new_str_i = "
          // << new_str_i << endl;

          if (result &&
              (max_matched_idx == -1 || new_str_i <= max_matched_idx)) {
            if ((new_str_i - tmp_str_i == 0) && j >= min_) {
              max_matched_idx = -1;
              break;
            }
            chars_consumed_list.push_back(new_str_i - tmp_str_i);
          } else {

            if (min_ <= j) {
              max_matched_idx = -1;
              // cout << "isGroupNode: Breaking due to insufficient matches,
              // min: " << min_ << " , j: " << j << endl;
              break;
            }
            if (i > 0 &&
                !dc<LeafNode *>(
                    dc<GroupNode *>(ast.get())
                        ? dc<GroupNode *>(ast.get())->children[i - 1].get()
                        : dc<RE *>(ast.get())->children[i - 1].get())) {
              str_i = pop(i, str_i);
              // cout<<"isGroupNode: Updated str_i after pop = "<<str_i;
              if (str_i == start_str_i)
                return make_tuple(false, str_i);
              max_matched_idx =
                  (max_matched_idx == -1) ? str_i - 1 : max_matched_idx - 1;
            }
            auto [can_bt, bt_str_i, bt_i] = backtrack(str_i, i, false);
            // cout<<"isGroupNode: Backtrack result: can_bt = "<<can_bt<<"
            // ,bt_str_i = "<< bt_str_i <<" ,bt_i ="<< bt_i<<endl;
            if (can_bt) {
              i = bt_i;
              str_i = bt_str_i;
              backtracking = true;
              // cout<<"isGroupNode: Backtracking. Retrying to match the current
              // node."<<endl;
              break;
            } else {
              return make_tuple(false, str_i);
            }
          }
          j++;
        }
        if (!backtracking) {
          // cout << "Moving to next node, i++" << endl;
          backtrack_stack.emplace_back(i, min_, j, chars_consumed_list);
          max_matched_idx = -1;
          i++;
          // cout<<"isGroupNode: Updated backtrack_stack
          // ="<<backtrack_stack<<endl; cout<<"isGroupNode: Incremented i
          // to"<<i<<endl;
        }
        continue;

      } else if (dc<LeafNode *>(curr_node.get())) {
        // cout << "Inside Leaf Node, type: " << typeid(*curr_node).name() <<
        // endl; cout<<"isLeafNode: Inside Leaf Node, type:"<<
        // typeid(*curr_node).name()<<endl;
        if (auto ele = dc<Element *>(curr_node.get())) {
          // cout<<"isLeafNode: isElement:"<<endl;
          // cout<<"reached here"<<endl;
          // cout<<ele->match<<endl;
          int min_ = 0;
          int max_ = 0;
          // if(typeid(*curr_node).name()==typeid(WildcardElement).name()){
          //     min_ = 1;
          //     max_ = 1;
          //     cout << "isLeafNode: min (int): " << min_ <<endl;

          // }else {
          // Use holds_alternative and get to handle min and max
          if (holds_alternative<int>(ele->min)) {
            min_ = get<int>(ele->min);
            // cout << "isLeafNode: min (int): " << min_ << endl;
          } else if (holds_alternative<double>(ele->min)) {
            double min_double = get<double>(ele->min);
            // cout << "isLeafNode: min (double): " << min_double << endl;
          } else {
            cout << "Unexpected type in min!" << endl;
          }

          if (holds_alternative<int>(ele->max)) {
            max_ = get<int>(ele->max);
            // cout << "isLeafNode: max (int): " << max_ << endl;
          } else if (holds_alternative<double>(ele->max)) {
            double max_double = get<double>(ele->max);
            if (isinf(max_double)) {
              max_ = INT_MAX;
              // cout<< "isLeafNode: max is infinite (treated as
              // INT_MAX)"<<endl;
            } else {
              max_ = static_cast<int>(max_double);
              // cout<<"isLeafNode: max(double): "<<max_<<endl;
            }
          } else {
            cout << "Unexpected type in max!" << endl;
          }

          // cout<<"new_min is "<< min_<<endl;
          // cout<<"new_max is "<<max_<<endl;

          int j = 0;
          vector<int> chars_consumed_list;
          int before_str_i = str_i;
          // cout<<"isLeafNode: Starting inner while loop. j = "<< j<<" str_i =
          // "<<str_i<<endl;
          bool backtracking = false;
          while (j < max_) {
            // cout<<"isLeafNode: Inner while loop. j = "<< j<<" str_i =
            // "<<str_i<<endl;
            if (str_i < input_string.length()) {
              // cout<<"isLeafNode: Current character: "<<
              // input_string[str_i]<<endl;
              if (auto leaf_node = dc<LeafNode *>(curr_node.get())) {
                if (leaf_node->is_match(input_string.substr(str_i, 1), str_i,
                                        input_string.length()) &&
                    (max_matched_idx == -1 || str_i < max_matched_idx)) {
                  // cout<<"isLeafNode: Match successful for character: "<<
                  // input_string[str_i]<<endl;
                  if (!dc<StartElement *>(curr_node.get()) &&
                      !dc<EndElement *>(curr_node.get())) {
                    chars_consumed_list.push_back(1);
                    str_i++;
                    // cout<< "isLeafNode: Updated str_i = "<<str_i<<endl;
                  }
                } else {
                  // cout<<"isLeafNode: Match failed for character:
                  // "<<input_string[str_i]<<endl;
                  if (min_ <= j) {
                    // cout<<"isLeafNode: Breaking due to meeting minimum
                    // requirement. min = "<<min_<< " j = "<<j<<endl;
                    break;
                  }
                  if (i > 0 &&
                      !dc<LeafNode *>(
                          dc<GroupNode *>(ast.get())
                              ? dc<GroupNode *>(ast.get())
                                    ->children[i - 1]
                                    .get()
                              : dc<RE *>(ast.get())->children[i - 1].get())) {
                    str_i = pop(i, str_i);
                    // cout<<"isLeafNode: Updated str_i after pop =
                    // "<<str_i<<endl;
                    if (str_i == start_str_i)
                      return make_tuple(false, str_i);
                    max_matched_idx = str_i - 1;
                  }

                  auto [can_bt, bt_str_i, bt_i] = backtrack(str_i, i, false);
                  // cout<<"isLeafNode: Backtrack result: can_bt = "<< can_bt<<"
                  // bt_str_i = "<<bt_str_i<<" bt_i = "<<bt_i<<endl;
                  if (can_bt) {
                    i = bt_i;
                    str_i = bt_str_i;
                    backtracking = true;
                    // cout<<"isLeafNode: Backtracking. Retrying to match the
                    // current node."<<endl;
                    break;
                  } else {
                    return make_tuple(false, str_i);
                  }
                }
              } else {
                // cout<<"u shant be here"<<endl;
                return make_tuple(false, str_i);
              }
            } else {
              // cout<<"isLeafNode: Input fully processed. str_i = "<<str_i<<"
              // len(string) = "<< input_string.size()<<endl;
              if (dc<StartElement *>(curr_node.get()) ||
                  dc<EndElement *>(curr_node.get())) {
                if (auto leaf_node = dc<LeafNode *>(curr_node.get())) {
                  if (leaf_node->is_match("", str_i, input_string.length())) {
                    // cout<<"isLeafNode: Node is StartElement or EndElement and
                    // matches."<<endl;
                  }
                }
              } else if (min_ <= j) {
                // cout<<"isLeafNode: Breaking due to meeting minimum
                // requirement. min = "<<min_<<" j = "<<j<<endl;
                break;
              } else {
                // cout<<"isLeafNode: More states exist, but input is
                // finished."<<endl;
                auto [can_bt, bt_str_i, bt_i] =
                    backtrack(before_str_i, i, false);
                // cout<<"isLeafNode: Backtrack result: can_bt = "<<can_bt<<"
                // bt_str_i = "<<bt_str_i<<" bt_i = "<<bt_i<<endl;
                if (can_bt) {
                  i = bt_i;
                  str_i = bt_str_i;
                  backtracking = true;
                  // cout<<"isLeafNode: Backtracking. Retrying to match the
                  // current node."<<endl;
                  break;
                } else {
                  return make_tuple(false, str_i);
                }
              }
            }
            j++;
          }
          if (!backtracking) {
            // cout<<"isLeafNode: Incrementing i to "<<i + 1<<endl;
            backtrack_stack.emplace_back(i, min_, j, chars_consumed_list);
            i++;
          }
          continue;

        } else if (auto ele = dc<RangeElement *>(curr_node.get())) {
          // cout<<"isLeafNode: isRangeElement:"<<endl;
          // cout<<"reached here"<<endl;
          // cout<<ele->match<<endl;
          int min_ = 0;
          int max_ = 0;
          // if(typeid(*curr_node).name()==typeid(WildcardElement).name()){
          //     min_ = 1;
          //     max_ = 1;
          //     cout << "isLeafNode: min (int): " << min_ <<endl;

          // }else {
          // Use holds_alternative and get to handle min and max
          if (holds_alternative<int>(ele->min)) {
            min_ = get<int>(ele->min);
            // cout << "isLeafNode: min (int): " << min_ << endl;
          } else if (holds_alternative<double>(ele->min)) {
            double min_double = get<double>(ele->min);
            // cout << "isLeafNode: min (double): " << min_double << endl;
          } else {
            cout << "Unexpected type in min!" << endl;
          }

          if (holds_alternative<int>(ele->max)) {
            max_ = get<int>(ele->max);
            // cout << "isLeafNode: max (int): " << max_ << endl;
          } else if (holds_alternative<double>(ele->max)) {
            double max_double = get<double>(ele->max);
            if (isinf(max_double)) {
              max_ = INT_MAX;
              // cout<< "isLeafNode: max is infinite (treated as
              // INT_MAX)"<<endl;
            } else {
              max_ = static_cast<int>(max_double);
              // cout<<"isLeafNode: max(double): "<<max_<<endl;
            }
          } else {
            cout << "Unexpected type in max!" << endl;
          }

          // cout<<"new_min is "<< min_<<endl;
          // cout<<"new_max is "<<max_<<endl;

          int j = 0;
          vector<int> chars_consumed_list;
          int before_str_i = str_i;
          // cout<<"isLeafNode: Starting inner while loop. j = "<< j<<" str_i =
          // "<<str_i<<endl;
          bool backtracking = false;
          while (j < max_) {
            // cout<<"isLeafNode: Inner while loop. j = "<< j<<" str_i =
            // "<<str_i<<endl;
            if (str_i < input_string.length()) {
              // cout<<"isLeafNode: Current character: "<<
              // input_string[str_i]<<endl;
              if (auto leaf_node = dc<LeafNode *>(curr_node.get())) {
                if (leaf_node->is_match(input_string.substr(str_i, 1), str_i,
                                        input_string.length()) &&
                    (max_matched_idx == -1 || str_i < max_matched_idx)) {
                  // cout<<"isLeafNode: Match successful for character: "<<
                  // input_string[str_i]<<endl;
                  if (!dc<StartElement *>(curr_node.get()) &&
                      !dc<EndElement *>(curr_node.get())) {
                    chars_consumed_list.push_back(1);
                    str_i++;
                    // cout<< "isLeafNode: Updated str_i = "<<str_i<<endl;
                  }
                } else {
                  // cout<<"isLeafNode: Match failed for character:
                  // "<<input_string[str_i]<<endl;
                  if (min_ <= j) {
                    // cout<<"isLeafNode: Breaking due to meeting minimum
                    // requirement. min = "<<min_<< " j = "<<j<<endl;
                    break;
                  }
                  if (i > 0 &&
                      !dc<LeafNode *>(
                          dc<GroupNode *>(ast.get())
                              ? dc<GroupNode *>(ast.get())
                                    ->children[i - 1]
                                    .get()
                              : dc<RE *>(ast.get())->children[i - 1].get())) {
                    str_i = pop(i, str_i);
                    // cout<<"isLeafNode: Updated str_i after pop =
                    // "<<str_i<<endl;
                    if (str_i == start_str_i)
                      return make_tuple(false, str_i);
                    max_matched_idx = str_i - 1;
                  }

                  auto [can_bt, bt_str_i, bt_i] = backtrack(str_i, i, false);
                  // cout<<"isLeafNode: Backtrack result: can_bt = "<< can_bt<<"
                  // bt_str_i = "<<bt_str_i<<" bt_i = "<<bt_i<<endl;
                  if (can_bt) {
                    i = bt_i;
                    str_i = bt_str_i;
                    backtracking = true;
                    // cout<<"isLeafNode: Backtracking. Retrying to match the
                    // current node."<<endl;
                    break;
                  } else {
                    return make_tuple(false, str_i);
                  }
                }
              } else {
                // cout<<"u shant be here"<<endl;
                return make_tuple(false, str_i);
              }
            } else {
              // cout<<"isLeafNode: Input fully processed. str_i = "<<str_i<<"
              // len(string) = "<< input_string.size()<<endl;
              if (dc<StartElement *>(curr_node.get()) ||
                  dc<EndElement *>(curr_node.get())) {
                if (auto leaf_node = dc<LeafNode *>(curr_node.get())) {
                  if (leaf_node->is_match("", str_i, input_string.length())) {
                    // cout<<"isLeafNode: Node is StartElement or EndElement and
                    // matches."<<endl;
                  }
                }
              } else if (min_ <= j) {
                // cout<<"isLeafNode: Breaking due to meeting minimum
                // requirement. min = "<<min_<<" j = "<<j<<endl;
                break;
              } else {
                // cout<<"isLeafNode: More states exist, but input is
                // finished."<<endl;
                auto [can_bt, bt_str_i, bt_i] =
                    backtrack(before_str_i, i, false);
                // cout<<"isLeafNode: Backtrack result: can_bt = "<<can_bt<<"
                // bt_str_i = "<<bt_str_i<<" bt_i = "<<bt_i<<endl;
                if (can_bt) {
                  i = bt_i;
                  str_i = bt_str_i;
                  backtracking = true;
                  // cout<<"isLeafNode: Backtracking. Retrying to match the
                  // current node."<<endl;
                  break;
                } else {
                  return make_tuple(false, str_i);
                }
              }
            }
            j++;
          }
          if (!backtracking) {
            // cout<<"isLeafNode: Incrementing i to "<<i + 1<<endl;
            backtrack_stack.emplace_back(i, min_, j, chars_consumed_list);
            i++;
          }
          continue;
        } else if (auto ele = dc<StartElement *>(curr_node.get())) {
          // cout<<"isLeafNode: isStartElement:"<<endl;
          // cout<<"reached here"<<endl;
          // cout<<ele->match<<endl;
          int min_ = 0;
          int max_ = 0;
          // if(typeid(*curr_node).name()==typeid(WildcardElement).name()){
          //     min_ = 1;
          //     max_ = 1;
          //     cout << "isLeafNode: min (int): " << min_ <<endl;

          // }else {
          // Use holds_alternative and get to handle min and max
          if (holds_alternative<int>(ele->min)) {
            min_ = get<int>(ele->min);
            // cout << "isLeafNode: min (int): " << min_ << endl;
          } else if (holds_alternative<double>(ele->min)) {
            double min_double = get<double>(ele->min);
            // cout << "isLeafNode: min (double): " << min_double << endl;
          } else {
            cout << "Unexpected type in min!" << endl;
          }

          if (holds_alternative<int>(ele->max)) {
            max_ = get<int>(ele->max);
            // cout << "isLeafNode: max (int): " << max_ << endl;
          } else if (holds_alternative<double>(ele->max)) {
            double max_double = get<double>(ele->max);
            if (isinf(max_double)) {
              max_ = INT_MAX;
              // cout<< "isLeafNode: max is infinite (treated as
              // INT_MAX)"<<endl;
            } else {
              max_ = static_cast<int>(max_double);
              // cout<<"isLeafNode: max(double): "<<max_<<endl;
            }
          } else {
            cout << "Unexpected type in max!" << endl;
          }

          // cout<<"new_min is "<< min_<<endl;
          // cout<<"new_max is "<<max_<<endl;

          int j = 0;
          vector<int> chars_consumed_list;
          int before_str_i = str_i;
          // cout<<"isLeafNode: Starting inner while loop. j = "<< j<<" str_i =
          // "<<str_i<<endl;
          bool backtracking = false;
          while (j < max_) {
            // cout<<"isLeafNode: Inner while loop. j = "<< j<<" str_i =
            // "<<str_i<<endl;
            if (str_i < input_string.length()) {
              // cout<<"isLeafNode: Current character: "<<
              // input_string[str_i]<<endl;
              if (auto leaf_node = dc<LeafNode *>(curr_node.get())) {
                if (leaf_node->is_match(input_string.substr(str_i, 1), str_i,
                                        input_string.length()) &&
                    (max_matched_idx == -1 || str_i < max_matched_idx)) {
                  // cout<<"isLeafNode: Match successful for character: "<<
                  // input_string[str_i]<<endl;
                  if (!dc<StartElement *>(curr_node.get()) &&
                      !dc<EndElement *>(curr_node.get())) {
                    chars_consumed_list.push_back(1);
                    str_i++;
                    // cout<< "isLeafNode: Updated str_i = "<<str_i<<endl;
                  }
                } else {
                  // cout<<"isLeafNode: Match failed for character:
                  // "<<input_string[str_i]<<endl;
                  if (min_ <= j) {
                    // cout<<"isLeafNode: Breaking due to meeting minimum
                    // requirement. min = "<<min_<< " j = "<<j<<endl;
                    break;
                  }
                  if (i > 0 &&
                      !dc<LeafNode *>(
                          dc<GroupNode *>(ast.get())
                              ? dc<GroupNode *>(ast.get())
                                    ->children[i - 1]
                                    .get()
                              : dc<RE *>(ast.get())->children[i - 1].get())) {
                    str_i = pop(i, str_i);
                    // cout<<"isLeafNode: Updated str_i after pop =
                    // "<<str_i<<endl;
                    if (str_i == start_str_i)
                      return make_tuple(false, str_i);
                    max_matched_idx = str_i - 1;
                  }

                  auto [can_bt, bt_str_i, bt_i] = backtrack(str_i, i, false);
                  // cout<<"isLeafNode: Backtrack result: can_bt = "<< can_bt<<"
                  // bt_str_i = "<<bt_str_i<<" bt_i = "<<bt_i<<endl;
                  if (can_bt) {
                    i = bt_i;
                    str_i = bt_str_i;
                    backtracking = true;
                    // cout<<"isLeafNode: Backtracking. Retrying to match the
                    // current node."<<endl;
                    break;
                  } else {
                    return make_tuple(false, str_i);
                  }
                }
              } else {
                // cout<<"u shant be here"<<endl;
                return make_tuple(false, str_i);
              }
            } else {
              // cout<<"isLeafNode: Input fully processed. str_i = "<<str_i<<"
              // len(string) = "<< input_string.size()<<endl;
              if (dc<StartElement *>(curr_node.get()) ||
                  dc<EndElement *>(curr_node.get())) {
                if (auto leaf_node = dc<LeafNode *>(curr_node.get())) {
                  if (leaf_node->is_match("", str_i, input_string.length())) {
                    // cout<<"isLeafNode: Node is StartElement or EndElement and
                    // matches."<<endl;
                  }
                }
              } else if (min_ <= j) {
                // cout<<"isLeafNode: Breaking due to meeting minimum
                // requirement. min = "<<min_<<" j = "<<j<<endl;
                break;
              } else {
                // cout<<"isLeafNode: More states exist, but input is
                // finished."<<endl;
                auto [can_bt, bt_str_i, bt_i] =
                    backtrack(before_str_i, i, false);
                // cout<<"isLeafNode: Backtrack result: can_bt = "<<can_bt<<"
                // bt_str_i = "<<bt_str_i<<" bt_i = "<<bt_i<<endl;
                if (can_bt) {
                  i = bt_i;
                  str_i = bt_str_i;
                  backtracking = true;
                  // cout<<"isLeafNode: Backtracking. Retrying to match the
                  // current node."<<endl;
                  break;
                } else {
                  return make_tuple(false, str_i);
                }
              }
            }
            j++;
          }
          if (!backtracking) {
            // cout<<"isLeafNode: Incrementing i to "<<i + 1<<endl;
            backtrack_stack.emplace_back(i, min_, j, chars_consumed_list);
            i++;
          }
          continue;
        } else if (auto ele = dc<EndElement *>(curr_node.get())) {
          // cout<<"isLeafNode: isEndElement:"<<endl;
          // cout<<"reached here"<<endl;
          // cout<<ele->match<<endl;
          int min_ = 0;
          int max_ = 0;
          // if(typeid(*curr_node).name()==typeid(WildcardElement).name()){
          //     min_ = 1;
          //     max_ = 1;
          //     cout << "isLeafNode: min (int): " << min_ <<endl;

          // }else {
          // Use holds_alternative and get to handle min and max
          if (holds_alternative<int>(ele->min)) {
            min_ = get<int>(ele->min);
            // cout << "isLeafNode: min (int): " << min_ << endl;
          } else if (holds_alternative<double>(ele->min)) {
            double min_double = get<double>(ele->min);
            // cout << "isLeafNode: min (double): " << min_double << endl;
          } else {
            cout << "Unexpected type in min!" << endl;
          }

          if (holds_alternative<int>(ele->max)) {
            max_ = get<int>(ele->max);
            // cout << "isLeafNode: max (int): " << max_ << endl;
          } else if (holds_alternative<double>(ele->max)) {
            double max_double = get<double>(ele->max);
            if (isinf(max_double)) {
              max_ = INT_MAX;
              // cout<< "isLeafNode: max is infinite (treated as
              // INT_MAX)"<<endl;
            } else {
              max_ = static_cast<int>(max_double);
              // cout<<"isLeafNode: max(double): "<<max_<<endl;
            }
          } else {
            cout << "Unexpected type in max!" << endl;
          }

          // cout<<"new_min is "<< min_<<endl;
          // cout<<"new_max is "<<max_<<endl;

          int j = 0;
          vector<int> chars_consumed_list;
          int before_str_i = str_i;
          // cout<<"isLeafNode: Starting inner while loop. j = "<< j<<" str_i =
          // "<<str_i<<endl;
          bool backtracking = false;
          while (j < max_) {
            // cout<<"isLeafNode: Inner while loop. j = "<< j<<" str_i =
            // "<<str_i<<endl;
            if (str_i < input_string.length()) {
              // cout<<"isLeafNode: Current character: "<<
              // input_string[str_i]<<endl;
              if (auto leaf_node = dc<LeafNode *>(curr_node.get())) {
                if (leaf_node->is_match(input_string.substr(str_i, 1), str_i,
                                        input_string.length()) &&
                    (max_matched_idx == -1 || str_i < max_matched_idx)) {
                  // cout<<"isLeafNode: Match successful for character: "<<
                  // input_string[str_i]<<endl;
                  if (!dc<StartElement *>(curr_node.get()) &&
                      !dc<EndElement *>(curr_node.get())) {
                    chars_consumed_list.push_back(1);
                    str_i++;
                    // cout<< "isLeafNode: Updated str_i = "<<str_i<<endl;
                  }
                } else {
                  // cout<<"isLeafNode: Match failed for character:
                  // "<<input_string[str_i]<<endl;
                  if (min_ <= j) {
                    // cout<<"isLeafNode: Breaking due to meeting minimum
                    // requirement. min = "<<min_<< " j = "<<j<<endl;
                    break;
                  }
                  if (i > 0 &&
                      !dc<LeafNode *>(
                          dc<GroupNode *>(ast.get())
                              ? dc<GroupNode *>(ast.get())
                                    ->children[i - 1]
                                    .get()
                              : dc<RE *>(ast.get())->children[i - 1].get())) {
                    str_i = pop(i, str_i);
                    // cout<<"isLeafNode: Updated str_i after pop =
                    // "<<str_i<<endl;
                    if (str_i == start_str_i)
                      return make_tuple(false, str_i);
                    max_matched_idx = str_i - 1;
                  }

                  auto [can_bt, bt_str_i, bt_i] = backtrack(str_i, i, false);
                  // cout<<"isLeafNode: Backtrack result: can_bt = "<< can_bt<<"
                  // bt_str_i = "<<bt_str_i<<" bt_i = "<<bt_i<<endl;
                  if (can_bt) {
                    i = bt_i;
                    str_i = bt_str_i;
                    backtracking = true;
                    // cout<<"isLeafNode: Backtracking. Retrying to match the
                    // current node."<<endl;
                    break;
                  } else {
                    return make_tuple(false, str_i);
                  }
                }
              } else {
                // cout<<"u shant be here"<<endl;
                return make_tuple(false, str_i);
              }
            } else {
              // cout<<"isLeafNode: Input fully processed. str_i = "<<str_i<<"
              // len(string) = "<< input_string.size()<<endl;
              if (dc<StartElement *>(curr_node.get()) ||
                  dc<EndElement *>(curr_node.get())) {
                if (auto leaf_node = dc<LeafNode *>(curr_node.get())) {
                  if (leaf_node->is_match("", str_i, input_string.length())) {
                    // cout<<"isLeafNode: Node is StartElement or EndElement and
                    // matches."<<endl;
                  }
                }
              } else if (min_ <= j) {
                // cout<<"isLeafNode: Breaking due to meeting minimum
                // requirement. min = "<<min_<<" j = "<<j<<endl;
                break;
              } else {
                // cout<<"isLeafNode: More states exist, but input is
                // finished."<<endl;
                auto [can_bt, bt_str_i, bt_i] =
                    backtrack(before_str_i, i, false);
                // cout<<"isLeafNode: Backtrack result: can_bt = "<<can_bt<<"
                // bt_str_i = "<<bt_str_i<<" bt_i = "<<bt_i<<endl;
                if (can_bt) {
                  i = bt_i;
                  str_i = bt_str_i;
                  backtracking = true;
                  // cout<<"isLeafNode: Backtracking. Retrying to match the
                  // current node."<<endl;
                  break;
                } else {
                  return make_tuple(false, str_i);
                }
              }
            }
            j++;
          }
          if (!backtracking) {
            // cout<<"isLeafNode: Incrementing i to "<<i + 1<<endl;
            backtrack_stack.emplace_back(i, min_, j, chars_consumed_list);
            i++;
          }
          continue;
        } else {
          // cout<<"Implement this node bitch"<<endl;
          return make_tuple(false, str_i);
        }
      } else {
        cout << "Unhandled node type, exiting." << endl;
        return make_tuple(false, str_i);
      }
    }
    // cout << "Exiting main while loop." << endl;
    return make_tuple(true, str_i);
  };

  //**********************************************************************************************************************
  //*/
  int i = str_i;

  if (input_string.empty()) {

    auto [result, chars_consumed] =
        save_matches(match_group, ast, input_string, str_i);
    return give_back(result, chars_consumed);
  }

  while (str_i < input_string.length()) {
    auto [result, _] = save_matches(match_group, ast, input_string, str_i);
    i++;
    if (result) {
      return give_back(true, str_i);
    } else {
      matches.clear();
      str_i = i;
    }
  }
  return give_back(false, str_i);
}
