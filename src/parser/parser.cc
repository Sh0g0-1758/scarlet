#include "parser.hh"
#include<iostream>

using namespace std;

shared_ptr<RE> Parser::parse(const string& re) {
    tokens = lxr.scan(re);
    i = -1;
    next_tkn();
    auto ast = parse_re();
    if (curr_tkn) {
        throw runtime_error("Unable to parse the regex.");
    }
    return ast;
}

regex::Token* Parser::next_tkn(bool without_consuming) {
    if (without_consuming) {
        return (i + 1 < tokens.size()) ? tokens[i + 1] : nullptr;
    }
    i++;
    if (i < tokens.size()) {
        curr_tkn = tokens[i];
    } else {
        curr_tkn = nullptr;
    }
    return curr_tkn;
}

function<regex::Token*(bool)> Parser::next_tkn_initializer(const string& re) {
    vector<regex::Token*> tokens = lxr.scan(re);
    int i = -1;

    return [tokens, i](bool without_consuming = false) mutable -> regex::Token* {
        if (without_consuming) {
            return (i + 1 < tokens.size()) ? tokens[i + 1] : nullptr;
        }
        i++;
        return (i < tokens.size()) ? tokens[i] : nullptr;
    };
}

shared_ptr<RE> Parser::parse_re() {
    return make_shared<RE>(parse_re_seq());
}



shared_ptr<ASTNode> Parser::parse_re_seq(bool capturing, const string& group_name, int group_id) {
    bool match_start = false;
    bool match_end = false;

    // Debug: Print the current token at the start of the function
    //std::cout << "parse_re_seq: Current token = " << (curr_tkn ? curr_tkn->char_ : "nullptr") << std::endl;

    // Handle start anchor (^)
    if (dc<regex::Start*>(curr_tkn) || dc<regex::Circumflex*>(curr_tkn)) {
        //cout << "parse_re_seq: Found start anchor (^), setting match_start = true" << std::endl;
        next_tkn();
        match_start = true;
    }

    // Parse the group
    //cout << "parse_re_seq: Parsing group..." << std::endl;
    shared_ptr<ASTNode> node = parse_group(capturing, group_name, group_id);

    // Handle end anchor ($)
    if (dc<regex::EndToken*>(curr_tkn)) {
        //cout << "parse_re_seq: Found end anchor ($), setting match_end = true" << std::endl;
        next_tkn();
        match_end = true;
    }

    // Add StartElement if match_start is true
    if (match_start) {
        //cout << "parse_re_seq: Adding StartElement to the group" << std::endl;
        dynamic_pointer_cast<GroupNode>(node)->children.push_front(make_shared<StartElement>());
    }

    // Add EndElement if match_end is true
    if (match_end) {
        //cout << "parse_re_seq: Adding EndElement to the group" << std::endl;
        dynamic_pointer_cast<GroupNode>(node)->children.push_back(make_shared<EndElement>());
    }

    // Handle alternation (|)
    if (dc<regex::OrToken*>(curr_tkn)) {
        //cout << "parse_re_seq: Found alternation (|), creating OrNode" << std::endl;
        next_tkn();
        node = make_shared<OrNode>(node, parse_re_seq(true, group_name, dynamic_pointer_cast<GroupNode>(node)->group_id));
    }

    return node;
}

shared_ptr<GroupNode> Parser::parse_group(bool capturing, const string& group_name, int group_id) {
    if (group_id == -1) {
        group_id = groups_counter++;
    }

    deque<shared_ptr<ASTNode>> elements;

    //cout << "Parsing group with group_id: " << group_id << endl;

    while (curr_tkn && !dynamic_cast<regex::OrToken*>(curr_tkn) &&
           !dynamic_cast<regex::RightParenthesis*>(curr_tkn) &&
           !dynamic_cast<regex::EndToken*>(curr_tkn)) {
        //cout << "Current token: " << (curr_tkn ? curr_tkn->char_ : "nullptr") << endl;

        auto new_el = parse_range_el();
        //cout << "Parsed element: " << new_el << endl;

        next_tkn();

        if (dynamic_cast<regex::EndToken*>(curr_tkn)) {
            elements.push_back(new_el);
            //cout << "EndToken encountered, breaking the loop." << endl;
            break;
        }

        // Handle quantifiers for any valid AST node
        if (dynamic_cast<regex::Quantifier*>(curr_tkn)) {
            //cout << "Quantifier found, handling..." << endl;

            // Downcast new_el to the appropriate derived class
            if (auto element = dynamic_pointer_cast<Element>(new_el)) {
                if (dynamic_cast<regex::ZeroOrOne*>(curr_tkn)) {
                    //cout << "ZeroOrOne quantifier." << endl;
                    element->min = 0;
                    element->max = 1;
                } else if (dynamic_cast<regex::ZeroOrMore*>(curr_tkn)) {
                    //cout << "ZeroOrMore quantifier." << endl;
                    element->min = 0;
                    element->max = numeric_limits<double>::infinity();
                } else if (dynamic_cast<regex::OneOrMore*>(curr_tkn)) {
                    //cout << "OneOrMore quantifier." << endl;
                    element->min = 1;
                    element->max = numeric_limits<double>::infinity();
                }
            } else if (auto range_element = dynamic_pointer_cast<RangeElement>(new_el)) {
                if (dynamic_cast<regex::ZeroOrOne*>(curr_tkn)) {
                    //cout << "ZeroOrOne quantifier." << endl;
                    range_element->min = 0;
                    range_element->max = 1;
                } else if (dynamic_cast<regex::ZeroOrMore*>(curr_tkn)) {
                    //cout << "ZeroOrMore quantifier." << endl;
                    range_element->min = 0;
                    range_element->max = numeric_limits<double>::infinity();
                } else if (dynamic_cast<regex::OneOrMore*>(curr_tkn)) {
                    //cout << "OneOrMore quantifier." << endl;
                    range_element->min = 1;
                    range_element->max = numeric_limits<double>::infinity();
                }
            } else if (auto group_node = dynamic_pointer_cast<GroupNode>(new_el)) {
                if (dynamic_cast<regex::ZeroOrOne*>(curr_tkn)) {
                    //cout << "ZeroOrOne quantifier for group." << endl;
                    group_node->min = 0;
                    group_node->max = 1;
                } else if (dynamic_cast<regex::ZeroOrMore*>(curr_tkn)) {
                    //cout << "ZeroOrMore quantifier." << endl;
                    group_node->min = 0;
                    group_node->max = numeric_limits<double>::infinity();
                } else if (dynamic_cast<regex::OneOrMore*>(curr_tkn)) {
                    //cout << "OneOrMore quantifier." << endl;
                    group_node->min = 1;
                    group_node->max = numeric_limits<double>::infinity();
                }
            } else if (auto or_node = dynamic_pointer_cast<OrNode>(new_el)) {
                if (dynamic_cast<regex::ZeroOrOne*>(curr_tkn)) {
                    //cout << "ZeroOrOne quantifier." << endl;
                    or_node->min = 0;
                    or_node->max = 1;
                } else if (dynamic_cast<regex::ZeroOrMore*>(curr_tkn)) {
                    //cout << "ZeroOrMore quantifier." << endl;
                    or_node->min = 0;
                    or_node->max = numeric_limits<double>::infinity();
                } else if (dynamic_cast<regex::OneOrMore*>(curr_tkn)) {
                    //cout << "OneOrMore quantifier." << endl;
                    or_node->min = 1;
                    or_node->max = numeric_limits<double>::infinity();
                }
            }

            next_tkn(); // Consume the quantifier token
        } else if (dynamic_cast<regex::LeftCurlyBrace*>(curr_tkn)) {
            //cout << "Current token: " << (curr_tkn ? curr_tkn->char_ : "nullptr") << endl;
            //cout << "LeftCurlyBrace found, parsing curly..." << endl;
            //cout << "Current token: " << (curr_tkn ? curr_tkn->char_ : "nullptr") << endl;
            parse_curly(new_el);
        }

        elements.push_back(new_el);
    }

    //cout << "Parsed " << elements.size() << " elements in the group." << endl;

    //cout << "Returning GroupNode..." << endl;
    return make_shared<GroupNode>(elements, capturing, group_name, group_id);
}
// main engine works on this
void Parser::parse_curly(shared_ptr<ASTNode> new_el) {
    next_tkn();//past the curly brace
    //cout << "Current token: " << (curr_tkn ? curr_tkn->char_ : "nullptr") << endl;
    //cases: 2,} 2} 2,3} }
    if (dynamic_cast<regex::RightCurlyBrace*>(curr_tkn)) {
        throw runtime_error("Empty brace found, Invalid syntax");
    }
    //cases: 2,} 2} 2,3}
    string val_1, val_2;

    try {
        //cout << "Inside the try block" << endl;
        //cases: 2,} 2} 2,3}
        while (dc<regex::ElementToken*>(curr_tkn)) {
            auto shit = dc<regex::ElementToken*>(curr_tkn);
            val_1 += shit->char_;
            next_tkn();
            //cout << "Current token: " << (curr_tkn ? curr_tkn->char_ : "nullptr") << endl;
        }
        //past all the elements, extracted val_1
        //cases: ,} } ,3}

        if (val_1.empty()) val_1 = "0";
        //cout << "val_1 is " << val_1 << endl;
        //cout<<"reached here?"<<endl;
        auto element = dynamic_pointer_cast<Element>(new_el);
        //cout<<"reached here??"<<endl;
        auto range_element = dynamic_pointer_cast<RangeElement>(new_el);
        //cout<<"reached here???"<<endl;
        auto gn = dynamic_pointer_cast<GroupNode>(new_el);
        //cout<<gn<<endl;

        if (!element && !range_element && !gn) {
            //cout<<"Invalid node type for curly brace quantifier"<<endl;
            throw runtime_error("Invalid node type for curly brace quantifier");
        }
        //cout<<"reached here????"<<endl;
        

        if (dynamic_cast<regex::RightCurlyBrace*>(curr_tkn)) {
            //cases }
            
            if (element) {
                element->min = stoi(val_1);
                element->max = stoi(val_1);
            } else if (range_element) {
                range_element->min = stoi(val_1);
                range_element->max = stoi(val_1);
            } else if (gn){
                gn->min = stoi(val_1);
                gn->max = stoi(val_1);
            }

            next_tkn();
            return;
        }
        //cases: ,}  ,3}

        next_tkn();//past the comma
        //cases: }  3}

        if (dynamic_cast<regex::RightCurlyBrace*>(curr_tkn)) {
            val_2 = "inf"; // Use "inf" to represent infinity
            //cout << "val_2 is " << val_2 << endl;

            if (element) {
                //cout << "Setting element min and max" << endl;
                element->min = stoi(val_1);
                element->max = numeric_limits<int>::max(); // Use max() for infinity
            } else if (range_element) {
                //cout << "Setting range_element min and max" << endl;
                range_element->min = stoi(val_1);
                range_element->max = numeric_limits<int>::max(); // Use max() for infinity
            } else if (gn){
                gn->min = stoi(val_1);
                gn->max = numeric_limits<int>::max();
            }

            next_tkn(); // Move past the right curly brace
            return;
        }

        //cout << "Current token: " << (curr_tkn ? curr_tkn->char_ : "nullptr") << endl;
        while (dynamic_cast<regex::ElementToken*>(curr_tkn)) {
            auto shit = dynamic_cast<regex::ElementToken*>(curr_tkn);
            val_2 += shit->char_;
            next_tkn();
            //cout << "Current token: " << (curr_tkn ? curr_tkn->char_ : "nullptr") << endl;
        }
        if (val_2.empty()) val_2 = to_string(numeric_limits<int>::infinity());
        //cout << "val_2 is " << val_2 << endl;
        if (!dynamic_cast<regex::RightCurlyBrace*>(curr_tkn)) {
            throw runtime_error("Invalid curly brace syntax: expected closing brace");
        }

        next_tkn(); // Skip the closing brace
        //cout << "Current token: " << (curr_tkn ? curr_tkn->char_ : "nullptr") << endl;

        if (element) {
            element->min = stoi(val_1);
            element->max = stoi(val_2);
        } else if (range_element) {
            range_element->min = stoi(val_1);
            range_element->max = stoi(val_2);
        } else if (gn){
                gn->min = stoi(val_1);
                gn->max = stoi(val_2);
        }

    } catch (const exception& e) {
        throw runtime_error("Invalid curly brace syntax.");
    }
}






shared_ptr<ASTNode> Parser::parse_range_el() {
    if (dynamic_cast<regex::LeftBracket*>(curr_tkn)) {
        next_tkn();

        auto element = parse_inner_el();
        if (dynamic_cast<regex::RightBracket*>(curr_tkn)) {
            return element;
        } else {
            throw runtime_error("Missing closing ']'.");
        }
    } else {
        //cout << "parse_range_el: reached here" << endl;
        return parse_el();
    }
}

string Parser::get_range_str(char start, char end) {
    string result;
    for (int i = start; i <= end; ++i) {
        result += static_cast<char>(i);
    }
    return result;
}

shared_ptr<RangeElement> Parser::parse_inner_el() {
    string match_str;
    bool positive_logic = true;

    if (dc<regex::NotToken*>(curr_tkn)) {
        positive_logic = false;
        next_tkn();
    }

    char prev_char = '\0';

    while (curr_tkn) {
        if (dc<regex::RightBracket*>(curr_tkn)) break;

        if (dc<regex::SpaceToken*>(curr_tkn)) {
            match_str += dc<regex::SpaceToken*>(curr_tkn)->char_;
            //cout << "This is the space token char in parse_inner_el() " << dc<SpaceToken*>(curr_tkn)->char_ << endl;
            next_tkn();
            continue;
        }

        if (!dynamic_cast<regex::ElementToken*>(curr_tkn)) {
            curr_tkn = new regex::ElementToken(dynamic_cast<regex::Token*>(curr_tkn)->char_);
        }

        regex::Token* peek_tkn = next_tkn(true);

        if (peek_tkn && dc<regex::Dash*>(peek_tkn)) {
            prev_char = dc<regex::ElementToken*>(curr_tkn)->char_[0];
            //cout << "Just before consuming 2 tokens " << prev_char << endl;
            next_tkn();
            //cout << "Yeah consumed 2 tokens here, now on: " << (curr_tkn ? curr_tkn->char_ : "nullptr") << endl;

            regex::Token* next_peek_tkn = next_tkn(true);

            if (dc<regex::RightBracket*>(next_peek_tkn) || dc<regex::SpaceToken*>(next_peek_tkn)) {
                match_str += prev_char;
                match_str += dc<regex::Dash*>(peek_tkn)->char_;
            } else {
                next_tkn();
                if (!curr_tkn) throw runtime_error("Missing closing ']'");

                char end_char = dc<regex::ElementToken*>(curr_tkn)->char_[0];
                //cout << "end char is" << end_char << endl;
                if (prev_char > end_char) {
                    throw runtime_error("Range values reversed.");
                }
                match_str += get_range_str(prev_char, end_char);
            }
        } else {
            match_str += dc<regex::ElementToken*>(curr_tkn)->char_;
            next_tkn();
        }
    }

    sort(match_str.begin(), match_str.end());
    //cout << "match str after sort " << match_str << endl;
    match_str.erase(unique(match_str.begin(), match_str.end()), match_str.end());
    //cout << "match str after an erase " << match_str << endl;

    return make_shared<RangeElement>(match_str, positive_logic);
}

shared_ptr<ASTNode> Parser::parse_el() {
    string group_name;

    // Debug: Print the current token being processed
    //cout << "parse_el: Current token = " << (curr_tkn ? curr_tkn->char_ : "nullptr") << std::endl;

    if (dynamic_cast<regex::ElementToken*>(curr_tkn)) {
        // Debug: Handling ElementToken
        //cout << "parse_el: Found ElementToken, creating Element node with match = " 
                  //<< dynamic_cast<ElementToken*>(curr_tkn)->char_ << std::endl;
        return make_shared<Element>(dynamic_cast<regex::ElementToken*>(curr_tkn)->char_);
    } else if (dynamic_cast<regex::Wildcard*>(curr_tkn)) {
        // Debug: Handling Wildcard
        //cout << "parse_el: Found Wildcard, creating WildcardElement node" << std::endl;
        return make_shared<WildcardElement>();
    } else if (dynamic_cast<regex::SpaceToken*>(curr_tkn)) {
        // Debug: Handling SpaceToken
        //cout << "parse_el: Found SpaceToken, creating SpaceElement node" << std::endl;
        return make_shared<SpaceElement>();
    } else if (dynamic_cast<regex::LeftParenthesis*>(curr_tkn)) {
        // Debug: Handling LeftParenthesis (start of a group)
        //cout << "parse_el: Found LeftParenthesis, starting group parsing" << std::endl;

        next_tkn(); // Move past the '('
        bool capturing = true;

        // Debug: Check for non-capturing group or named group
        if (dynamic_cast<regex::QuestionMark*>(curr_tkn)) {
            //cout << "parse_el: Found QuestionMark, checking for non-capturing or named group" << std::endl;
            next_tkn(); // Move past the '?'

            if (dynamic_cast<regex::ElementToken*>(curr_tkn) && dynamic_cast<regex::ElementToken*>(curr_tkn)->char_ == ":") {
                // Debug: Non-capturing group
                //cout << "parse_el: Found ':', marking group as non-capturing" << std::endl;
                capturing = false;
                next_tkn(); // Move past the ':'
            } else if (dynamic_cast<regex::ElementToken*>(curr_tkn) && dynamic_cast<regex::ElementToken*>(curr_tkn)->char_ == "<") {
                // Debug: Named group
                //cout << "parse_el: Found '<', parsing named group" << std::endl;
                next_tkn(); // Move past the '<'
                group_name = parse_group_name();
                //cout << "parse_el: Parsed group name = " << group_name << std::endl;
            } else {
                // Debug: Invalid group syntax
                //cout << "parse_el: Invalid group syntax after '?'" << std::endl;
                throw runtime_error("Invalid group syntax.");
            }
        }

        // Parse the group contents
        //cout << "parse_el: Parsing group contents" << std::endl;
        auto res = parse_re_seq(capturing, group_name);

        // Debug: Check for closing parenthesis
        if (dynamic_cast<regex::RightParenthesis*>(curr_tkn)) {
            //cout << "parse_el: Found RightParenthesis, group parsing complete" << std::endl;
            return res;
        } else {
            // Debug: Missing closing parenthesis
            //cout << "parse_el: Missing closing group parenthesis ')'" << std::endl;
            throw runtime_error("Missing closing group parenthesis ')'.");
        }
    } else {
        // Debug: Unhandled token
        //cout << "parse_el: Unhandled token = " << (curr_tkn ? curr_tkn->char_ : "nullptr") << std::endl;
        throw runtime_error("Unescaped special character");
    }
}

string Parser::parse_group_name() {
    if(!curr_tkn){
        throw runtime_error("Unterminated named group name.");
    }
    string group_name;

    while (curr_tkn && dynamic_cast<regex::ElementToken*>(curr_tkn) && dynamic_cast<regex::ElementToken*>(curr_tkn)->char_ != ">") {
        group_name += dynamic_cast<regex::ElementToken*>(curr_tkn)->char_;
        next_tkn();
    }
    
    if(group_name.empty()) throw runtime_error("Unexpected empty named group name.");
    
    //consume the > token
    if (curr_tkn && dynamic_cast<regex::ElementToken*>(curr_tkn) && dynamic_cast<regex::ElementToken*>(curr_tkn)->char_ == ">") {
        next_tkn();
    } else {
        throw runtime_error("Expected '>' to terminate named group name.");
    }
    
    return group_name;
}