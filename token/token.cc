#include "token.hh"

namespace scarlet {
namespace token {

void print_token(TOKEN token) {
  switch (token) {
  case TOKEN::IDENTIFIER:
    std::cerr << "identifier ";
    break;
  case TOKEN::INT_CONSTANT:
    std::cerr << "int constant ";
    break;
  case TOKEN::UINT_CONSTANT:
    std::cerr << "uint constant ";
    break;
  case TOKEN::LONG_CONSTANT:
    std::cerr << "long constant ";
    break;
  case TOKEN::ULONG_CONSTANT:
    std::cerr << "ulong constant ";
    break;
  case TOKEN::DOUBLE_CONSTANT:
    std::cerr << "double constant ";
    break;
  case TOKEN::CHARACTER:
    std::cerr << "character ";
    break;
  case TOKEN::STRING:
    std::cerr << "string ";
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
  case TOKEN::UNTIL:
    std::cerr << "until ";
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
  case TOKEN::SWITCH:
    std::cerr << "switch ";
    break;
  case TOKEN::CASE:
    std::cerr << "case ";
    break;
  case TOKEN::DEFAULT_CASE:
    std::cerr << "default_case ";
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
  case TOKEN::DOUBLE:
    std::cerr << "double ";
    break;
  case TOKEN::CHAR:
    std::cerr << "char ";
    break;
  case TOKEN::SIZEOF:
    std::cerr << "sizeof ";
    break;
  case TOKEN::STRUCT:
    std::cerr << "struct ";
    break;
  case TOKEN::GOTO:
    std::cerr << "goto ";
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
  case TOKEN::OPEN_BRACKET:
    std::cerr << "[ ";
    break;
  case TOKEN::CLOSE_BRACKET:
    std::cerr << "] ";
    break;
  case TOKEN::COLON:
    std::cerr << ": ";
    break;
  case TOKEN::ELLIPSIS:
    std::cerr << "... ";
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
  case TOKEN::INCREMENT_OPERATOR:
    std::cerr << "++";
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
  case TOKEN::ARROW_OPERATOR:
    std::cerr << "->";
    break;
  case TOKEN::DOT:
    std::cerr << ".";
    break;
  case TOKEN::COMPOUND_SUM:
    std::cerr << "+=";
    break;
  case TOKEN::COMPOUND_PRODUCT:
    std::cerr << "*=";
    break;
  case TOKEN::COMPOUND_DIFFERENCE:
    std::cerr << "-=";
    break;
  case TOKEN::COMPOUND_DIVISION:
    std::cerr << "/=";
    break;
  case TOKEN::COMPOUND_REMAINDER:
    std::cerr << "%=";
    break;
  case TOKEN::COMPOUND_LEFTSHIFT:
    std::cerr << "<<=";
    break;
  case TOKEN::COMPOUND_RIGHTSHIFT:
    std::cerr << ">>=";
    break;
  case TOKEN::COMPOUND_AND:
    std::cerr << "&=";
    break;
  case TOKEN::COMPOUND_OR:
    std::cerr << "|=";
    break;
  case TOKEN::COMPOUND_XOR:
    std::cerr << "^=";
    break;
  case TOKEN::UNKNOWN:
    std::cerr << "UNKNOWN ";
    break;
  case TOKEN::BINARYSTART:
  case TOKEN::BINARYEND:
  case TOKEN::BITWISESTART:
  case TOKEN::BITWISEEND:
  case TOKEN::LOGICALSTART:
  case TOKEN::LOGICALEND:
  case TOKEN::UNARYSTART:
  case TOKEN::UNARYEND:
  case TOKEN::KEYWORDSTART:
  case TOKEN::KEYWORDEND:
  case TOKEN::GRAMMARSTART:
  case TOKEN::GRAMMAREND:
  case TOKEN::CONSTANTSTART:
  case TOKEN::CONSTANTEND:
    UNREACHABLE()
  }
}

std::string to_string(TOKEN token) {
  switch (token) {
  case TOKEN::IDENTIFIER:
    return "identifier";
  case TOKEN::INT_CONSTANT:
    return "int constant";
  case TOKEN::UINT_CONSTANT:
    return "uint constant";
  case TOKEN::LONG_CONSTANT:
    return "long constant";
  case TOKEN::ULONG_CONSTANT:
    return "ulong constant";
  case TOKEN::DOUBLE_CONSTANT:
    return "double constant";
  case TOKEN::CHARACTER:
    return "character";
  case TOKEN::STRING:
    return "string";
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
  case TOKEN::UNTIL:
    return "until";
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
  case TOKEN::SWITCH:
    return "switch";
  case TOKEN::CASE:
    return "case";
  case TOKEN::DEFAULT_CASE:
    return "default_case";
  case TOKEN::LONG:
    return "long";
  case TOKEN::SIGNED:
    return "signed";
  case TOKEN::UNSIGNED:
    return "unsigned";
  case TOKEN::DOUBLE:
    return "double";
  case TOKEN::CHAR:
    return "char";
  case TOKEN::SIZEOF:
    return "sizeof";
  case TOKEN::STRUCT:
    return "struct";
  case TOKEN::GOTO:
    return "goto";
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
  case TOKEN::ELLIPSIS:
    return "...";
  case TOKEN::OPEN_BRACKET:
    return "[";
  case TOKEN::CLOSE_BRACKET:
    return "]";
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
  case TOKEN::INCREMENT_OPERATOR:
    return "++";
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
  case TOKEN::ARROW_OPERATOR:
    return "->";
  case TOKEN::DOT:
    return ".";
  case TOKEN::COMPOUND_SUM:
    return "+=";
  case TOKEN::COMPOUND_DIFFERENCE:
    return "-=";
  case TOKEN::COMPOUND_PRODUCT:
    return "*=";
  case TOKEN::COMPOUND_DIVISION:
    return "/=";
  case TOKEN::COMPOUND_REMAINDER:
    return "%=";
  case TOKEN::COMPOUND_AND:
    return "&=";
  case TOKEN::COMPOUND_OR:
    return "|=";
  case TOKEN::COMPOUND_XOR:
    return "^=";
  case TOKEN::COMPOUND_LEFTSHIFT:
    return "<<=";
  case TOKEN::COMPOUND_RIGHTSHIFT:
    return ">>=";
  case TOKEN::UNKNOWN:
    return "UNKNOWN";
  case TOKEN::BINARYSTART:
  case TOKEN::BINARYEND:
  case TOKEN::BITWISESTART:
  case TOKEN::BITWISEEND:
  case TOKEN::LOGICALSTART:
  case TOKEN::LOGICALEND:
  case TOKEN::UNARYSTART:
  case TOKEN::UNARYEND:
  case TOKEN::KEYWORDSTART:
  case TOKEN::KEYWORDEND:
  case TOKEN::GRAMMARSTART:
  case TOKEN::GRAMMAREND:
  case TOKEN::CONSTANTSTART:
  case TOKEN::CONSTANTEND:
    UNREACHABLE()
  }
  return "INVALID";
}

bool is_unary_op(TOKEN token) {
  return token == TOKEN::TILDE or token == TOKEN::HYPHEN or
         token == TOKEN::NOT or token == TOKEN::DECREMENT_OPERATOR or
         token == TOKEN::INCREMENT_OPERATOR or token == TOKEN::AAND or
         token == TOKEN::ASTERISK;
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
         token == TOKEN::QUESTION_MARK or token == TOKEN::COMPOUND_DIFFERENCE or
         token == TOKEN::COMPOUND_DIVISION or token == TOKEN::COMPOUND_SUM or
         token == TOKEN::COMPOUND_PRODUCT or
         token == TOKEN::COMPOUND_REMAINDER or token == TOKEN::COMPOUND_AND or
         token == TOKEN::COMPOUND_OR or token == TOKEN::COMPOUND_LEFTSHIFT or
         token == TOKEN::COMPOUND_RIGHTSHIFT or token == TOKEN::COMPOUND_XOR;
}

char char_to_esc(char c) {
  switch (c) {
  case 'a':
    return '\a';
  case 'n':
    return '\n';
  case 'f':
    return '\f';
  case 'b':
    return '\b';
  case 'r':
    return '\r';
  case 't':
    return '\t';
  case 'v':
    return '\v';
  case '\'':
    return '\'';
  case '"':
    return '\"';
  case '?':
    return '\?';
  case '\\':
    return '\\';
  case '0':
    return '\0';
  default:
    return '\0';
  }
}

std::string get_token_type(TOKEN token) {
  if (token == TOKEN::IDENTIFIER) {
    return "IDENTIFIER";
  } else if (token > TOKEN::CONSTANTSTART && token < TOKEN::CONSTANTEND) {
    return "CONSTANT";
  } else if (token > TOKEN::KEYWORDSTART && token < TOKEN::KEYWORDEND) {
    return "KEYWORD";
  } else if (token > TOKEN::GRAMMARSTART && token < TOKEN::ELLIPSIS) {
    return "SPECIAL SYMBOL";
  } else {
    if (token > TOKEN::UNARYSTART && token < TOKEN::UNARYEND) {
      return "OPERATOR (UNARY)";
    } else if (token > TOKEN::BINARYSTART && token < TOKEN::BINARYEND) {
      return "OPERATOR (BINARY)";
    } else if (token > TOKEN::BITWISESTART && token < TOKEN::BITWISEEND) {
      return "OPERATOR (BITWISE)";
    } else if (token > TOKEN::LOGICALSTART && token < TOKEN::LOGICALEND) {
      return "OPERATOR (LOGICAL)";
    }
  }
  return "UNKNOWN";
}

bool is_right_associative(TOKEN token) {
  return token == token::TOKEN::ASSIGNMENT or
         token == token::TOKEN::COMPOUND_DIFFERENCE or
         token == token::TOKEN::COMPOUND_DIVISION or
         token == token::TOKEN::COMPOUND_PRODUCT or
         token == token::TOKEN::COMPOUND_REMAINDER or
         token == token::TOKEN::COMPOUND_SUM or
         token == token::TOKEN::COMPOUND_AND or
         token == token::TOKEN::COMPOUND_OR or
         token == token::TOKEN::COMPOUND_XOR or
         token == token::TOKEN::COMPOUND_LEFTSHIFT or
         token == token::TOKEN::COMPOUND_RIGHTSHIFT;
}

bool is_type_specifier(TOKEN token) {
  return token == TOKEN::INT or token == TOKEN::LONG or
         token == TOKEN::SIGNED or token == TOKEN::UNSIGNED or
         token == TOKEN::DOUBLE or token == TOKEN::CHAR or token == TOKEN::VOID;
}

bool is_storage_specifier(TOKEN token) {
  return token == TOKEN::STATIC or token == TOKEN::EXTERN;
}

bool is_constant(TOKEN token) {
  return token == TOKEN::INT_CONSTANT or token == TOKEN::LONG_CONSTANT or
         token == TOKEN::UINT_CONSTANT or token == TOKEN::ULONG_CONSTANT or
         token == TOKEN::DOUBLE_CONSTANT or token == TOKEN::CHARACTER or
         token == TOKEN::STRING;
}

bool is_integer_constant(TOKEN token) {
  return is_constant(token) and !(token == TOKEN::DOUBLE_CONSTANT) and
         !(token == TOKEN::STRING);
}

int get_binop_prec(TOKEN token) {
  if (token == TOKEN::COMMA) {
    return 0;
  } else if (is_right_associative(token)) {
    return 5;
  } else if (token == TOKEN::QUESTION_MARK) {
    // Ternary operator
    // only precendence of ? is required
    // precedence of : is not required
    return 10;
  } else if (token == TOKEN::LOR) {
    return 15;
  } else if (token == TOKEN::LAND) {
    return 20;
  } else if (token == TOKEN::AOR) {
    return 25;
  } else if (token == TOKEN::XOR) {
    return 30;
  } else if (token == TOKEN::AAND) {
    return 35;
  } else if (token == TOKEN::NOTEQUAL or token == TOKEN::EQUAL) {
    return 40;
  } else if (token == TOKEN::LESSTHAN or token == TOKEN::GREATERTHAN or
             token == TOKEN::LESSTHANEQUAL or
             token == TOKEN::GREATERTHANEQUAL) {
    return 45;
  } else if (token == TOKEN::LEFT_SHIFT or token == TOKEN::RIGHT_SHIFT) {
    return 50;
  } else if (token == TOKEN::PLUS or token == TOKEN::HYPHEN) {
    return 55;
  } else if (token == TOKEN::ASTERISK or token == TOKEN::FORWARD_SLASH or
             token == TOKEN::PERCENT_SIGN) {
    return 60;
  }
  UNREACHABLE()
}
} // namespace token
} // namespace scarlet
