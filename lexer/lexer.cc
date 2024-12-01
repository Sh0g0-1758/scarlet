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
    } else if(regex.matchDigit(file_contents[0])) {
        std::string constant;
        while(regex.matchDigit(file_contents[0])) {
            constant += file_contents[0];
            file_contents.erase(0, 1);
        }
        tokens.emplace_back(TOKEN::CONSTANT);
        tokenize();
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
    for(auto token : tokens) {
        switch(token) {
            case TOKEN::IDENTIFIER:
                std::cout << "IDENTIFIER" << std::endl;
                break;
            case TOKEN::CONSTANT:
                std::cout << "CONSTANT" << std::endl;
                break;
            case TOKEN::INT:
                std::cout << "INT" << std::endl;
                break;
            case TOKEN::VOID:
                std::cout << "VOID" << std::endl;
                break;
            case TOKEN::RETURN:
                std::cout << "RETURN" << std::endl;
                break;
            case TOKEN::OPEN_PARANTHESES:
                std::cout << "OPEN_PARANTHESES" << std::endl;
                break;
            case TOKEN::CLOSE_PARANTHESES:
                std::cout << "CLOSE_PARANTHESES" << std::endl;
                break;
            case TOKEN::OPEN_BRACE:
                std::cout << "OPEN_BRACE" << std::endl;
                break;
            case TOKEN::CLOSE_BRACE:
                std::cout << "CLOSE_BRACE" << std::endl;
                break;
            case TOKEN::SEMICOLON:
                std::cout << "SEMICOLON" << std::endl;
                break;
        }
    }
}

void lexer::read_file(const std::string& file_path) {
    std::ifstream file(file_path);
    if(file.is_open()) {
        std::string line;
        while(std::getline(file, line)) {
            file_contents += line;
        }
        file.close();
    }
}
