# scarlet

A custom Compiler in C++. The goal is to learn more about compilers and implement a blazing fast 
machine learning framework on top of it. 

# Roadmap

- [x] Implement a compiler driver which has custom lexer, parser, assembly generation pass, and code emission pass. Since we are not concerned with linking and preprocessing for now, we use their gcc implementations.
- [x] Use Boost Program Options instead of manually handling argc and argv functions.
- [x] (Lexer.cc) Alternative to erasing the file_contents text repeatedly.
- [x] (lexer.cc) Take characters from filestream.
- [x] (parser.cc) {Line:125} unop set_op can be enum also.
- [ ] Generate txt file for scar.
- [x] Create static library for lexer, parser, codegen.

# Contributing Guidlines

1. Please follow conventional commits: https://www.conventionalcommits.org/en/v1.0.0/
2. Please do your changes on a separate branch
3. Try to make your PRs as atomic as possible. 
