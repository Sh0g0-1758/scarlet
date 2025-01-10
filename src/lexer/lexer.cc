#include "lexer.hh"


bool Lexer::isDigit(char ch) const {
    return digits.find(ch) != string::npos;
}

vector<regex::Token*> Lexer::scan(const string& re) {
    vector<regex::Token*> tokens;

    auto append = [&tokens](regex::Token* elem) {
        tokens.push_back(elem);
    };

    size_t i = 0;
    bool escape_found = false;

    while (i < re.size()) {
        char ch = re[i];

        if (escape_found) {
            // Handle escape sequences
            if (ch == 't') {
                append(new regex::ElementToken("\t"));
            } else if (ch == 's') {
                append(new regex::SpaceToken(ch));
            } else {
                string str(1, ch);
                append(new regex::ElementToken(str));
            }
        } else if (ch == '\\') {
            escape_found = true;
            ++i;
            continue;
        } else if (ch == '.') {
            append(new regex::Wildcard());
        } else if (ch == '(') {
            append(new regex::LeftParenthesis());
        } else if (ch == ')') {
            append(new regex::RightParenthesis());
        } else if (ch == '[') {
            append(new regex::LeftBracket());
        } else if (ch == '-') {
            append(new regex::Dash());
        } else if (ch == ']') {
            append(new regex::RightBracket());
        } else if (ch == '{') {
            append(new regex::LeftCurlyBrace());
            ++i;
            while (i < re.size()) {
                ch = re[i];
                if (ch == ',') {
                    append(new regex::Comma());
                } else if (isDigit(ch)) {
                    append(new regex::ElementToken(string(1, ch)));
                } else if (ch == '}') {
                    append(new regex::RightCurlyBrace());
                    break;
                } else {
                    throw invalid_argument("Bad token at index " + to_string(i));
                }
                ++i;
            }
        } else if (ch == '^') {
            if (i == 0) {
                append(new regex::Start());
            } else {
                append(new regex::Circumflex());
            }
        } else if (ch == '$') {
            append(new regex::End());
        } else if (ch == '?') {
            append(new regex::QuestionMark());
        } else if (ch == '*') {
            append(new regex::Asterisk());
        } else if (ch == '+') {
            append(new regex::Plus());
        } else if (ch == '|') {
            append(new regex::VerticalBar());
        } else if (ch == '}') {
            append(new regex::RightCurlyBrace());
        } else {
            append(new regex::ElementToken(string(1, ch)));
        }

        escape_found = false;
        ++i;
    }
    // for (const auto& token : tokens) {
    //     cerr << "Token: " << token->char_ << " (Type: " << typeid(*token).name() << ")" << endl;
    // }

    return tokens;
}