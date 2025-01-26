#pragma once

#include <string>

namespace Scarlet {
namespace Unop {
enum class UNOP { UNKNOWN, COMPLEMENT, NEGATE };
std::string to_string_unop(UNOP unop);
}
}