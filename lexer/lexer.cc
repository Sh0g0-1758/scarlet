#include "lexer.hh"
namespace scarlet {
namespace lexer {
// clang check
void lexer::tokenize() {
  std::ifstream file(file_path);
  if (!file.is_open()) {
    success = false;
    return;
  }

  char ch;
  while (file.get(ch)) {
    if (ch == '(') {
      tokens.emplace_back(token::Token(token::TOKEN::OPEN_PARANTHESES,
                                       token::TOKEN_TYPE::SPECIAL_SYMBOLS));
    } else if (ch == ')') {
      tokens.emplace_back(token::Token(token::TOKEN::CLOSE_PARANTHESES,token::TOKEN_TYPE::SPECIAL_SYMBOLS);
    } else if (ch == '{') {
      tokens.emplace_back(token::Token(token::TOKEN::OPEN_BRACE,
                                       token::TOKEN_TYPE::SPECIAL_SYMBOLS));
    } else if (ch == '}') {
      tokens.emplace_back(token::Token(token::TOKEN::CLOSE_BRACE,
                                       token::TOKEN_TYPE::SPECIAL_SYMBOLS));
    } else if (ch == ';') {
      tokens.emplace_back(token::Token(token::TOKEN::SEMICOLON,
                                       token::TOKEN_TYPE::SPECIAL_SYMBOLS));
    } else if (ch == ':') {
      tokens.emplace_back(token::Token(token::TOKEN::COLON,
                                       token::TOKEN_TYPE::SPECIAL_SYMBOLS));
    } else if (ch == '?') {
      tokens.emplace_back(token::Token(token::TOKEN::QUESTION_MARK,
                                       token::TOKEN_TYPE::OPERATORS));
    } else if (ch == ',') {
      tokens.emplace_back(
          token::Token(token::TOKEN::COMMA, token::TOKEN_TYPE::OPERATORS));
    } else if (ch == '~') {
      tokens.emplace_back(
          token::Token(token::TOKEN::TILDE, token::TOKEN_TYPE::OPERATORS));
    } else if (ch == '+') {
      tokens.emplace_back(
          token::Token(token::TOKEN::PLUS, token::TOKEN_TYPE::KEYWORDS));
    } else if (ch == '*') {
      tokens.emplace_back(
          token::Token(token::TOKEN::ASTERISK, token::TOKEN_TYPE::KEYWORDS));
    } else if (ch == '/') {
      tokens.emplace_back(token::Token(token::TOKEN::FORWARD_SLASH,
                                       token::TOKEN_TYPE::KEYWORDS));
    } else if (ch == '%') {
      tokens.emplace_back(token::Token(token::TOKEN::PERCENT_SIGN,
                                       token::TOKEN_TYPE::KEYWORDS));
    } else if (ch == '&') {
      file.get(ch);
      if (ch == '&') {
        tokens.emplace_back(token::TOKEN::LAND);
      } else {
        file.seekg(-1, std::ios::cur);
        tokens.emplace_back(token::TOKEN::AAND);
      }
    } else if (ch == '|') {
      file.get(ch);
      if (ch == '|') {
        tokens.emplace_back(token::TOKEN::LOR);
      } else {
        file.seekg(-1, std::ios::cur);
        tokens.emplace_back(token::TOKEN::AOR);
      }
    } else if (ch == '^') {
      tokens.emplace_back(token::TOKEN::XOR);
    } else if (ch == '!') {
      file.get(ch);
      if (ch == '=') {
        tokens.emplace_back(token::TOKEN::NOTEQUAL);
      } else {
        file.seekg(-1, std::ios::cur);
        tokens.emplace_back(token::TOKEN::NOT);
      }
    } else if (ch == '=') {
      file.get(ch);
      if (ch == '=') {
        tokens.emplace_back(token::TOKEN::EQUAL);
      } else {
        file.seekg(-1, std::ios::cur);
        tokens.emplace_back(token::TOKEN::ASSIGNMENT);
      }
    } else if (ch == '>') {
      file.get(ch);
      if (ch == '>') {
        tokens.emplace_back(token::TOKEN::RIGHT_SHIFT);
      } else if (ch == '=') {
        tokens.emplace_back(token::TOKEN::GREATERTHANEQUAL);
      } else {
        file.seekg(-1, std::ios::cur);
        tokens.emplace_back(token::TOKEN::GREATERTHAN);
      }
    } else if (ch == '<') {
      file.get(ch);
      if (ch == '<') {
        tokens.emplace_back(token::TOKEN::LEFT_SHIFT);
      } else if (ch == '=') {
        tokens.emplace_back(token::TOKEN::LESSTHANEQUAL);
      } else {
        file.seekg(-1, std::ios::cur);
        tokens.emplace_back(token::TOKEN::LESSTHAN);
      }
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
      } else if (identifier == "if") {
        tokens.emplace_back(token::TOKEN::IF);
      } else if (identifier == "else") {
        tokens.emplace_back(token::TOKEN::ELSE);
      } else if (identifier == "do") {
        tokens.emplace_back(token::TOKEN::DO);
      } else if (identifier == "while") {
        tokens.emplace_back(token::TOKEN::WHILE);
      } else if (identifier == "for") {
        tokens.emplace_back(token::TOKEN::FOR);
      } else if (identifier == "break") {
        tokens.emplace_back(token::TOKEN::BREAK);
      } else if (identifier == "continue") {
        tokens.emplace_back(token::TOKEN::CONTINUE);
      } else if (identifier == "static") {
        tokens.emplace_back(token::TOKEN::STATIC);
      } else if (identifier == "extern") {
        tokens.emplace_back(token::TOKEN::EXTERN);
      } else if (identifier == "long") {
        tokens.emplace_back(token::TOKEN::LONG);
      } else if (identifier == "signed") {
        tokens.emplace_back(token::TOKEN::SIGNED);
      } else if (identifier == "unsigned") {
        tokens.emplace_back(token::TOKEN::UNSIGNED);
      } else {
        tokens.emplace_back(token::Token(token::TOKEN::IDENTIFIER, identifier));
      }
    } else if (regex.matchDigit(ch) or ch == '.') {
      std::string constant;
      while (regex.matchDigit(ch)) {
        constant += ch;
        file.get(ch);
      }
      std::string literal_suffix = "";
      if (ch == 'l' || ch == 'L' || ch == 'u' || ch == 'U') {
        literal_suffix += ch;
        char prev_ch = ch;
        file.get(ch);
        if (ch == 'l' || ch == 'L' || ch == 'u' || ch == 'U') {
          if (std::toupper(prev_ch) != std::toupper(ch)) {
            literal_suffix += ch;
            file.get(ch);
          }
        }
      }
      constant += literal_suffix;
      if (ch == '.' and literal_suffix.size() == 0) {
        constant += '.';
        file.get(ch);
        while (regex.matchDigit(ch)) {
          constant += ch;
          file.get(ch);
        }
      }
      if ((ch == 'e' or ch == 'E') and literal_suffix.size() == 0) {
        file.get(ch);
        if (ch == '+' or ch == '-') {
          constant += ch;
          file.get(ch);
        }
        if (regex.matchDigit(ch)) {
          while (regex.matchDigit(ch)) {
            constant += ch;
            file.get(ch);
          }
        } else {
          success = false;
          tokens.emplace_back(token::TOKEN::UNKNOWN);
        }
      }

      if (regex.matchWord(ch) or ch == '.') {
        success = false;
        tokens.emplace_back(token::TOKEN::UNKNOWN);
      } else {
        tokens.emplace_back(token::Token(token::TOKEN::CONSTANT, constant));
      }
      file.seekg(-1, std::ios::cur);
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
  }
}

void lexer::print_tokens() {
  for (auto token : tokens)
    print_token(token.get_token());
  std::cout << std::endl;
}

std::vector<token::Token> lexer::get_tokens() { return tokens; }
} // namespace lexer
} // namespace scarlet
