#include "lexer.hh"

void lexer::tokenize() {
  std::ifstream file(file_path);
  if (!file.is_open()) {
    success = false;
    return;
  }

  char ch;
  while (file.get(ch)) {
    if (ch == '(') {
      tokens.emplace_back(TOKEN::OPEN_PARANTHESES);
    } else if (ch == ')') {
      tokens.emplace_back(TOKEN::CLOSE_PARANTHESES);
    } else if (ch == '{') {
      tokens.emplace_back(TOKEN::OPEN_BRACE);
    } else if (ch == '}') {
      tokens.emplace_back(TOKEN::CLOSE_BRACE);
    } else if (ch == ';') {
      tokens.emplace_back(TOKEN::SEMICOLON);
    } else if (ch == '~') {
      tokens.emplace_back(TOKEN::TILDE);
    } else if (ch == '+') {
      tokens.emplace_back(TOKEN::PLUS);
    } else if (ch == '*') {
      tokens.emplace_back(TOKEN::ASTERISK);
    } else if (ch == '/') {
      tokens.emplace_back(TOKEN::FORWARD_SLASH);
    } else if (ch == '%') {
      tokens.emplace_back(TOKEN::PERCENT_SIGN);
    } else if (ch == '-') {
      file.get(ch);
      if (ch == '-') {
        tokens.emplace_back(TOKEN::DECREMENT_OPERATOR);
      } else {
        file.seekg(-1, std::ios::cur);
        tokens.emplace_back(TOKEN::HYPHEN);
      }
    } else if (regex.matchWord(ch)) {
      std::string identifier;
      while (regex.matchWord(ch) || regex.matchDigit(ch)) {
        identifier += ch;
        file.get(ch);
      }
      file.seekg(-1, std::ios::cur);
      if (identifier == "int") {
        tokens.emplace_back(TOKEN::INT);
      } else if (identifier == "void") {
        tokens.emplace_back(TOKEN::VOID);
      } else if (identifier == "return") {
        tokens.emplace_back(TOKEN::RETURN);
      } else {
        tokens.emplace_back(Token(TOKEN::IDENTIFIER, identifier));
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
        tokens.emplace_back(TOKEN::UNKNOWN);
      } else {
        tokens.emplace_back(Token(TOKEN::CONSTANT, constant));
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
        tokens.emplace_back(TOKEN::UNKNOWN);
      }
    } else {
      success = false;
      tokens.emplace_back(TOKEN::UNKNOWN);
    }
  }
}

void lexer::print_tokens() {
  for (auto token : tokens)
    print_token(token.get_token());
  std::cout << std::endl;
}

std::vector<Token> lexer::get_tokens() { return tokens; }
