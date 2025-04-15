# Scarlet

Scarlet is a custom compiler developed in C++, designed with the primary goal of enhancing our understanding of compiler construction. The overarching objective is to implement an exceptionally fast machine learning framework built atop this compiler.

## Building Scarlet

[![ubuntu](https://github.com/Sh0g0-1758/scarlet/actions/workflows/UBUNTU_RELEASE_test_gcc.yml/badge.svg)](https://github.com/Sh0g0-1758/scarlet/actions/workflows/UBUNTU_RELEASE_test_gcc.yml)&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;[![ubuntu](https://github.com/Sh0g0-1758/scarlet/actions/workflows/UBUNTU_DEBUG_test_gcc.yml/badge.svg)](https://github.com/Sh0g0-1758/scarlet/actions/workflows/UBUNTU_DEBUG_test_gcc.yml)

[![macos](https://github.com/Sh0g0-1758/scarlet/actions/workflows/MACOS_RELEASE_test_clang.yml/badge.svg)](https://github.com/Sh0g0-1758/scarlet/actions/workflows/MACOS_RELEASE_test_clang.yml)&nbsp;&nbsp;&nbsp;[![macos](https://github.com/Sh0g0-1758/scarlet/actions/workflows/MACOS_DEBUG_test_clang.yml/badge.svg)](https://github.com/Sh0g0-1758/scarlet/actions/workflows/MACOS_DEBUG_test_clang.yml)

To build Scarlet, invoke `cmake` with the `DCMAKE_CXX_COMPILER` flag within the build directory and then run `make`. By default, scarlet builds in `Release` mode (-O3).

```sh
cmake -DCMAKE_CXX_COMPILER=<compiler> -DCMAKE_BUILD_TYPE=<Release/Debug> ..
make
```

## Testing Scarlet

[![stress](https://github.com/Sh0g0-1758/scarlet/actions/workflows/UBUNTU_RELEASE_stress_test_gcc.yml/badge.svg)](https://github.com/Sh0g0-1758/scarlet/actions/workflows/UBUNTU_RELEASE_stress_test_gcc.yml)&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;[![stress](https://github.com/Sh0g0-1758/scarlet/actions/workflows/UBUNTU_DEBUG_stress_test_gcc.yml/badge.svg)](https://github.com/Sh0g0-1758/scarlet/actions/workflows/UBUNTU_DEBUG_stress_test_gcc.yml)

[![stress](https://github.com/Sh0g0-1758/scarlet/actions/workflows/UBUNTU_RELEASE_stress_test_clang.yml/badge.svg)](https://github.com/Sh0g0-1758/scarlet/actions/workflows/UBUNTU_RELEASE_stress_test_clang.yml)&nbsp;&nbsp;&nbsp;[![stress](https://github.com/Sh0g0-1758/scarlet/actions/workflows/UBUNTU_DEBUG_stress_test_clang.yml/badge.svg)](https://github.com/Sh0g0-1758/scarlet/actions/workflows/UBUNTU_DEBUG_stress_test_clang.yml)

To test Scarlet, execute the following command within the build directory:

```sh
make test-scarlet
```

To execute our custom stress tests, execute the following command within the `stress` directory:

```sh
./stress_test.sh
```

To check memory leaks in scarlet, run the following command within the `stress` directory:

```sh
./check_memory_leaks.sh
```

## Supported Architectures

Scarlet currently supports the following architectures:

| Operating System | Compiler | Supported Architectures |
| ---------------- | -------- | ----------------------- |
| Ubuntu           | GCC      | x86_64                  |
| Ubuntu           | Clang    | x86_64                  |
| macOS            | Clang    | ARM64 using Rosetta 2   |

## Roadmap

- [x] Implement a compiler driver with a custom lexer, parser, assembly generation pass, and code emission pass. (Note: We rely on GCC implementations for linking and preprocessing at this stage.)
- [x] Integrate Boost Program Options to replace manual handling of `argc` and `argv` functions.
- [x] (Lexer.cc) Find an alternative to erasing the `file_contents` text repeatedly.
- [x] (Parser.cc) Refactor every string to use enums.
- [x] Create a static library for the lexer, parser, and code generation components.
- [x] Add support for Unary Operations (Neg, Not)
- [x] Add support for Binary Operations (Add, Sub, Mul, Div, Rem)
- [x] Add support for Bitwise Binary Operations (And, Or, Xor, LeftShift, RightShift)
- [x] Add support for logical and relational operators (! && || == !=  < > <= >=)
- [x] Add support for user defined local variables
- [x] Add support for if-else construct
- [x] Add support for ternary operator
- [x] Add support for increment/decrement operator
- [x] Add support for compound operators (+=,-=,*=,/=,<<=,>>=,%=)
- [x] Add support for goto statement
- [x] Add support for compound statements
- [x] Add support for Loops (for, while, do-while)
- [x] Add support for switch-case
- [x] Add support for Functions
- [x] Add support for static keyword and global scope
- [x] Add support for a type system
- [x] Add support for Long Integers
- [x] Add support for Unsigned Integers
- [x] Add support for double precision
- [x] Add support for Pointers
- [x] Add support for Pointer Arithmetic
- [x] Add support for Arrays
- [x] Add support for characters and strings
- [x] Add support for dynamic memory allocation
- [ ] Add support for user defined structures
- [ ] Add support for NaN

## Resources

Our compiler implementation draws primarily from Nora Sandler's 'How to Write a Compiler in C' guide, supplemented with concepts from the Dragon Book. Both resources are highly recommended for anyone studying compiler design.

## What we do not support
We try to be as close to the C standard as possible but there are a few features we do not support as it is in the standard because quite frankly it does not make much sense. 

- We do not support subscripting of type `1[a]` where `a` is an array or a pointer.
- Compound operations and increment/decrement are just syntactic sugars. (They get exapanded to their base expressions ie. `a+=2` will become `a = a + 2` and `b++` will become `b = b + 1`. Note that in post increment/decrement operations, the correct ie. un-updated value will be returned).
- For comparing pointers the base type needs to be the same. So you need to cast a `long*` to a `void*` before comparing it with a `void*`
- The expression `sizeof 'a'` get's evaluated to 1, although gcc and clang say its 4. (come on its a character) 

## Contributing Guidelines

We ask all contributors to adhere to the following guidelines:

- Please follow the Conventional Commits specification: [Conventional Commits](https://www.conventionalcommits.org/en/v1.0.0/).
- Make your changes on a separate branch.
- Ensure that your Pull Requests (PRs) are atomic, addressing one change or feature at a time.
