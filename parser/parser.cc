#include "parser.hh"

#define EXPECT(tok)                                                            \
    if (tokens.empty()) {                                                      \
        eof_error(tok);                                                        \
        return;                                                                \
    }                                                                          \
    expect(tokens[0].get_token(), tok);                                        \
    tokens.erase(tokens.begin());                                              \

#define EXPECT_FUNC(tok)                                                       \
    if (tokens.empty()) {                                                      \
        eof_error(tok);                                                        \
        return function;                                                       \
    }                                                                          \
    expect(tokens[0].get_token(), tok);                                        \
    tokens.erase(tokens.begin());                                              \

void parser::parse_program(std::vector<Token> tokens) {
    AST_Program_Node program;
    while(!tokens.empty()) {
        AST_Function_Node func = parse_function(tokens);
        program.add_function(func);
    }
    this->program = program;
}

AST_Function_Node parser::parse_function(std::vector<Token>& tokens) {
    AST_Function_Node function;
    EXPECT_FUNC(TOKEN::INT);
    parse_identifier(tokens, function);
    EXPECT_FUNC(TOKEN::OPEN_PARANTHESES);
    EXPECT_FUNC(TOKEN::VOID);
    EXPECT_FUNC(TOKEN::CLOSE_PARANTHESES);
    EXPECT_FUNC(TOKEN::OPEN_BRACE);
    parse_statement(tokens, function);
    EXPECT_FUNC(TOKEN::CLOSE_BRACE);
    if(!tokens.empty()) {
        success = false;
        error_messages.emplace_back("Expected end of file but got " + to_string(tokens[0].get_token()));
    }
    return function;
}

void parser::parse_statement(std::vector<Token>& tokens, AST_Function_Node& function) {
    AST_Statement_Node statement("Return");
    EXPECT(TOKEN::RETURN);
    parse_exp(tokens, statement);
    EXPECT(TOKEN::SEMICOLON);
    function.add_statement(statement);
}

void parser::parse_exp(std::vector<Token>& tokens, AST_Statement_Node& statement) {
    AST_exp_Node exp;
    parse_int(tokens, exp);
    statement.add_exp(exp);
}

void parser::parse_identifier(std::vector<Token>& tokens, AST_Function_Node& function) {
    AST_identifier_Node identifier;
    identifier.set_identifier(tokens[0].get_value().value());
    function.set_identifier(identifier);
    EXPECT(TOKEN::IDENTIFIER);
}

void parser::parse_int(std::vector<Token>& tokens, AST_exp_Node& exp) {
    AST_int_Node int_node;
    int_node.set_value(tokens[0].get_value().value());
    exp.set_int_node(int_node);
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

void parser::pretty_print() {
    std::cerr << "Program(" << std::endl;
    for(auto function : program.get_functions()) {
        std::cerr << "\tFunction(" << std::endl;
        std::cerr << "\t\tname=\"" << function.get_identifier().get_value() << "\"," << std::endl;
        std::cerr << "\t\tbody=[" << std::endl;
        for(auto statement : function.get_statements()) {
            std::cerr << "\t\t\t" << statement.get_type() << "(" << std::endl;
            for(auto exp : statement.get_exps()) {
                std::cerr << "\t\t\t\tExpression(" << exp.get_int_node().get_AST_name() << "(" << exp.get_int_node().get_value() << "))" << std::endl;
            }
            std::cerr << "\t\t\t)," << std::endl;
        }
        std::cerr << "\t\t]" << std::endl;
        std::cerr << "\t)," << std::endl;
    }
    std::cerr << ")" << std::endl;
}
