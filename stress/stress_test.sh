#!/bin/bash

# Usage: ./stress_test.sh <compiler>
if [ $# -ne 1 ]; then
   echo "Usage: $0 <compiler>"
   exit 1
fi


CC=$1

# We only build and test scarlet against gcc and clang
if ! [[ "$CC" =~ ^(gcc|clang)$ ]]; then
   echo "Error: Compiler must be either 'gcc' or 'clang'"
   echo "Usage: $0 <compiler>"
   exit 1
fi

SCARLET="../build/scarlet"
TEST_DIR="tests"

GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

run_test() {
    local test_file=$1
    local test_name=$(basename "$test_file" .c)

    echo -n "Testing $test_name... "

    # Compile and execute the test using CC (gcc/clang)
    $CC -o "$test_name" "$test_file"
    ./"$test_name"
    local CC_return_code=$?
    rm "$test_name"

    # Compile and execute the test using SCARLET
    $SCARLET "$test_file" 1>/dev/null 2>/dev/null
    ./"$test_name"
    local SCARLET_return_code=$?
    rm "$test_name"

    if [ "$CC_return_code" != "$SCARLET_return_code" ]; then
        echo -e "${RED}ERROR: Return codes do not match${NC}"
        echo "CC return code: $CC_return_code"
        echo "SCARLET return code: $SCARLET_return_code"
        echo "Test file: $test_file"
        return 1
    else
        echo -e "${GREEN}OK${NC}"
        return 0
    fi
}

for test in $TEST_DIR/test{1..5}.c; do
    run_test "$test"
done
