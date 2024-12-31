#!/bin/bash

REFERENCE="./build/reference/reference"
NONRECURSIVE="./build/nonrecursive/nonrecursive"
PARALLEL="./build/parallel/parallel"

TEST_DIR="./tests"

GREEN="\033[0;32m"
RED="\033[0;31m"
NC="\033[0m"

counter=1

for test_file in "$TEST_DIR"/test_*.in; do
    
    "$REFERENCE" < $test_file > reference.out
    "$NONRECURSIVE" < $test_file > nonrecursive.out
    "$PARALLEL" < $test_file > parallel.out

    if diff -q reference.out nonrecursive.out > /dev/null && diff -q reference.out parallel.out > /dev/null; then
        echo -e "Test $counter: \t${GREEN}PASS${NC}"
    else
        echo -e "Test $counter: \t${RED}FAIL${NC}"
        if ! diff -q reference.out nonrecursive.out > nonrecursive.diff; then
            echo "NONRECURSIVE Output Differs"
        fi
        if ! diff -q reference.out parallel.out > parallel.diff; then
            echo "PARALLEL Output Differs"
        fi
        exit 1
    fi

    counter=$((counter+1))
done

rm reference.out nonrecursive.out parallel.out nonrecursive.diff parallel.diff