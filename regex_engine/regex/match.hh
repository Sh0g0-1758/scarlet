// src/regex/match.hh
#ifndef MATCH_HH
#define MATCH_HH

#include <string>

class Match {
public:
  // Constructor
  Match(int group_id, int start_idx, int end_idx, const std::string &str,
        const std::string &name)
      : group_id(group_id), start_idx(start_idx), end_idx(end_idx), str(str),
        name(name) {
    match = str.substr(start_idx, end_idx - start_idx);
  }

  // Member variables
  int group_id;
  int start_idx;
  int end_idx;
  std::string str;
  std::string name;
  std::string match;
};

#endif // MATCH_HH