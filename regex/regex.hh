#pragma once

namespace scarlet {
namespace regex {
class Regex {
public:
  bool matchDigit(char c);
  bool matchWord(char c);
  bool matchASCIIPrintable(char c);
  bool matchEscape(char c);
};
} // namespace regex
} // namespace scarlet
