#!/bin/bash

VALGRIND_CMD="valgrind --leak-check=full --error-exitcode=1 --quiet"
PROGRAM="../build/scarlet"
TEST_DIR="tests"

GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

check_leak() {
    local test_file=$1
    local test_name=$(basename "$test_file" .c)

    echo -n "Testing $test_name... "

    if $VALGRIND_CMD $PROGRAM "$test_file" 1>/dev/null 2>/dev/null; then
        echo -e "${GREEN}OK${NC}"
        return 0
    else
        echo -e "${RED}FAILED${NC}"
        echo "Running again with full output:"
        valgrind --leak-check=full $PROGRAM "$test_file"
        return 1
    fi
}

for test in $TEST_DIR/test{1..5}.c; do
    check_leak "$test"
done

for exe in test{1..5}; do
    rm $exe
done
