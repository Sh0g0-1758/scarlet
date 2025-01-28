#ifndef REGEX_HH
#define REGEX_HH

#include "../parser/parser.hh"
#include "match.hh"
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

class RegexEngine {
public:
  RegexEngine() : prev_re(""), prev_ast(nullptr) {}

  string prev_re;
  shared_ptr<RE> prev_ast;
  void printChildren(const shared_ptr<ASTNode> &node);
  tuple<bool, int, vector<deque<Match>>>
  match(const string &re, const string &input_string,
        bool return_matches = false, bool continue_after_match = false);
  void printAST(const shared_ptr<ASTNode> &node, int depth = 0);
  tuple<bool, int, deque<Match>> matchInternal(shared_ptr<RE> ast,
                                               const string &input_string,
                                               int start_str_i);
};

#endif // REGEX_HH