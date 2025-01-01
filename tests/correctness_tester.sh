#!/bin/bash

REFERENCE=$(realpath "../build/reference/reference")
NONRECURSIVE=$(realpath "../build/nonrecursive/nonrecursive")
PARALLEL=$(realpath "../build/parallel/parallel")

THREADS_FOR_PARALLEL=8

TEST_DIR="./correctness_tests"
OUTPUT_DIR="./correctness_output"

GREEN="\033[0;32m"
RED="\033[0;31m"
NC="\033[0m"

echo -e "===Running tests ${RED}without valgrind${NC}==="

counter=1

limit="prlimit --as=$(($THREADS_FOR_PARALLEL*128*1024*1024)) time -f'%e seconds, %M kb (max RSS),  exit=%x' timeout --foreground 10s"

for test_file in "$TEST_DIR"/test_*.in; do
    
    "$REFERENCE" < $test_file | head -n 1 > "$OUTPUT_DIR/reference.out"
    (eval $limit "$NONRECURSIVE" < "$test_file" | head -n 1 > $OUTPUT_DIR/nonrecursive.out) 2> "$OUTPUT_DIR/nonrecursive.log"
    (eval $limit "$PARALLEL" < "$test_file" | head -n 1 > $OUTPUT_DIR/parallel.out) 2> "$OUTPUT_DIR/parallel.log"

    if diff -q $OUTPUT_DIR/reference.out $OUTPUT_DIR/nonrecursive.out > /dev/null && diff -q $OUTPUT_DIR/reference.out $OUTPUT_DIR/parallel.out > /dev/null; then
        echo -e "Test $counter: \t${GREEN}PASS${NC}"
    else
        echo -e "Test $counter: \t${RED}FAIL${NC}"
        if ! diff $OUTPUT_DIR/reference.out $OUTPUT_DIR/nonrecursive.out > $OUTPUT_DIR/nonrecursive.diff; then
            echo "NONRECURSIVE output differs"
        fi
        if ! diff $OUTPUT_DIR/reference.out $OUTPUT_DIR/parallel.out > $OUTPUT_DIR/parallel.diff; then
            echo "PARALLEL output differs"
        fi
        exit 1
    fi

    counter=$((counter+1))
done

echo -e "===Running tests ${RED}with valgrind${NC}==="

counter=1

for test_file in "$TEST_DIR"/test_*.in; do
    
    valgrind --error-exitcode=1 --leak-check=full --show-leak-kinds=all --errors-for-leak-kinds=all -q "$NONRECURSIVE" < $test_file > /dev/null 2> $OUTPUT_DIR/nonrecursive.val

    if [ $? -ne 0 ] ; then
        echo -e "Test $counter: \t${RED}FAIL${NC}"
        echo -e "Nonrecursive failed valgrind (details in $OUTPUT_DIR/nonrecursive.val)"
        exit 1
    fi
    
    valgrind --error-exitcode=1 --leak-check=full --show-leak-kinds=all --errors-for-leak-kinds=all -q "$PARALLEL" < $test_file > /dev/null 2> $OUTPUT_DIR/parallel.val

    if [ $? -ne 0 ] ; then
        echo -e "Test $counter: \t${RED}FAIL${NC}"
        echo -e "Parallel failed valgrind (details in $OUTPUT_DIR/parallel.val)"
        exit 1
    fi

    echo -e "Test $counter: \t${GREEN}PASS${NC}"
    
    counter=$((counter+1))
done

rm -f $OUTPUT_DIR/*.out $OUTPUT_DIR/*.log $OUTPUT_DIR/*.diff $OUTPUT_DIR/*.val