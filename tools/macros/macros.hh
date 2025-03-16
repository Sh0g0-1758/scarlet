#pragma once

#include <iostream>
#include <memory>

#define NOTNULL(x) x != nullptr
#define UNREACHABLE()                                                          \
  std::cout << "Unreachable code reached in " << __FILE__ << " at line "       \
            << __LINE__ << std::endl;                                          \
  __builtin_unreachable();
#define MAKE_SHARED(a, b) std::shared_ptr<a> b = std::make_shared<a>()
