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

    if diff -q <(head -n 1 reference.out) <(head -n 1 nonrecursive.out) > /dev/null && diff -q <(head -n 1 reference.out) <(head -n 1 parallel.out) > /dev/null; then
        echo -e "Test $counter: \t${GREEN}PASS${NC}"
    else
        echo -e "Test $counter: \t${RED}FAIL${NC}"
        if ! diff <(head -n 1 reference.out) <(head -n 1 nonrecursive.out) > nonrecursive.diff; then
            echo "NONRECURSIVE output differs"
        fi
        if ! diff <(head -n 1 reference.out) <(head -n 1 parallel.out) > parallel.diff; then
            echo "PARALLEL output differs"
        fi
        exit 1
    fi

    counter=$((counter+1))
done

rm *.out