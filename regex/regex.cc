
#include "regex.hh"
namespace scarlet {
namespace regex {

bool Regex::matchWord(char c) {
  if ((c >= 'a' and c <= 'z') or (c >= 'A' and c <= 'Z') or (c == '_'))
    return true;
  return false;
}

bool Regex::matchDigit(char c) {
  if (c >= '0' and c <= '9')
    return true;
  return false;
}
bool Regex::matchASCIIPrintable(char c) {
  if (int(c) >= 32)
    return true;
  return false;
}
bool Regex::matchEscape(char c) {
  if (c == '\'' or c == '\"' or c == '?' or c == 'a' or c == 'b' or c == 'f' or
      c == 'n' or c == 'r' or c == 't' or c == 'v' or c == '\\')
    return true;
  return false;
}
} // namespace regex
} // namespace scarlet
