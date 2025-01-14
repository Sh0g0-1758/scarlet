#include "ast.hh"
// RE Class Implementation
RE::RE(shared_ptr<ASTNode> child, bool capturing, string group_name)
    : __capturing__(capturing), group_name(group_name), group_id(-1), child(child) {
    children.push_back(child);
}

bool RE::is_capturing() const {
    return __capturing__;
}

// LeafNode Class Implementation
LeafNode::LeafNode() : ASTNode() {}

bool LeafNode::is_match(const string& ch, int str_i, int str_len) const {
    return false;
}

// Element Class Implementation
Element::Element(const string& match_ch) : LeafNode(), match(match_ch), min(1), max(1) {}

bool Element::is_match(const string& ch, int str_i, int str_len) const {
    return match == ch;
}

// WildcardElement Class Implementation
WildcardElement::WildcardElement() : Element("anything") {}

bool WildcardElement::is_match(const string& ch, int str_i, int str_len) const {
    return ch != "\n";
}

// SpaceElement Class Implementation
SpaceElement::SpaceElement() : Element("") {
    match = "";
}

bool SpaceElement::is_match(const string& ch, int str_i, int str_len) const {
    return ch.length() == 1 && isspace(ch[0]);
}

// RangeElement Class Implementation
RangeElement::RangeElement(const string& match_str, bool is_positive_logic)
    : LeafNode(), match(match_str), min(1), max(1), is_positive_logic(is_positive_logic) {}

bool RangeElement::is_match(const string& ch, int str_i, int str_len) const {
    if (ch.empty()) return false;
    bool ch_in_match = (match.find(ch[0]) != string::npos);
    return is_positive_logic ? ch_in_match : !ch_in_match;
}

// StartElement Class Implementation
StartElement::StartElement() : LeafNode() {
    match = "";
    min = 1;
    max = 1;
}

bool StartElement::is_match(const string& ch, int str_i, int str_len) const {
    return str_i == 0;
}

// EndElement Class Implementation
EndElement::EndElement() : LeafNode() {
    match = "";
    min = 1;
    max = 1;
}

bool EndElement::is_match(const string& ch, int str_i, int str_len) const {
    return str_i == str_len;
}

// OrNode Class Implementation
OrNode::OrNode(shared_ptr<ASTNode> left, shared_ptr<ASTNode> right) : ASTNode() {
    this->left = left;
    this->right = right;
    this->children.push_back(left);
    this->children.push_back(right);
    this->min = 1;
    this->max = 1;
}

// GroupNode Class Implementation
GroupNode::GroupNode(deque<shared_ptr<ASTNode>> children, bool capturing, string group_name, int group_id)
    : __capturing__(capturing), group_id(group_id), min(1), max(1) {
    this->group_name = (group_name.empty()) ? "Group" + to_string(this->group_id) : group_name;
    this->children = children;
}

bool GroupNode::is_capturing() const {
    return __capturing__;
}