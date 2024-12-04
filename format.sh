#!/bin/bash

files=$(find . -type f \
    -not -path "./.git/*" \
    -not -path "./tests/*" \
    -not -path "./test_framework/*" \
    \( -name "*.cpp" -o -name "*.hpp" -o -name "*.c" -o -name "*.h" -o -name "*.cc" -o -name "*.hh" \))

for file in $files; do
    clang-format -i "$file"
done
