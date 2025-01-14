#include "lexer.hh"


bool Lexer::isDigit(char ch) const {
    return digits.find(ch) != string::npos;
}

vector<Token*> Lexer::scan(const string& re) {
    vector<Token*> tokens;

    auto append = [&tokens](Token* elem) {
        tokens.push_back(elem);
    };

    size_t i = 0;
    bool escape_found = false;

    while (i < re.size()) {
        char ch = re[i];

        if (escape_found) {
            // Handle escape sequences
            if (ch == 't') {
                append(new ElementToken("\t"));
            } else if (ch == 's') {
                append(new SpaceToken(ch));
            } else {
                string str(1, ch);
                append(new ElementToken(str));
            }
        } else if (ch == '\\') {
            escape_found = true;
            ++i;
            continue;
        } else if (ch == '.') {
            append(new Wildcard());
        } else if (ch == '(') {
            append(new LeftParenthesis());
        } else if (ch == ')') {
            append(new RightParenthesis());
        } else if (ch == '[') {
            append(new LeftBracket());
        } else if (ch == '-') {
            append(new Dash());
        } else if (ch == ']') {
            append(new RightBracket());
        } else if (ch == '{') {
            append(new LeftCurlyBrace());
            ++i;
            while (i < re.size()) {
                ch = re[i];
                if (ch == ',') {
                    append(new Comma());
                } else if (isDigit(ch)) {
                    append(new ElementToken(string(1, ch)));
                } else if (ch == '}') {
                    append(new RightCurlyBrace());
                    break;
                } else {
                    throw invalid_argument("Bad token at index " + to_string(i));
                }
                ++i;
            }
        } else if (ch == '^') {
            if (i == 0) {
                append(new Start());
            } else {
                append(new Circumflex());
            }
        } else if (ch == '$') {
            append(new End());
        } else if (ch == '?') {
            append(new QuestionMark());
        } else if (ch == '*') {
            append(new Asterisk());
        } else if (ch == '+') {
            append(new Plus());
        } else if (ch == '|') {
            append(new VerticalBar());
        } else if (ch == '}') {
            append(new RightCurlyBrace());
        } else {
            append(new ElementToken(string(1, ch)));
        }

        escape_found = false;
        ++i;
    }
    // for (const auto& token : tokens) {
    //     cerr << "Token: " << token->char_ << " (Type: " << typeid(*token).name() << ")" << endl;
    // }

    return tokens;
}