# scarlet

A custom Compiler in C++. The goal is to learn more about compilers and implement a blazing fast 
machine learning framework on top of it. 

# Roadmap

1. Implement a compiler driver which has custom lexer, parser, assembly generation pass and code emission pass. Since we are not concerned with linking and preprocessing for now, we use their gcc implementations. [DONE]
2. Use Boost Program Options instead of manually handling argc and argv functions.
3. (Lexer.cc) Alternative to erasing the file_contents text repeatedly? [DONE]
4. (lexer.cc) Take characters from filestream. [DONE]
5. (parser.cc) {Line:125} unop set_op can be enum also.
6. Generate txt file for scar.
7. Create static library for lexer, parser, codegen.

# Contributing Guidlines

1. Please follow conventional commits: https://www.conventionalcommits.org/en/v1.0.0/
2. Please do your changes on a separate branch
3. Try to make your PRs as atomic as possible. 
