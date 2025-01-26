#pragma once

#include <string>
namespace scarlet{
    namespace unop{
enum class UNOP { UNKNOWN, COMPLEMENT, NEGATE };
std::string to_string_unop(UNOP unop);
    }
}

