#ifndef PARSER_HH
#define PARSER_HH

#include <vector>
#include <deque>
#include <memory>
#include <string>
#include <variant>
#include <limits>
#include <algorithm>
#include <stdexcept>
#include <functional>
#include "../lexer/lexer.hh"
#include "../ast/ast.hh"
#include <iostream>

#define dc dynamic_cast

using namespace std;

class Parser {
public:
    Lexer lxr;
    vector<Token*> tokens;
    int i = -1;
    Token* curr_tkn = nullptr;

    shared_ptr<RE> parse(const string& re);
    Token* next_tkn(bool without_consuming = false);
    function<Token*(bool)> next_tkn_initializer(const string& re);
    shared_ptr<RE> parse_re();
    shared_ptr<ASTNode> parse_re_seq(bool capturing = true, const string& group_name = "", int group_id = -1);
    int groups_counter = 0;
    shared_ptr<GroupNode> parse_group(bool capturing = true, const string& group_name = "", int group_id = -1);
    void parse_curly(shared_ptr<ASTNode> new_el);
    shared_ptr<ASTNode> parse_range_el();
    string get_range_str(char start, char end);
    shared_ptr<RangeElement> parse_inner_el();
    shared_ptr<ASTNode> parse_el();
    string parse_group_name();
};

#endif // PARSER_HH