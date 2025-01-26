#include "lexer.hh"

namespace Scarlet{
namespace Lexer {
void lexer::tokenize() {
  std::ifstream file(file_path);
  if (!file.is_open()) {
    success = false;
    return;
  }

  char ch;
  while (file.get(ch)) {
    if (ch == '(') {
      tokens.emplace_back(Token::TOKEN::OPEN_PARANTHESES);
    } else if (ch == ')') {
      tokens.emplace_back(Token::TOKEN::CLOSE_PARANTHESES);
    } else if (ch == '{') {
      tokens.emplace_back(Token::TOKEN::OPEN_BRACE);
    } else if (ch == '}') {
      tokens.emplace_back(Token::TOKEN::CLOSE_BRACE);
    } else if (ch == ';') {
      tokens.emplace_back(Token::TOKEN::SEMICOLON);
    } else if (ch == '~') {
      tokens.emplace_back(Token::TOKEN::TILDE);
    } else if (ch == '+') {
      tokens.emplace_back(Token::TOKEN::PLUS);
    } else if (ch == '*') {
      tokens.emplace_back(Token::TOKEN::ASTERISK);
    } else if (ch == '/') {
      tokens.emplace_back(Token::TOKEN::FORWARD_SLASH);
    } else if (ch == '%') {
      tokens.emplace_back(Token::TOKEN::PERCENT_SIGN);
    } else if (ch == '-') {
      file.get(ch);
      if (ch == '-') {
        tokens.emplace_back(Token::TOKEN::DECREMENT_OPERATOR);
      } else {
        file.seekg(-1, std::ios::cur);
        tokens.emplace_back(Token::TOKEN::HYPHEN);
      }
    } else if (regex.matchWord(ch)) {
      std::string identifier;
      while (regex.matchWord(ch) || regex.matchDigit(ch)) {
        identifier += ch;
        file.get(ch);
      }
      file.seekg(-1, std::ios::cur);
      if (identifier == "int") {
        tokens.emplace_back(Token::TOKEN::INT);
      } else if (identifier == "void") {
        tokens.emplace_back(Token::TOKEN::VOID);
      } else if (identifier == "return") {
        tokens.emplace_back(Token::TOKEN::RETURN);
      } else {
        tokens.emplace_back(Token::Token(Token::TOKEN::IDENTIFIER, identifier));
      }
    } else if (regex.matchDigit(ch)) {
      std::string constant;
      while (regex.matchDigit(ch)) {
        constant += ch;
        file.get(ch);
      }
      file.seekg(-1, std::ios::cur);
      if (regex.matchWord(ch)) {
        success = false;
        tokens.emplace_back(Token::TOKEN::UNKNOWN);
      } else {
        tokens.emplace_back(Token::Token(Token::TOKEN::CONSTANT, constant));
      }
    } else if (ch == '\n' or ch == ' ' or ch == '\t') {
      // do nothing
    } else if (ch == '/') {
      file.get(ch);
      if (ch == '/') {
        while (ch != '\n') {
          file.get(ch);
        }
        file.seekg(-1, std::ios::cur);
      } else {
        file.seekg(-1, std::ios::cur);
        success = false;
        tokens.emplace_back(Token::TOKEN::UNKNOWN);
      }
    } else {
      success = false;
      tokens.emplace_back(Token::TOKEN::UNKNOWN);
    }
  }
}

void lexer::print_tokens() {
  for (auto token : tokens)
    print_token(token.get_token());
  std::cout << std::endl;
}

std::vector<Token::Token> lexer::get_tokens() { return tokens; }
}
}
