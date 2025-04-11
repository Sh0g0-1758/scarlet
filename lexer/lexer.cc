#include "lexer.hh"
namespace scarlet {
namespace lexer {

#define ERROR_LOCATION                                                         \
  file_path.substr(0, file_path.length() - 1) + ":" +                          \
      std::to_string(line_number) + ":" + std::to_string(col_number)

void lexer::tokenize() {
  std::ifstream file(file_path);
  if (!file.is_open()) {
    error_recovery.emplace_back(std::make_pair(
        "Unable to open the file> " + file_path, "Please check file path"));
    success = false;
    return;
  }

  char ch;
  while (file.get(ch)) {
    if (ch == '(') {
      tokens.emplace_back(token::TOKEN::OPEN_PARANTHESES);
      col_number++;
    } else if (ch == ')') {
      tokens.emplace_back(token::TOKEN::CLOSE_PARANTHESES);
      col_number++;
    } else if (ch == '{') {
      tokens.emplace_back(token::TOKEN::OPEN_BRACE);
      col_number++;
    } else if (ch == '}') {
      tokens.emplace_back(token::TOKEN::CLOSE_BRACE);
      col_number++;
    } else if (ch == ';') {
      tokens.emplace_back(token::TOKEN::SEMICOLON);
      col_number++;
    } else if (ch == '[') {
      tokens.emplace_back(token::TOKEN::OPEN_BRACKET);
    } else if (ch == ']') {
      tokens.emplace_back(token::TOKEN::CLOSE_BRACKET);
    } else if (ch == ':') {
      tokens.emplace_back(token::TOKEN::COLON);
      col_number++;
    } else if (ch == '?') {
      tokens.emplace_back(token::TOKEN::QUESTION_MARK);
      col_number++;
    } else if (ch == ',') {
      tokens.emplace_back(token::TOKEN::COMMA);
      col_number++;
    } else if (ch == '~') {
      tokens.emplace_back(token::TOKEN::TILDE);
      col_number++;
    } else if (ch == '+') {
      file.get(ch);
      if (ch == '+') {
        tokens.emplace_back(token::TOKEN::INCREMENT_OPERATOR);
        col_number++;
      } else if (ch == '=') {
        tokens.emplace_back(token::TOKEN::COMPOUND_SUM);
        col_number++;
      } else {
        file.seekg(-1, std::ios::cur);
        tokens.emplace_back(token::TOKEN::PLUS);
      }
      col_number++;
    } else if (ch == '*') {
      file.get(ch);
      if (ch == '=') {
        tokens.emplace_back(token::TOKEN::COMPOUND_PRODUCT);
        col_number++;
      } else {
        file.seekg(-1, std::ios::cur);
        tokens.emplace_back(token::TOKEN::ASTERISK);
      }
      col_number++;
    } else if (ch == '/') {
      file.get(ch);
      if (ch == '=') {
        tokens.emplace_back(token::TOKEN::COMPOUND_DIVISION);
        col_number++;
      } else {
        file.seekg(-1, std::ios::cur);
        tokens.emplace_back(token::TOKEN::FORWARD_SLASH);
      }
      col_number++;
    } else if (ch == '%') {
      file.get(ch);
      if (ch == '=') {
        tokens.emplace_back(token::TOKEN::COMPOUND_REMAINDER);
        col_number++;
      } else {
        file.seekg(-1, std::ios::cur);
        tokens.emplace_back(token::TOKEN::PERCENT_SIGN);
      }
      col_number++;
    } else if (ch == '&') {
      file.get(ch);
      if (ch == '=') {
        tokens.emplace_back(token::TOKEN::COMPOUND_AND);
        col_number += 2;
      } else if (ch == '&') {
        tokens.emplace_back(token::TOKEN::LAND);
        col_number += 2;
      } else {
        file.seekg(-1, std::ios::cur);
        tokens.emplace_back(token::TOKEN::AAND);
        col_number++;
      }
    } else if (ch == '|') {
      file.get(ch);
      if (ch == '=') {
        tokens.emplace_back(token::TOKEN::COMPOUND_OR);
        col_number += 2;
      } else if (ch == '|') {
        tokens.emplace_back(token::TOKEN::LOR);
        col_number += 2;
      } else {
        file.seekg(-1, std::ios::cur);
        tokens.emplace_back(token::TOKEN::AOR);
        col_number++;
      }
    } else if (ch == '^') {
      file.get(ch);
      if (ch == '=') {
        tokens.emplace_back(token::TOKEN::COMPOUND_XOR);
        col_number++;
      } else {
        file.seekg(-1, std::ios::cur);
        tokens.emplace_back(token::TOKEN::XOR);
      }
      col_number++;
    } else if (ch == '!') {
      file.get(ch);
      if (ch == '=') {
        tokens.emplace_back(token::TOKEN::NOTEQUAL);
        col_number += 2;
      } else {
        file.seekg(-1, std::ios::cur);
        tokens.emplace_back(token::TOKEN::NOT);
        col_number++;
      }
    } else if (ch == '=') {
      file.get(ch);
      if (ch == '=') {
        tokens.emplace_back(token::TOKEN::EQUAL);
        col_number += 2;
      } else {
        file.seekg(-1, std::ios::cur);
        tokens.emplace_back(token::TOKEN::ASSIGNMENT);
        col_number++;
      }
    } else if (ch == '>') {
      file.get(ch);
      if (ch == '>') {
        file.get(ch);
        if (ch == '=') {
          tokens.emplace_back(token::TOKEN::COMPOUND_RIGHTSHIFT);
          col_number++;
        } else {
          file.seekg(-1, std::ios::cur);
          tokens.emplace_back(token::TOKEN::RIGHT_SHIFT);
        }
        col_number += 2;
      } else if (ch == '=') {
        tokens.emplace_back(token::TOKEN::GREATERTHANEQUAL);
        col_number += 2;
      } else {
        file.seekg(-1, std::ios::cur);
        tokens.emplace_back(token::TOKEN::GREATERTHAN);
        col_number++;
      }
    } else if (ch == '<') {
      file.get(ch);
      if (ch == '<') {
        file.get(ch);
        if (ch == '=') {
          tokens.emplace_back(token::TOKEN::COMPOUND_LEFTSHIFT);
          col_number++;
        } else {
          file.seekg(-1, std::ios::cur);
          tokens.emplace_back(token::TOKEN::LEFT_SHIFT);
        }
        col_number += 2;
      } else if (ch == '=') {
        tokens.emplace_back(token::TOKEN::LESSTHANEQUAL);
        col_number += 2;
      } else {
        file.seekg(-1, std::ios::cur);
        tokens.emplace_back(token::TOKEN::LESSTHAN);
        col_number++;
      }
    } else if (ch == '-') {
      file.get(ch);
      if (ch == '=') {
        tokens.emplace_back(token::TOKEN::COMPOUND_DIFFERENCE);
        col_number += 2;
      } else if (ch == '-') {
        tokens.emplace_back(token::TOKEN::DECREMENT_OPERATOR);
        col_number += 2;
      } else if (ch == '>') {
        tokens.emplace_back(token::TOKEN::ARROW_OPERATOR);
        col_number += 2;
      } else {
        file.seekg(-1, std::ios::cur);
        tokens.emplace_back(token::TOKEN::HYPHEN);
        col_number++;
      }
    } else if (ch == '\'') {
      file.get(ch);
      std::string tmp;
      col_number += 2;
      if (ch == '\\') {
        // parse the next character as escape character
        file.get(ch);
        col_number++;
        if (regex.matchEscape(ch)) {
          tmp += token::char_to_esc(ch);
        } else {
          success = false;
          tokens.emplace_back(token::TOKEN::UNKNOWN);
          error_recovery.emplace_back(
              std::make_pair(ERROR_LOCATION + " " + RED + "error:" + RESET +
                                 " " + "unkown escape character " + tmp,
                             "please use escape characters from c standard"));
        }
      } else if (regex.matchASCIIPrintable(ch) and ch != '\'') {
        tmp += ch;
      } else {
        success = false;
        tokens.emplace_back(token::TOKEN::UNKNOWN);
        error_recovery.emplace_back(
            std::make_pair(ERROR_LOCATION + " " + RED + "error:" + RESET + " " +
                               "unkwown character " + tmp,
                           "please use characters from c standard"));
      }
      file.get(ch);
      col_number++;
      if (ch != '\'' and success != false) {
        success = false;
        tokens.emplace_back(token::TOKEN::UNKNOWN);
        error_recovery.emplace_back(
            std::make_pair(ERROR_LOCATION + " " + RED + "error:" + RESET + " " +
                               "no end quote for character " + tmp,
                           "please add end quotes"));
      } else {
        tokens.emplace_back(
            token::Token(token::TOKEN::CHARACTER_CONSTANT, tmp));
      }
      col_number++;
    } else if (ch == '\"') {
      std::string literal;
      file.get(ch);
      col_number += 2;
      while (ch != '"' and regex.matchASCIIPrintable(ch)) {
        if (ch == '\\') {
          // parse the next character as escape character
          file.get(ch);
          col_number++;
          if (regex.matchEscape(ch)) {
            literal += token::char_to_esc(ch);
          } else {
            success = false;
            tokens.emplace_back(token::TOKEN::UNKNOWN);
            error_recovery.emplace_back(std::make_pair(
                ERROR_LOCATION + " " + RED + "error:" + RESET + " " +
                    "bad escape character in " + literal + ": " + ch,
                "please use escape characters from c standard"));
          }
        } else if (regex.matchASCIIPrintable(ch) and ch != '\\') {
          literal += ch;
        } else { // will this ever be called?
          success = false;
          tokens.emplace_back(token::TOKEN::UNKNOWN);
          error_recovery.emplace_back(std::make_pair(
              ERROR_LOCATION + " " + RED + "error:" + RESET + " " +
                  "unkwown character in " + literal + " : " + ch,
              "please use characters from c standard"));
        }
        file.get(ch);
        col_number++;
      }
      if (ch == '\"') {
        tokens.emplace_back(token::Token(token::TOKEN::CHAR_ARR, literal));
      } else {
        success = false;
        tokens.emplace_back(token::TOKEN::UNKNOWN);
        error_recovery.emplace_back(
            std::make_pair(ERROR_LOCATION + " " + RED + "error:" + RESET + " " +
                               "string missing end quotes " + " \"" + literal,
                           "please add end quotes"));
        file.seekg(-1, std::ios::cur);
      }
    } else if (regex.matchWord(ch)) {
      std::string identifier;
      while (regex.matchWord(ch) || regex.matchDigit(ch)) {
        identifier += ch;
        file.get(ch);
      }
      while (ch == ' ' or ch == '\n') {
        if (ch == '\n') {
          line_number++;
          col_number = 1;
        } else {
          col_number++;
        }
        if (!file.get(ch))
          break;
      }
      // cases when we have a struct and need to
      // access its members
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
      } else if (identifier == "switch") {
        tokens.emplace_back(token::TOKEN::SWITCH);
      } else if (identifier == "case") {
        tokens.emplace_back(token::TOKEN::CASE);
      } else if (identifier == "default") {
        tokens.emplace_back(token::TOKEN::DEFAULT_CASE);
      } else if (identifier == "extern") {
        tokens.emplace_back(token::TOKEN::EXTERN);
      } else if (identifier == "long") {
        tokens.emplace_back(token::TOKEN::LONG);
      } else if (identifier == "signed") {
        tokens.emplace_back(token::TOKEN::SIGNED);
      } else if (identifier == "unsigned") {
        tokens.emplace_back(token::TOKEN::UNSIGNED);
      } else if (identifier == "double") {
        tokens.emplace_back(token::TOKEN::DOUBLE);
      } else if (identifier == "char") {
        tokens.emplace_back(token::TOKEN::CHAR);
      } else if (identifier == "sizeof") {
        tokens.emplace_back(token::TOKEN::SIZEOF);
      } else if (identifier == "struct") {
        tokens.emplace_back(token::TOKEN::STRUCT);
      } else if (identifier == "goto") {
        tokens.emplace_back(token::TOKEN::GOTO);
      } else {
        tokens.emplace_back(token::Token(token::TOKEN::IDENTIFIER, identifier));
      }
      if (ch == '.') {
        tokens.emplace_back(token::TOKEN::DOT);
        col_number++;
      } else {
        file.seekg(-1, std::ios::cur);
      }
      col_number += (identifier.length());
    } else if (regex.matchDigit(ch) or ch == '.') {
      std::string constant;
      while (regex.matchDigit(ch)) {
        constant += ch;
        file.get(ch);
      }
      std::string literal_suffix = "";
      // supported suffixes: l, L, u, U, lu, lU, Lu, LU, ul, uL, Ul, UL
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

      // Treat as a float when there is no proceeding l,L,u,U suffix and next
      // token is dot
      bool isFloat = false;
      if (ch == '.' and literal_suffix.size() == 0) {
        isFloat = true;
        constant += '.';
        file.get(ch);
        while (regex.matchDigit(ch)) {
          constant += ch;
          file.get(ch);
        }
      }

      if ((ch == 'e' or ch == 'E') and literal_suffix.size() == 0) {
        isFloat = true;
        constant += 'e';
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
          col_number += (constant.length());
          error_recovery.emplace_back(std::make_pair(
              ERROR_LOCATION + " " + RED + "error:" + RESET + " " + ch +
                  " follows e/E in a number> " + std::string(1, ch),
              "some digit/+/- must follow a number that has e/E"));
          success = false;
          tokens.emplace_back(token::TOKEN::UNKNOWN);
          file.seekg(-1, std::ios::cur);
          continue;
        }
      }

      // If a character follows a number or if we have two dots in a row
      if (regex.matchWord(ch) or ch == '.') {
        col_number += (constant.length());
        if (ch == '.') {
          error_recovery.emplace_back(
              std::make_pair(ERROR_LOCATION + " " + RED + "error:" + RESET +
                                 " . follows another . in a float number",
                             "a float must have only one ."));
        } else {
          error_recovery.emplace_back(
              std::make_pair(ERROR_LOCATION + " " + RED + "error:" + RESET +
                                 " " + ch + " follows a digit",
                             "characters cannot follow a number"));
        }
        success = false;
        tokens.emplace_back(token::TOKEN::UNKNOWN);
        file.seekg(-1, std::ios::cur);
        continue;
      } else {
        if (isFloat) {
          tokens.emplace_back(
              token::Token(token::TOKEN::DOUBLE_CONSTANT, constant));
        } else {
          if (literal_suffix == "") {
            tokens.emplace_back(
                token::Token(token::TOKEN::INT_CONSTANT, constant));
          } else if (literal_suffix == "l" or literal_suffix == "L") {
            tokens.emplace_back(
                token::Token(token::TOKEN::LONG_CONSTANT, constant));
          } else if (literal_suffix == "u" or literal_suffix == "U") {
            tokens.emplace_back(
                token::Token(token::TOKEN::UINT_CONSTANT, constant));
          } else if (literal_suffix == "lu" or literal_suffix == "lU" or
                     literal_suffix == "Lu" or literal_suffix == "LU" or
                     literal_suffix == "ul" or literal_suffix == "uL" or
                     literal_suffix == "Ul" or literal_suffix == "UL") {
            tokens.emplace_back(
                token::Token(token::TOKEN::ULONG_CONSTANT, constant));
          }
        }
      }
      file.seekg(-1, std::ios::cur);
      col_number += (constant.length());
    } else if (ch == '\n' or ch == ' ') {
      if (ch == '\n') {
        line_number++;
        col_number = 1;
      } else {
        col_number++;
      }
    } else {
      error_recovery.emplace_back(
          std::make_pair(ERROR_LOCATION + " " + RED + "error:" + RESET +
                             " invalid token> " + std::string(1, ch),
                         "Please check the code"));
      success = false;
      tokens.emplace_back(token::TOKEN::UNKNOWN);
    }
  }
}

void lexer::print_symbol_table() {
  const int w = 64;
  std::cout << BOLD << CYAN << std::left << std::string(2 * w, '-')
            << std::endl;
  std::cout << std::left << std::setw(w) << "Lexeme" << std::setw(w) << "Tokens"
            << std::endl;
  std::cout << std::left << std::string(2 * w, '-') << RESET << std::endl;
  for (auto &token_ : lexer::tokens) {
    if (token_.get_value().has_value()) {
      std::cout << RED << std::left << std::setw(w)
                << token_.get_value().value() << GREEN << std::setw(w)
                << token::get_token_type(token_.get_token()) << RESET
                << std::endl;
    } else {
      std::cout << RED << std::left << std::setw(w)
                << token::to_string(token_.get_token()) << GREEN << std::setw(w)
                << token::get_token_type(token_.get_token()) << RESET
                << std::endl;
    }
  }
  std::cout << BOLD << CYAN << std::left << std::string(2 * w, '-') << RESET
            << std::endl;
}

void lexer::print_error_recovery() {
  for (auto &error : error_recovery) {
    std::cout << error.first << std::endl;
    std::cout << GREEN << "info:" << RESET << " " << error.second << std::endl;
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
