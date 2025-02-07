#include "token.hh"

namespace scarlet {
namespace token {
void print_token(TOKEN token) {
  switch (token) {
  case TOKEN::IDENTIFIER:
    std::cerr << "identifier ";
    break;
  case TOKEN::CONSTANT:
    std::cerr << "constant ";
    break;
  case TOKEN::INT:
    std::cerr << "int ";
    break;
  case TOKEN::VOID:
    std::cerr << "void ";
    break;
  case TOKEN::RETURN:
    std::cerr << "return ";
    break;
  case TOKEN::IF:
    std::cerr << "if ";
    break;
  case TOKEN::ELSE:
    std::cerr << "else ";
    break;
  case TOKEN::DO:
    std::cerr << "do ";
    break;
  case TOKEN::WHILE:
    std::cerr << "while ";
    break;
  case TOKEN::FOR:
    std::cerr << "for ";
    break;
  case TOKEN::BREAK:
    std::cerr << "break ";
    break;
  case TOKEN::CONTINUE:
    std::cerr << "continue ";
    break;
  case TOKEN::STATIC:
    std::cerr << "static ";
    break;
  case TOKEN::EXTERN:
    std::cerr << "extern ";
    break;
  case TOKEN::LONG:
    std::cerr << "long ";
    break;
  case TOKEN::SIGNED:
    std::cerr << "signed ";
    break;
  case TOKEN::UNSIGNED:
    std::cerr << "unsigned ";
    break;
  case TOKEN::OPEN_PARANTHESES:
    std::cerr << "( ";
    break;
  case TOKEN::CLOSE_PARANTHESES:
    std::cerr << ") ";
    break;
  case TOKEN::OPEN_BRACE:
    std::cerr << "{ ";
    break;
  case TOKEN::CLOSE_BRACE:
    std::cerr << "} ";
    break;
  case TOKEN::SEMICOLON:
    std::cerr << "; ";
    break;
  case TOKEN::COLON:
    std::cerr << ": ";
    break;
  case TOKEN::QUESTION_MARK:
    std::cerr << "? ";
    break;
  case TOKEN::TILDE:
    std::cerr << "~";
    break;
  case TOKEN::HYPHEN:
    std::cerr << "-";
    break;
  case TOKEN::PLUS:
    std::cerr << "+";
    break;
  case TOKEN::ASTERISK:
    std::cerr << "*";
    break;
  case TOKEN::FORWARD_SLASH:
    std::cerr << "/";
    break;
  case TOKEN::PERCENT_SIGN:
    std::cerr << "%";
    break;
  case TOKEN::ASSIGNMENT:
    std::cerr << "=";
    break;
  case TOKEN::DECREMENT_OPERATOR:
    std::cerr << "--";
    break;
  case TOKEN::AAND:
    std::cerr << "&";
    break;
  case TOKEN::AOR:
    std::cerr << "|";
    break;
  case TOKEN::XOR:
    std::cerr << "^";
    break;
  case TOKEN::LEFT_SHIFT:
    std::cerr << "<<";
    break;
  case TOKEN::RIGHT_SHIFT:
    std::cerr << ">>";
    break;
  case TOKEN::NOT:
    std::cerr << "!";
    break;
  case TOKEN::LAND:
    std::cerr << "&&";
    break;
  case TOKEN::LOR:
    std::cerr << "||";
    break;
  case TOKEN::EQUAL:
    std::cerr << "==";
    break;
  case TOKEN::NOTEQUAL:
    std::cerr << "!=";
    break;
  case TOKEN::LESSTHAN:
    std::cerr << "<";
    break;
  case TOKEN::GREATERTHAN:
    std::cerr << ">";
    break;
  case TOKEN::LESSTHANEQUAL:
    std::cerr << "<=";
    break;
  case TOKEN::GREATERTHANEQUAL:
    std::cerr << ">=";
    break;
  case TOKEN::COMMA:
    std::cerr << ",";
    break;
  case TOKEN::UNKNOWN:
    std::cerr << "UNKNOWN ";
    break;
  }
}

std::string to_string(TOKEN token) {
  switch (token) {
  case TOKEN::IDENTIFIER:
    return "identifier";
  case TOKEN::CONSTANT:
    return "constant";
  case TOKEN::INT:
    return "int";
  case TOKEN::VOID:
    return "void";
  case TOKEN::RETURN:
    return "return";
  case TOKEN::IF:
    return "if";
  case TOKEN::ELSE:
    return "else";
  case TOKEN::DO:
    return "do";
  case TOKEN::WHILE:
    return "while";
  case TOKEN::FOR:
    return "for";
  case TOKEN::BREAK:
    return "break";
  case TOKEN::CONTINUE:
    return "continue";
  case TOKEN::STATIC:
    return "static";
  case TOKEN::EXTERN:
    return "extern";
  case TOKEN::LONG:
    return "long";
  case TOKEN::SIGNED:
    return "signed";
  case TOKEN::UNSIGNED:
    return "unsigned";
  case TOKEN::OPEN_PARANTHESES:
    return "(";
  case TOKEN::CLOSE_PARANTHESES:
    return ")";
  case TOKEN::OPEN_BRACE:
    return "{";
  case TOKEN::CLOSE_BRACE:
    return "}";
  case TOKEN::SEMICOLON:
    return ";";
  case TOKEN::COLON:
    return ":";
  case TOKEN::QUESTION_MARK:
    return "?";
  case TOKEN::TILDE:
    return "~";
  case TOKEN::HYPHEN:
    return "-";
  case TOKEN::PLUS:
    return "+";
  case TOKEN::ASTERISK:
    return "*";
  case TOKEN::FORWARD_SLASH:
    return "/";
  case TOKEN::PERCENT_SIGN:
    return "%";
  case TOKEN::ASSIGNMENT:
    return "=";
  case TOKEN::DECREMENT_OPERATOR:
    return "--";
  case TOKEN::AAND:
    return "&";
  case TOKEN::AOR:
    return "|";
  case TOKEN::NOT:
    return "!";
  case TOKEN::LAND:
    return "&&";
  case TOKEN::LOR:
    return "||";
  case TOKEN::EQUAL:
    return "==";
  case TOKEN::NOTEQUAL:
    return "!=";
  case TOKEN::LESSTHAN:
    return "<";
  case TOKEN::GREATERTHAN:
    return ">";
  case TOKEN::LESSTHANEQUAL:
    return "<=";
  case TOKEN::GREATERTHANEQUAL:
    return ">=";
  case TOKEN::XOR:
    return "^";
  case TOKEN::LEFT_SHIFT:
    return "<<";
  case TOKEN::RIGHT_SHIFT:
    return ">>";
  case TOKEN::COMMA:
    return ",";
  case TOKEN::UNKNOWN:
    return "UNKNOWN";
  }
  return "INVALID";
}

bool is_unary_op(TOKEN token) {
  return token == TOKEN::TILDE or token == TOKEN::HYPHEN or token == TOKEN::NOT;
}

bool is_binary_op(TOKEN token) {
  return token == TOKEN::PLUS or token == TOKEN::HYPHEN or
         token == TOKEN::ASTERISK or token == TOKEN::FORWARD_SLASH or
         token == TOKEN::PERCENT_SIGN or token == TOKEN::AAND or
         token == TOKEN::AOR or token == TOKEN::XOR or
         token == TOKEN::LEFT_SHIFT or token == TOKEN::RIGHT_SHIFT or
         token == TOKEN::LAND or token == TOKEN::LOR or token == TOKEN::EQUAL or
         token == TOKEN::NOTEQUAL or token == TOKEN::LESSTHAN or
         token == TOKEN::GREATERTHAN or token == TOKEN::LESSTHANEQUAL or
         token == TOKEN::GREATERTHANEQUAL or token == TOKEN::ASSIGNMENT or
         token == TOKEN::COLON or token == TOKEN::QUESTION_MARK;
}

std::string get_token_type(TOKEN token) {
  if (token == TOKEN::IDENTIFIER) {
    return "IDENTIFIER";
  } else if (token == TOKEN::CONSTANT) {
    return "CONSTANT";
  } else if (token >= TOKEN::INT && token <= TOKEN::UNSIGNED) {
    return "KEYWORD";
  } else if (token >= TOKEN::OPEN_PARANTHESES && token <= TOKEN::SEMICOLON) {
    return "SPECIAL SYMBOL";
  } else if (token >= TOKEN::TILDE) {
    return "OPERATORS";
  }
  return "UNKNOWN";
}

int get_binop_prec(TOKEN token) {
  if (token == TOKEN::COMMA) {
    return 0;
  } else if (token == TOKEN::ASSIGNMENT) {
    return 5;
  } else if (token == TOKEN::COLON or token == TOKEN::QUESTION_MARK) {
    return 10;
  } else if (token == TOKEN::LOR) {
    return 15;
  } else if (token == TOKEN::LAND) {
    return 20;
  } else if (token == TOKEN::NOTEQUAL or token == TOKEN::EQUAL) {
    return 25;
  } else if (token == TOKEN::LESSTHAN or token == TOKEN::GREATERTHAN or
             token == TOKEN::LESSTHANEQUAL or
             token == TOKEN::GREATERTHANEQUAL) {
    return 30;
  } else if (token == TOKEN::AOR) {
    return 35;
  } else if (token == TOKEN::XOR) {
    return 40;
  } else if (token == TOKEN::AAND) {
    return 45;
  } else if (token == TOKEN::LEFT_SHIFT or token == TOKEN::RIGHT_SHIFT) {
    return 50;
  } else if (token == TOKEN::PLUS or token == TOKEN::HYPHEN) {
    return 55;
  } else if (token == TOKEN::ASTERISK or token == TOKEN::FORWARD_SLASH or
             token == TOKEN::PERCENT_SIGN) {
    return 60;
  }
  __builtin_unreachable();
}
} // namespace token
} // namespace scarlet
