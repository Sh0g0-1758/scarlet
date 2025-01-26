#pragma once

#include <string>

enum class UNOP { UNKNOWN, COMPLEMENT, NEGATE };
std::string to_string_unop(UNOP unop);
