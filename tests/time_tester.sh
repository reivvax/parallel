#!/bin/bash
set -x
REFERENCE="../build/reference/reference"
NONRECURSIVE="../build/nonrecursive/nonrecursive"
PARALLEL="../build/parallel/parallel"

TEST_DIR="./tests"
OUTPUT_DIR="./time_output"

BOLD="\033[1m"
GREEN="\033[0;32m"
RED="\033[0;31m"
YELLOW="\033[0;33m"
NC="\033[0m"

OUTPUT_TO_FILE=false
BRIEF_MODE=false
RUN_REFERENCE=false
T_LIMIT=2

# Args
while [[ $# -gt 0 ]]; do
    case "$1" in
        -f|--file)
            OUTPUT_TO_FILE=true
            shift
            ;;
        -b|--brief)
            BRIEF_MODE=true
            shift
            ;;
        -r|--reference)
            RUN_REFERENCE=true
            shift
            ;;
        -n|--limit)
            if [[ -n "$2" && "$2" =~ ^[0-9]+$ ]]; then
                LIMIT="$2"
                shift 2
            else
                echo "Error: -n|--limit requires a positive integer argument."
                exit 1
            fi
            ;;
        *)
            echo "Error: Unknown option: $1"
            exit 1
            ;;
    esac
done

if (( T_LIMIT > 6 )); then
    echo -e "${YELLOW}Warning: Running with limit = 6${NC}"
    T_LIMIT=6
fi

# D_VALUES=(5 10 15 20 25 30 32 34)
D_VALUES=(5 10)


current_t=0
THREADS_COUNT=1

for d in $D_VALUES; do # Iteration over d parameter

    if [ "$RUN_REFERENCE" = "true" ]; then
        start=$(date +%s%N)
        reference_output=$( (echo "1 $d 0 1 1" | $REFERENCE > /dev/null) 2>&1 )
        end=$(date +%s%N)
        reference_real_time=$(((end - start) / 1000000)) # Miliseconds
    else
        echo -e "${BOLD}NOT IMPLEMENTED${NC}"
    fi

    while true; do # Iteration over t parameter

        if (( current_t > T_LIMIT )); then
            break
        fi


        if [ "$BRIEF_MODE" = "true" ]; then
            echo -e "${BOLD}NOT IMPLEMENTED${NC}"
        else
            parallel_output=$( { echo "$THREADS_COUNT $d 0 1 1" | time -f'%e %U' $PARALLEL; } 2>&1 )
            # parallel_real_time=$(echo "$parallel_output" | grep real | awk '{print $2}')
            # parallel_user_time=$(echo "$parallel_output" | grep user | awk '{print $2}')
            parallel_real_time=$(echo "$parallel_output" | awk '{print $1}')
            parallel_user_time=$(echo "$parallel_output" | awk '{print $2}')
            cpu_utilization=$(echo "$parallel_user_time / ($parallel_real_time * $THREADS_COUNT)" | bc -l)

            parallelization_factor=$(echo "$reference_real_time / $parallel_real_time" | bc -l)

            echo -e "For d=$d and THREADS_COUNT=$THREADS_COUNT:"
            echo -e "  Reference Real Time: $reference_real_time seconds"
            echo -e "  Parallel Real Time: $parallel_real_time seconds"
            echo -e "  Parallelization Factor: $parallelization_factor"
            echo -e "  CPU Utilization: $cpu_utilization"
        fi

        THREADS_COUNT=$(( 2 * THREADS_COUNT ))
        current_t=$((current_t+1))
    done
done

rm *.out