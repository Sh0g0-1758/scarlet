#pragma once

#include <optional>

enum class TOKEN {
    IDENTIFIER,
    CONSTANT,
    INT,
    VOID,
    RETURN,
    OPEN_PARANTHESES,
    CLOSE_PARANTHESES,
    OPEN_BRACE,
    CLOSE_BRACE,
    SEMICOLON,
    UNKNOWN
};

class Token {
private:
    TOKEN token;
    std::optional<std::string> value;

public:
    Token(TOKEN token, std::optional<std::string> value) : token(token), value(value) {}
    Token(TOKEN token) : token(token) {}

    TOKEN get_token() {
        return token;
    }

    std::optional<std::string> get_value() {
        return value;
    }
};

void print_token(TOKEN token) {
    switch(token) {
        case TOKEN::IDENTIFIER:
            std::cerr << "IDENTIFIER" << std::endl;
            break;
        case TOKEN::CONSTANT:
            std::cerr << "CONSTANT" << std::endl;
            break;
        case TOKEN::INT:
            std::cerr << "INT" << std::endl;
            break;
        case TOKEN::VOID:
            std::cerr << "VOID" << std::endl;
            break;
        case TOKEN::RETURN:
            std::cerr << "RETURN" << std::endl;
            break;
        case TOKEN::OPEN_PARANTHESES:
            std::cerr << "OPEN_PARANTHESES" << std::endl;
            break;
        case TOKEN::CLOSE_PARANTHESES:
            std::cerr << "CLOSE_PARANTHESES" << std::endl;
            break;
        case TOKEN::OPEN_BRACE:
            std::cerr << "OPEN_BRACE" << std::endl;
            break;
        case TOKEN::CLOSE_BRACE:
            std::cerr << "CLOSE_BRACE" << std::endl;
            break;
        case TOKEN::SEMICOLON:
            std::cerr << "SEMICOLON" << std::endl;
            break;
        case TOKEN::UNKNOWN:
            std::cerr << "UNKNOWN" << std::endl;
            break;
    }
}

std::string to_string(TOKEN token) {
    switch(token) {
        case TOKEN::IDENTIFIER:
            return "IDENTIFIER";
        case TOKEN::CONSTANT:
            return "CONSTANT";
        case TOKEN::INT:
            return "INT";
        case TOKEN::VOID:
            return "VOID";
        case TOKEN::RETURN:
            return "RETURN";
        case TOKEN::OPEN_PARANTHESES:
            return "OPEN_PARANTHESES";
        case TOKEN::CLOSE_PARANTHESES:
            return "CLOSE_PARANTHESES";
        case TOKEN::OPEN_BRACE:
            return "OPEN_BRACE";
        case TOKEN::CLOSE_BRACE:
            return "CLOSE_BRACE";
        case TOKEN::SEMICOLON:
            return "SEMICOLON";
        case TOKEN::UNKNOWN:
            return "UNKNOWN";
    }
    return "INVALID";
}
