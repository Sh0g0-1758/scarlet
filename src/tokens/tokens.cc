#include "tokens.hh"

regex::ElementToken::ElementToken(const std::string& ch) {
    char_ = ch;
}

regex::WildcardToken::WildcardToken(const std::string& ch) {
    char_ = ch;
}

regex::Wildcard::Wildcard() : WildcardToken(".") {}

regex::SpaceToken::SpaceToken(char ch) {
    char_ = ' ';
}

regex::StartToken::StartToken(const std::string& ch) {
    char_ = ch;
}

regex::Start::Start() : StartToken("^") {}

regex::EndToken::EndToken(const std::string& ch) {
    char_ = ch;
}

regex::End::End() : EndToken("$") {}

regex::Escape::Escape(const std::string& ch) {
    char_ = '\\';
}

regex::Comma::Comma() {
    char_ = ',';
}

regex::Parenthesis::Parenthesis() {}

regex::LeftParenthesis::LeftParenthesis() {
    char_ = '(';
}

regex::RightParenthesis::RightParenthesis() {
    char_ = ')';
}

regex::CurlyBrace::CurlyBrace() {}

regex::LeftCurlyBrace::LeftCurlyBrace() {
    char_ = '{';
}

regex::RightCurlyBrace::RightCurlyBrace() {
    char_ = '}';
}

regex::Bracket::Bracket() {}

regex::LeftBracket::LeftBracket() {
    char_ = '[';
}

regex::RightBracket::RightBracket() {
    char_ = ']';
}

regex::Quantifier::Quantifier(const std::string& ch) {
    char_ = ch;
}

regex::ZeroOrMore::ZeroOrMore(const std::string& ch) : Quantifier(ch) {}

regex::OneOrMore::OneOrMore(const std::string& ch) : Quantifier(ch) {}

regex::ZeroOrOne::ZeroOrOne(const std::string& ch) : Quantifier(ch) {}

regex::Asterisk::Asterisk() : ZeroOrMore("*") {}

regex::Plus::Plus() : OneOrMore("+") {}

regex::QuestionMark::QuestionMark() : ZeroOrOne("?") {}

regex::OrToken::OrToken(const std::string& ch) {
    char_ = ch;
}

regex::VerticalBar::VerticalBar() : OrToken("|") {}

regex::NotToken::NotToken(const std::string& ch) {
    char_ = ch;
}

regex::Circumflex::Circumflex() : NotToken("^") {}

regex::Dash::Dash() {
    char_ = "-";
}