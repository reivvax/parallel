#!/bin/bash

REFERENCE=$(realpath "../build/reference/reference")
NONRECURSIVE=$(realpath "../build/nonrecursive/nonrecursive")
PARALLEL=$(realpath "../build/parallel/parallel")


TEST_DIR="./correctness_tests"

GREEN="\033[0;32m"
RED="\033[0;31m"
NC="\033[0m"

echo -e "===Running tests ${RED}without valgrind${NC}==="

counter=1

for test_file in "$TEST_DIR"/test_*.in; do
    
    "$REFERENCE" < $test_file | head -n 1 > reference.out
    "$NONRECURSIVE" < $test_file | head -n 1 > nonrecursive.out
    "$PARALLEL" < $test_file | head -n 1 > parallel.out

    if diff -q reference.out nonrecursive.out > /dev/null && diff -q reference.out parallel.out > /dev/null; then
        echo -e "Test $counter: \t${GREEN}PASS${NC}"
    else
        echo -e "Test $counter: \t${RED}FAIL${NC}"
        if ! diff reference.out nonrecursive.out > nonrecursive.diff; then
            echo "NONRECURSIVE output differs"
        fi
        if ! diff reference.out parallel.out > parallel.diff; then
            echo "PARALLEL output differs"
        fi
        exit 1
    fi

    counter=$((counter+1))
done

echo -e "===Running tests ${RED}with valgrind${NC}==="

counter=1

for test_file in "$TEST_DIR"/test_*.in; do
    
    valgrind --error-exitcode=1 --leak-check=full --show-leak-kinds=all --errors-for-leak-kinds=all -q "$NONRECURSIVE" < $test_file > /dev/null 2> nonrecursive.val

    if [ $? -ne 0 ] ; then
        echo -e "Test $counter: \t${RED}FAIL${NC}"
        echo -e "Nonrecursive failed valgrind (details in nonrecursive.val)"
        exit 1
    fi
    
    valgrind --error-exitcode=1 --leak-check=full --show-leak-kinds=all --errors-for-leak-kinds=all -q "$PARALLEL" < $test_file > /dev/null 2> parallel.val

    if [ $? -ne 0 ] ; then
        echo -e "Test $counter: \t${RED}FAIL${NC}"
        echo -e "Parallel failed valgrind (details in parallel.val)"
        exit 1
    fi

    echo -e "Test $counter: \t${GREEN}PASS${NC}"
    
    counter=$((counter+1))
done

rm *.out *.val