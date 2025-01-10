#include "lexer.hh"

void lexer::tokenize() {
  regex_engine re;
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
  } else if (re.matchWord(file_contents[0])) {
    std::string identifier;
    while (re.matchWord(file_contents[0])) {
      identifier += file_contents[0];
      file_contents.erase(0, 1);
    }
    if (re.matchDigit(file_contents[0])) {
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
  } else if (re.matchDigit(file_contents[0])) {
    std::string constant;
    while (re.matchDigit(file_contents[0])) {
      constant += file_contents[0];
      file_contents.erase(0, 1);
    }
    if (re.matchWord(file_contents[0])) {
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
  }
  cout<<"now we print tokens"<<endl;
  print_tokens();
}

void lexer::print_tokens() {
  for (auto token : tokens)
    print_token(token.get_token());
  std::cout << std::endl;
}

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
