# Scarlet

Scarlet is a custom compiler developed in C++, designed with the primary goal of enhancing our understanding of compiler construction. The overarching objective is to implement an exceptionally fast machine learning framework built atop this compiler.

## Building Scarlet

[![ubuntu](https://github.com/Sh0g0-1758/scarlet/actions/workflows/UBUNTU_test_gcc.yml/badge.svg)](https://github.com/Sh0g0-1758/scarlet/actions/workflows/UBUNTU_test_gcc.yml) [![macos](https://github.com/Sh0g0-1758/scarlet/actions/workflows/MACOS_test_clang.yml/badge.svg)](https://github.com/Sh0g0-1758/scarlet/actions/workflows/MACOS_test_clang.yml)

To build Scarlet, invoke `cmake` with the `DCMAKE_CXX_COMPILER` flag within the build directory and then run `make`. By default, scarlet builds in `Release` mode (-O3).

```sh
cmake -DCMAKE_CXX_COMPILER=<compiler> -DCMAKE_BUILD_TYPE=<Release/Debug> ..
make
```

## Testing Scarlet

[![stress](https://github.com/Sh0g0-1758/scarlet/actions/workflows/UBUNTU_stress_test_gcc.yml/badge.svg)](https://github.com/Sh0g0-1758/scarlet/actions/workflows/UBUNTU_stress_test_gcc.yml) [![stress](https://github.com/Sh0g0-1758/scarlet/actions/workflows/UBUNTU_stress_test_clang.yml/badge.svg)](https://github.com/Sh0g0-1758/scarlet/actions/workflows/UBUNTU_stress_test_clang.yml)

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
- [ ] Add support for Long Integers
- [ ] Add support for Unsigned Integers
- [ ] Add support for Floating-Point Numbers
- [ ] Add support for Pointers
- [ ] Add support for Arrays
- [ ] Add support for characters and strings
- [ ] Add support for dynamic memory allocation
- [ ] Add support for user defined structures

## Resources

Our compiler implementation draws primarily from Nora Sandler's 'How to Write a Compiler in C' guide, supplemented with concepts from the Dragon Book. Both resources are highly recommended for anyone studying compiler design.

## Contributing Guidelines

We ask all contributors to adhere to the following guidelines:

- Please follow the Conventional Commits specification: [Conventional Commits](https://www.conventionalcommits.org/en/v1.0.0/).
- Make your changes on a separate branch.
- Ensure that your Pull Requests (PRs) are atomic, addressing one change or feature at a time.
