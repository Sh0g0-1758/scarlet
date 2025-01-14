#include "lexer.hh"
namespace scarlet {
namespace lexer {

void lexer::tokenize() {
<<<<<<< HEAD
  std::ifstream file(file_path);
  if (!file.is_open()) {
    success = false;
    return;
  }

  char ch;
  while (file.get(ch)) {
    if (ch == '(') {
      tokens.emplace_back(token::TOKEN::OPEN_PARANTHESES);
    } else if (ch == ')') {
      tokens.emplace_back(token::TOKEN::CLOSE_PARANTHESES);
    } else if (ch == '{') {
      tokens.emplace_back(token::TOKEN::OPEN_BRACE);
    } else if (ch == '}') {
      tokens.emplace_back(token::TOKEN::CLOSE_BRACE);
    } else if (ch == ';') {
      tokens.emplace_back(token::TOKEN::SEMICOLON);
    } else if (ch == '~') {
      tokens.emplace_back(token::TOKEN::TILDE);
    } else if (ch == '+') {
      tokens.emplace_back(token::TOKEN::PLUS);
    } else if (ch == '*') {
      tokens.emplace_back(token::TOKEN::ASTERISK);
    } else if (ch == '/') {
      tokens.emplace_back(token::TOKEN::FORWARD_SLASH);
    } else if (ch == '%') {
      tokens.emplace_back(token::TOKEN::PERCENT_SIGN);
    } else if (ch == '-') {
      file.get(ch);
      if (ch == '-') {
        tokens.emplace_back(token::TOKEN::DECREMENT_OPERATOR);
      } else {
        file.seekg(-1, std::ios::cur);
        tokens.emplace_back(token::TOKEN::HYPHEN);
      }
    } else if (regex.matchWord(ch)) {
      std::string identifier;
      while (regex.matchWord(ch) || regex.matchDigit(ch)) {
        identifier += ch;
        file.get(ch);
      }
      file.seekg(-1, std::ios::cur);
      if (identifier == "int") {
        tokens.emplace_back(token::TOKEN::INT);
      } else if (identifier == "void") {
        tokens.emplace_back(token::TOKEN::VOID);
      } else if (identifier == "return") {
        tokens.emplace_back(token::TOKEN::RETURN);
      } else {
        tokens.emplace_back(token::Token(token::TOKEN::IDENTIFIER, identifier));
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
        tokens.emplace_back(token::TOKEN::UNKNOWN);
      } else {
        tokens.emplace_back(token::Token(token::TOKEN::CONSTANT, constant));
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
        tokens.emplace_back(token::TOKEN::UNKNOWN);
      }
    } else {
      success = false;
      tokens.emplace_back(token::TOKEN::UNKNOWN);
    }
=======
  if (file_contents.empty())
    return;
  if (file_contents[0] == '(') {
    tokens.emplace_back(scarlet::TOKEN::OPEN_PARANTHESES);
    file_contents.erase(0, 1);
    tokenize();
  } else if (file_contents[0] == ')') {
    tokens.emplace_back(scarlet::TOKEN::CLOSE_PARANTHESES);
    file_contents.erase(0, 1);
    tokenize();
  } else if (file_contents[0] == '{') {
    tokens.emplace_back(scarlet::TOKEN::OPEN_BRACE);
    file_contents.erase(0, 1);
    tokenize();
  } else if (file_contents[0] == '}') {
    tokens.emplace_back(scarlet::TOKEN::CLOSE_BRACE);
    file_contents.erase(0, 1);
    tokenize();
  } else if (file_contents[0] == ';') {
    tokens.emplace_back(scarlet::TOKEN::SEMICOLON);
    file_contents.erase(0, 1);
    tokenize();
  } else if (file_contents[0] == '~') {
    tokens.emplace_back(scarlet::TOKEN::TILDE);
    file_contents.erase(0, 1);
    tokenize();
  } else if (file_contents[0] == '-') {
    if (file_contents[1] == '-') {
      tokens.emplace_back(scarlet::TOKEN::DECREMENT_OPERATOR);
      file_contents.erase(0, 2);
      tokenize();
    } else {
      tokens.emplace_back(scarlet::TOKEN::HYPHEN);
      file_contents.erase(0, 1);
      tokenize();
    }
  } else if (regex.matchWord(file_contents[0])) {
    std::string identifier;
    while (regex.matchWord(file_contents[0])) {
      identifier += file_contents[0];
      file_contents.erase(0, 1);
    }
    if (regex.matchDigit(file_contents[0])) {
      success = false;
      tokens.emplace_back(scarlet::TOKEN::UNKNOWN);
      tokenize();
    } else {
      if (identifier == "int") {
        tokens.emplace_back(scarlet::TOKEN::INT);
      } else if (identifier == "void") {
        tokens.emplace_back(scarlet::TOKEN::VOID);
      } else if (identifier == "return") {
        tokens.emplace_back(scarlet::TOKEN::RETURN);
      } else {
        tokens.emplace_back(scarlet::Token(scarlet::TOKEN::IDENTIFIER, identifier));
      }
      tokenize();
    }
  } else if (regex.matchDigit(file_contents[0])) {
    std::string constant;
    while (regex.matchDigit(file_contents[0])) {
      constant += file_contents[0];
      file_contents.erase(0, 1);
    }
    if (regex.matchWord(file_contents[0])) {
      success = false;
      tokens.emplace_back(scarlet::TOKEN::UNKNOWN);
      tokenize();
    } else {
      tokens.emplace_back(scarlet::Token(scarlet::TOKEN::CONSTANT, constant));
      tokenize();
    }
  } else if (file_contents[0] == '\n' or file_contents[0] == ' ' or
             file_contents[0] == '\t') {
    file_contents.erase(0, 1);
    tokenize();
  } else if (file_contents[0] == '/') {
    if (file_contents[1] == '/') {
      while (file_contents[0] != '\n') {
        file_contents.erase(0, 1);
      }
      file_contents.erase(0, 1);
      tokenize();
    } else {
      success = false;
      tokens.emplace_back(scarlet::TOKEN::UNKNOWN);
      file_contents.erase(0, 1);
      tokenize();
    }
  } else {
    success = false;
    tokens.emplace_back(scarlet::TOKEN::UNKNOWN);
    file_contents.erase(0, 1);
    tokenize();
>>>>>>> 5d3a511 (add regex engine and created namespace for tokens)
  }
}

void lexer::print_tokens() {
  for (auto token : tokens)
    print_token(token.get_token());
  std::cout << std::endl;
}

<<<<<<< HEAD
std::vector<token::Token> lexer::get_tokens() { return tokens; }
} // namespace lexer
} // namespace scarlet
=======
void lexer::read_file(const std::string &file_path) {
  std::ifstream file(file_path);
  if (file.is_open()) {
    std::string line;
    while (std::getline(file, line)) {
      file_contents += line;
      file_contents += '\n';
    }
    file.close();
  }
}

std::vector<scarlet::Token> lexer::get_tokens() { return tokens; }
>>>>>>> 5d3a511 (add regex engine and created namespace for tokens)
