#include "parser.hh"

#define EXPECT(tok)                                                            \
    if (tokens.empty()) {                                                      \
        eof_error(tok);                                                        \
        return;                                                                \
    }                                                                          \
    expect(tokens[0].get_token(), tok);                                        \
    tokens.erase(tokens.begin());                                              \

void parser::parse_program(std::vector<Token> tokens) {
    parse_function(tokens);
}

void parser::parse_function(std::vector<Token>& tokens) {
    EXPECT(TOKEN::INT);
    parse_identifier(tokens);
    EXPECT(TOKEN::OPEN_PARANTHESES);
    EXPECT(TOKEN::VOID);
    EXPECT(TOKEN::CLOSE_PARANTHESES);
    EXPECT(TOKEN::OPEN_BRACE);
    parse_statement(tokens);
    EXPECT(TOKEN::CLOSE_BRACE);
    if(!tokens.empty()) {
        success = false;
        error_messages.emplace_back("Expected end of file but got " + to_string(tokens[0].get_token()));
    }
}

void parser::parse_statement(std::vector<Token>& tokens) {
    EXPECT(TOKEN::RETURN);
    parse_exp(tokens);
    EXPECT(TOKEN::SEMICOLON);
}

void parser::parse_exp(std::vector<Token>& tokens) {
    parse_int(tokens);
}

void parser::parse_identifier(std::vector<Token>& tokens) {
    EXPECT(TOKEN::IDENTIFIER);
}

void parser::parse_int(std::vector<Token>& tokens) {
    EXPECT(TOKEN::CONSTANT);
}

void parser::expect(TOKEN actual_token, TOKEN expected_token) {
    if(actual_token != expected_token) {
        success = false;
        error_messages.emplace_back("Expected token " + to_string(expected_token) + " but got " + to_string(actual_token));
    }
}

void parser::display_errors() {
    for(auto error : error_messages) {
        std::cerr << error << std::endl;
    }
}

void parser::eof_error(Token token) {
    success = false;
    error_messages.emplace_back("Expected " + to_string(token.get_token()) + " but got end of file");
}
