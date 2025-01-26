# Scarlet

Scarlet is a custom compiler developed in C++, designed with the primary goal of enhancing our understanding of compiler construction. The overarching objective is to implement an exceptionally fast machine learning framework built atop this compiler.

## Building Scarlet

To build Scarlet, invoke `cmake` with the `DCMAKE_CXX_COMPILER` flag within the build directory.

```sh
cmake -DCMAKE_CXX_COMPILER=<compiler> ..
```

## Testing Scarlet

To test Scarlet, execute the following command within the build directory:

```sh
make test-scarlet
```

## Supported Architectures

Scarlet currently supports the following architectures:

| Operating System | Compiler | Supported Architectures |
| ---------------- | -------- | ----------------------- |
| Ubuntu           | GCC      | x86_64                  |
| Ubuntu           | Clang    | x86_64                  |
| macOS            | Clang    | ARM64                   |

## Roadmap

- [x] Implement a compiler driver with a custom lexer, parser, assembly generation pass, and code emission pass. (Note: We rely on GCC implementations for linking and preprocessing at this stage.)
- [x] Integrate Boost Program Options to replace manual handling of `argc` and `argv` functions.
- [x] (Lexer.cc) Find an alternative to erasing the `file_contents` text repeatedly.
- [x] (Lexer.cc) Refactor to read characters directly from the file stream.
- [x] (Parser.cc) Refactor `unop set_op` (Line:125) to use enums.
- [x] Generate a `.txt` file for the Scarlet output. {Not needed as it will incur a lot of overhead and serves no purpose}
- [x] Create a static library for the lexer, parser, and code generation components.

## Contributing Guidelines

We ask all contributors to adhere to the following guidelines:

- Please follow the Conventional Commits specification: [Conventional Commits](https://www.conventionalcommits.org/en/v1.0.0/).
- Make your changes on a separate branch.
- Ensure that your Pull Requests (PRs) are atomic, addressing one change or feature at a time.
