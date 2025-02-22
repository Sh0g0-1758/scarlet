#ifndef RE_AST_HH
#define RE_AST_HH

#include <algorithm>
#include <cctype>
#include <deque>
#include <iostream>
#include <memory>
#include <string>
#include <variant>
#include <vector>

using namespace std;

// Base class for all AST nodes
class ASTNode {
public:
  ASTNode() = default;
  virtual ~ASTNode(){};
};

// Regular Expression Node
class RE : public ASTNode {
public:
  bool __capturing__;
  string group_name;
  int group_id;
  shared_ptr<ASTNode> child;
  deque<shared_ptr<ASTNode>> children;

  RE(shared_ptr<ASTNode> child, bool capturing = false,
     string group_name = "RegEx");
  bool is_capturing() const;
};

// Leaf Node Base Class
class LeafNode : public ASTNode {
public:
  LeafNode();
  virtual bool is_match(const string &ch = "", int str_i = -1,
                        int str_len = -1) const;
};

// Element Node
class Element : public LeafNode {
public:
  Element(const string &match_ch = "");
  bool is_match(const string &ch = "", int str_i = 0,
                int str_len = 0) const override;
  string match;
  variant<int, double> min;
  variant<int, double> max;
};

// Wildcard Element Node
class WildcardElement : public Element {
public:
  WildcardElement();
  bool is_match(const string &ch = "", int str_i = 0,
                int str_len = 0) const override;
};

// Space Element Node
class SpaceElement : public Element {
public:
  SpaceElement();
  bool is_match(const string &ch = "", int str_i = 0,
                int str_len = 0) const override;
};

// Range Element Node
class RangeElement : public LeafNode {
public:
  RangeElement(const string &match_str, bool is_positive_logic = true);
  bool is_match(const string &ch = "", int str_i = 0,
                int str_len = 0) const override;
  string match;
  variant<int, double> min;
  variant<int, double> max;
  bool is_positive_logic;
};

// Start Element Node
class StartElement : public LeafNode {
public:
  StartElement();
  bool is_match(const string &ch = "", int str_i = 0,
                int str_len = 0) const override;
  string match;
  variant<int, double> min;
  variant<int, double> max;
};

// End Element Node
class EndElement : public LeafNode {
public:
  EndElement();
  bool is_match(const string &ch = "", int str_i = 0,
                int str_len = 0) const override;
  string match;
  variant<int, double> min;
  variant<int, double> max;
};

// OR Node
class OrNode : public ASTNode {
public:
  shared_ptr<ASTNode> left;
  shared_ptr<ASTNode> right;
  vector<shared_ptr<ASTNode>> children;
  variant<int, double> min;
  variant<int, double> max;

  OrNode(shared_ptr<ASTNode> left, shared_ptr<ASTNode> right);
};

// Group Node
class GroupNode : public ASTNode {
public:
  bool __capturing__;
  string group_name;
  int group_id;
  deque<shared_ptr<ASTNode>> children;
  variant<int, double> min;
  variant<int, double> max;

  GroupNode(deque<shared_ptr<ASTNode>> children, bool capturing = false,
            string group_name = "", int group_id = -1);
  bool is_capturing() const;
};

#endif // RE_AST_HH