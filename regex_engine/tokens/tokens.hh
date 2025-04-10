#ifndef TOKENS_H
#define TOKENS_H

#include <cassert>
#include <iostream>
#include <memory>
#include <string>
using namespace std;

class Token {
public:
  Token() : char_("") {}
  virtual ~Token() {}

  std::string char_;
};

class ElementToken : public Token {
public:
  ElementToken(const std::string &ch);
};

class WildcardToken : public Token {
public:
  WildcardToken(const std::string &ch);
};

class Wildcard : public WildcardToken {
public:
  Wildcard();
};

class SpaceToken : public Token {
public:
  SpaceToken(char ch);
};

class StartToken : public Token {
public:
  StartToken(const std::string &ch);
};

class Start : public StartToken {
public:
  Start();
};

class EndToken : public Token {
public:
  EndToken(const std::string &ch);
};

class End : public EndToken {
public:
  End();
};

class Escape : public Token {
public:
  Escape(const std::string &ch);
};

class Comma : public Token {
public:
  Comma();
};

class Parenthesis : public Token {
public:
  Parenthesis();
};

class LeftParenthesis : public Parenthesis {
public:
  LeftParenthesis();
};

class RightParenthesis : public Parenthesis {
public:
  RightParenthesis();
};

class CurlyBrace : public Token {
public:
  CurlyBrace();
};

class LeftCurlyBrace : public CurlyBrace {
public:
  LeftCurlyBrace();
};

class RightCurlyBrace : public CurlyBrace {
public:
  RightCurlyBrace();
};

class Bracket : public Token {
public:
  Bracket();
};

class LeftBracket : public Bracket {
public:
  LeftBracket();
};

class RightBracket : public Bracket {
public:
  RightBracket();
};

class Quantifier : public Token {
public:
  Quantifier(const std::string &ch);
};

class ZeroOrMore : public Quantifier {
public:
  ZeroOrMore(const std::string &ch);
};

class OneOrMore : public Quantifier {
public:
  OneOrMore(const std::string &ch);
};

class ZeroOrOne : public Quantifier {
public:
  ZeroOrOne(const std::string &ch);
};

class Asterisk : public ZeroOrMore {
public:
  Asterisk();
};

class Plus : public OneOrMore {
public:
  Plus();
};

class QuestionMark : public ZeroOrOne {
public:
  QuestionMark();
};

class OrToken : public Token {
public:
  OrToken(const std::string &ch);
};

class VerticalBar : public OrToken {
public:
  VerticalBar();
};

class NotToken : public Token {
public:
  NotToken(const std::string &ch);
};

class Circumflex : public NotToken {
public:
  Circumflex();
};

class Dash : public Token {
public:
  Dash();
};

#endif // TOKENS_H