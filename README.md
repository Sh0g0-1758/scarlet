# Scarlet

Scarlet is a highly customizable C compiler written in C++. It is compliant with the systemV ABI specification and follows the latest C standard (c23). Its correctness have been ratified over nearly 1100 test-cases against both gcc and clang which makes sure that it handles nearly every obscure C program one can ever write. Every stage of scarlet has been handwritten. Its parser is a recursive descent one with arbitary lookahead and it has its own IR (we call it scar) which is quite intuitive to follow. Coupled with scar optimizations (dead-store,copy-propagation,unreachable-code,constant-folding) and a custom register allocator, the assembly that it generates is very close to what you will get from production compilers like gcc and clang.

## Building Scarlet

[![ubuntu](https://github.com/Sh0g0-1758/scarlet/actions/workflows/UBUNTU_RELEASE_test_gcc.yml/badge.svg)](https://github.com/Sh0g0-1758/scarlet/actions/workflows/UBUNTU_RELEASE_test_gcc.yml)&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;[![ubuntu](https://github.com/Sh0g0-1758/scarlet/actions/workflows/UBUNTU_DEBUG_test_gcc.yml/badge.svg)](https://github.com/Sh0g0-1758/scarlet/actions/workflows/UBUNTU_DEBUG_test_gcc.yml)

[![ubuntu](https://github.com/Sh0g0-1758/scarlet/actions/workflows/UBUNTU_RELEASE_test_clang.yml/badge.svg)](https://github.com/Sh0g0-1758/scarlet/actions/workflows/UBUNTU_RELEASE_test_clang.yml)&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;[![ubuntu](https://github.com/Sh0g0-1758/scarlet/actions/workflows/UBUNTU_DEBUG_test_clang.yml/badge.svg)](https://github.com/Sh0g0-1758/scarlet/actions/workflows/UBUNTU_DEBUG_test_clang.yml)

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

## How to use Scarlet

```
Usage: scarlet [options] file...
Options:
  -h [ --help ]                 produce this help message
  -v [ --version ]              print version string
  --lex                         run only till the lexer stage
  --parse                       run only till the parser stage and print the 
                                AST
  --validate                    run semantic analysis and print the AST
  --scar                        print scar(IR for scarlet)
  --fold-constants              Enable constant folding
  --propagate-copies            Enable copy propagation
  --eliminate-unreachable-code  Enable unreachable code elimination
  --eliminate-dead-stores       Enable dead store elimination
  -O [ --optimize ]             Enable all optimizations
  -S [ --asm ]                  generate .s file containing X86-64 assembly
  -c                            Complie and assemble, but do not link
  -l                            specify the libraries you want to link against
  -o [ --output-file ] arg      output file
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
- [x] Add support for variadic argument function calling
- [ ] Add support for user defined structures
- [ ] Add support for NaN
- [x] Add constant folding optimization
- [x] Add a construct for control flow graph
- [x] Add unreachable code elimination optimization
- [x] Add propagate copies optimization
- [x] Add dead store elimination optimization
- [x] Test the complete scar optimization pipeline
- [x] Add a register allocator

## Resources

Our compiler implementation draws primarily from Nora Sandler's 'How to Write a Compiler in C' guide, supplemented with concepts from the Dragon Book. Both resources are highly recommended for anyone studying compiler design.

## What we do not support
We try to be as close to the C standard as possible but there are a few features we do not support as it is in the standard because quite frankly it does not make much sense. 

- We do not support subscripting of type `1[a]` where `a` is an array or a pointer.
- Compound operations and increment/decrement are just syntactic sugars. (They get exapanded to their base expressions ie. `a+=2` will become `a = a + 2` and `b++` will become `b = b + 1`. Note that in post increment/decrement operations, the correct ie. un-updated value will be returned).
- For comparing pointers the base type needs to be the same. So you need to cast a `long*` to a `void*` before comparing it with a `void*`
- The expression `sizeof 'a'` get's evaluated to 1, although gcc and clang say its 4. (come on its a character) 
- We do not support variable length arrays (`int arr[b];`, b known at runtime) or array length initialization using expressions(`int arr[1+1];`)
- We do not support empty parameter lists. You have to explicitly add void to the function parameter list. So `int foo();` should be changed to `int foo(void);`
- We do not support function declaration with no variables. You cannot just write the type-specifier. So `int foo(int, int);` should be changed to `int foo(int a, int b);`
- Declaration like `int x,y;` are not supported. You have to write `int x` and `int y` in seperate lines.
- Array function parameters are not supported, as they are anyway decayed to pointers. You should rewrite `int foo(int a[])` to `int foo(int *a)`.
- We do not support the `const` keyword right now.
- We also do not support `function pointers` right now.
- The cases in a switch statement must be a constant. So you cannot have `unop<constant>` because scarlet will try to evaluate this and thus treat the result as a variable. So `case -100` is wrong. 

## Contributing Guidelines

We ask all contributors to adhere to the following guidelines:

- Please follow the Conventional Commits specification: [Conventional Commits](https://www.conventionalcommits.org/en/v1.0.0/).
- Make your changes on a separate branch.
- Ensure that your Pull Requests (PRs) are atomic, addressing one change or feature at a time.
