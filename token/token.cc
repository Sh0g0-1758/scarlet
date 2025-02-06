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
         token == TOKEN::GREATERTHANEQUAL;
}

int get_binop_prec(TOKEN token) {
  if (token == TOKEN::LOR) {
    return 5;
  } else if (token == TOKEN::LAND) {
    return 10;
  } else if (token == TOKEN::NOTEQUAL or token == TOKEN::EQUAL) {
    return 15;
  } else if (token == TOKEN::LESSTHAN or token == TOKEN::GREATERTHAN or
             token == TOKEN::LESSTHANEQUAL or
             token == TOKEN::GREATERTHANEQUAL) {
    return 20;
  } else if (token == TOKEN::AOR) {
    return 25;
  } else if (token == TOKEN::XOR) {
    return 30;
  } else if (token == TOKEN::AAND) {
    return 35;
  } else if (token == TOKEN::LEFT_SHIFT or token == TOKEN::RIGHT_SHIFT) {
    return 40;
  } else if (token == TOKEN::PLUS or token == TOKEN::HYPHEN) {
    return 45;
  } else if (token == TOKEN::ASTERISK or token == TOKEN::FORWARD_SLASH or
             token == TOKEN::PERCENT_SIGN) {
    return 50;
  }
  __builtin_unreachable();
}
} // namespace token
} // namespace scarlet
