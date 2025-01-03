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
T_LIMIT=5

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
D_VALUES=(5 10 15 20)


current_t=0

for d in $D_VALUES; do # Iteration over d parameter

    while true; do # Iteration over t parameter

        if (( current_t > T_LIMIT )); then
            break
        fi

        THREADS_COUNT=$(( 2 ** current_t ))

        if [ "$BRIEF_MODE" = "true" ]; then
            echo -e "${BOLD}NOT IMPLEMENTED${NC}"
        else
            
            if [ "$RUN_REFERENCE" = "true" ]; then
                echo -e "${BOLD}NOT IMPLEMENTED${NC}"
            else
                # here run the program witch such command:
                # time -f'<SOME FSTRING TO GET REAL TIME IN SECONDS> echo 1 $d 0 1 1 | $REFERENCE > (SOME VARIABLE)
                reference_output=$( (time -f\'%e\' echo "1 $d 0 1 1" | $REFERENCE) 2>&1 )
                reference_real_time=$(echo "$reference_output" | awk '{print $1}')

                # time -f'<SOME FSTRING TO GET REAL TIME AND USER TIME IN SECONDS> echo $THREADS_COUNT $d 0 1 1 | $PARALLEL > (SOME VARIABLE)
                parallel_output=$( { time echo "$THREADS_COUNT $d 0 1 1" | $PARALLEL; } 2>&1 )
                parallel_real_time=$(echo "$parallel_output" | grep real | awk '{print $2}')                
                parallel_user_time=$(echo "$parallel_output" | grep user | awk '{print $2}')
                
                cpu_utilization=$(echo "$parallel_user_time / ($parallel_real_time * $THREADS_COUNT)" | bc -l)

                parallelization_factor=$(echo "$reference_real_time / $parallel_real_time" | bc -l)


                echo -e "For d=$d and THREADS_COUNT=$THREADS_COUNT:"
                echo -e "  Reference Real Time: $reference_real_time seconds"
                echo -e "  Parallel Real Time: $parallel_real_time seconds"
                echo -e "  CPU Utilization: $cpu_utilization"
                echo -e "  Parallelization Factor: $parallelization_factor"
            fi
        fi

        current_t=$((current_t+1))
    done
done

rm *.out