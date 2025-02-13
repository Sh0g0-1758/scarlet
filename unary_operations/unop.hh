#pragma once

#include <string>
namespace scarlet {
namespace unop {
enum class UNOP { UNKNOWN, COMPLEMENT, NEGATE, NOT, INCREMENT, DECREMENT };
std::string to_string(UNOP unop);
} // namespace unop
} // namespace scarlet
