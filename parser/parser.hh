#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <token/token.hh>

/*

Grammar: 

<program> ::= <function>
<function> ::= "int" <identifier> "(" "void" ")" "{" <statement> "}"
<statement> ::= "return" <exp> ";"
<exp> ::= <int>
<identifier> ::= ? An identifier token ?
<int> ::= ? A constant token ?

*/

class parser {
private:
    bool success = true;
    std::vector<std::string> error_messages;
    void parse_function(std::vector<Token>& tokens);
    void parse_statement(std::vector<Token>& tokens);
    void parse_exp(std::vector<Token>& tokens);
    void parse_identifier(std::vector<Token>& tokens);
    void parse_int(std::vector<Token>& tokens);
    void expect(TOKEN actual_token, TOKEN expected_token);
    void eof_error(Token token);
public:
    void parse_program(std::vector<Token> tokens);
    bool is_success() { return success; }
    void display_errors();
};