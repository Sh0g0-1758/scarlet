#pragma once

#include <string>
namespace scarlet {
namespace unop {
enum class UNOP { UNKNOWN, COMPLEMENT, NEGATE };
std::string to_string(UNOP unop);
} // namespace unop
} // namespace scarlet
