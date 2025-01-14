#include "tokens.hh"

ElementToken::ElementToken(const std::string& ch) {
    char_ = ch;
}

WildcardToken::WildcardToken(const std::string& ch) {
    char_ = ch;
}

Wildcard::Wildcard() : WildcardToken(".") {}

SpaceToken::SpaceToken(char ch) {
    char_ = ' ';
}

StartToken::StartToken(const std::string& ch) {
    char_ = ch;
}

Start::Start() : StartToken("^") {}

EndToken::EndToken(const std::string& ch) {
    char_ = ch;
}

End::End() : EndToken("$") {}

Escape::Escape(const std::string& ch) {
    char_ = '\\';
}

Comma::Comma() {
    char_ = ',';
}

Parenthesis::Parenthesis() {}

LeftParenthesis::LeftParenthesis() {
    char_ = '(';
}

RightParenthesis::RightParenthesis() {
    char_ = ')';
}

CurlyBrace::CurlyBrace() {}

LeftCurlyBrace::LeftCurlyBrace() {
    char_ = '{';
}

RightCurlyBrace::RightCurlyBrace() {
    char_ = '}';
}

Bracket::Bracket() {}

LeftBracket::LeftBracket() {
    char_ = '[';
}

RightBracket::RightBracket() {
    char_ = ']';
}

Quantifier::Quantifier(const std::string& ch) {
    char_ = ch;
}

ZeroOrMore::ZeroOrMore(const std::string& ch) : Quantifier(ch) {}

OneOrMore::OneOrMore(const std::string& ch) : Quantifier(ch) {}

ZeroOrOne::ZeroOrOne(const std::string& ch) : Quantifier(ch) {}

Asterisk::Asterisk() : ZeroOrMore("*") {}

Plus::Plus() : OneOrMore("+") {}

QuestionMark::QuestionMark() : ZeroOrOne("?") {}

OrToken::OrToken(const std::string& ch) {
    char_ = ch;
}

VerticalBar::VerticalBar() : OrToken("|") {}

NotToken::NotToken(const std::string& ch) {
    char_ = ch;
}

Circumflex::Circumflex() : NotToken("^") {}

Dash::Dash() {
    char_ = "-";
}