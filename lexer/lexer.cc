#include "lexer.hh"

void lexer::tokenize() {
    if(file_contents.empty()) return;
    if(file_contents[0] == '(') {
        tokens.emplace_back(TOKEN::OPEN_PARANTHESES);
        file_contents.erase(0, 1);
        tokenize();
    } else if(file_contents[0] == ')') {
        tokens.emplace_back(TOKEN::CLOSE_PARANTHESES);
        file_contents.erase(0, 1);
        tokenize();
    } else if(file_contents[0] == '{') {
        tokens.emplace_back(TOKEN::OPEN_BRACE);
        file_contents.erase(0, 1);
        tokenize();
    } else if(file_contents[0] == '}') {
        tokens.emplace_back(TOKEN::CLOSE_BRACE);
        file_contents.erase(0, 1);
        tokenize();
    } else if(file_contents[0] == ';') {
        tokens.emplace_back(TOKEN::SEMICOLON);
        file_contents.erase(0, 1);
        tokenize();
    } else if(regex.matchWord(file_contents[0])) {
        std::string identifier;
        while(regex.matchWord(file_contents[0])) {
            identifier += file_contents[0];
            file_contents.erase(0, 1);
        }
        if(regex.matchDigit(file_contents[0])) {
            success = false;
            tokens.emplace_back(TOKEN::UNKNOWN);
            tokenize();
        } else {
            if(identifier == "int") {
                tokens.emplace_back(TOKEN::INT);
            } else if(identifier == "void") {
                tokens.emplace_back(TOKEN::VOID);
            } else if(identifier == "return") {
                tokens.emplace_back(TOKEN::RETURN);
            } else {
                tokens.emplace_back(TOKEN::IDENTIFIER);
            }
            tokenize();
        }
    } else if(regex.matchDigit(file_contents[0])) {
        std::string constant;
        while(regex.matchDigit(file_contents[0])) {
            constant += file_contents[0];
            file_contents.erase(0, 1);
        }
        if (regex.matchWord(file_contents[0])) {
            success = false;
            tokens.emplace_back(TOKEN::UNKNOWN);
            tokenize();
        } else {
            tokens.emplace_back(Token(TOKEN::CONSTANT, constant));
            tokenize();
        }
    } else if(file_contents[0] == '\n' or file_contents[0] == ' ' or file_contents[0] == '\t') {
        file_contents.erase(0, 1);
        tokenize();
    } else if(file_contents[0] == '/') {
        if(file_contents[1] == '/') {
            while(file_contents[0] != '\n') {
                file_contents.erase(0, 1);
            }
            file_contents.erase(0, 1);
            tokenize();
        } else {
            success = false;
            tokens.emplace_back(TOKEN::UNKNOWN);
            file_contents.erase(0, 1);
            tokenize();
        }
    } else {
        success = false;
        tokens.emplace_back(TOKEN::UNKNOWN);
        file_contents.erase(0, 1);
        tokenize();
    }
}

void lexer::print_tokens() {
    for(auto token : tokens) print_token(token.get_token());
}

void lexer::read_file(const std::string& file_path) {
    std::ifstream file(file_path);
    if(file.is_open()) {
        std::string line;
        while(std::getline(file, line)) {
            file_contents += line;
            file_contents += '\n';
        }
        file.close();
    }
}

std::vector<Token> lexer::get_tokens() {
    return tokens;
}
