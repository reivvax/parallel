#!/bin/bash

REFERENCE=$(realpath "../build/reference/reference")
NONRECURSIVE=$(realpath "../build/nonrecursive/nonrecursive")
PARALLEL=$(realpath "../build/parallel/parallel")

TEST_DIR="./tests"
OUTPUT_DIR="./time_output"

BOLD="\033[1m"
GREEN="\033[0;32m"
RED="\033[0;31m"
YELLOW="\033[0;33m"
NC="\033[0m"

BRIEF_MODE=false
RUN_REFERENCE=false
T_LIMIT=5
D_LIMIT=8

# Args
while [[ $# -gt 0 ]]; do
    case "$1" in
        -b|--brief)
            BRIEF_MODE=true
            shift
            ;;
        -r|--reference)
            RUN_REFERENCE=true
            shift
            ;;
        -t|--threads_limit)
            if [[ -n "$2" && "$2" =~ ^[0-9]+$ ]]; then
                T_LIMIT="$2"
                shift 2
            else
                echo "Error: -t|--threads_limit requires a positive integer argument."
                exit 1
            fi
            ;;
        -d|--d_limit)
            if [[ -n "$2" && "$2" =~ ^[0-9]+$ ]]; then
                D_LIMIT="$2"
                shift 2
            else
                echo "Error: -d|--d_limit requires a positive integer argument."
                exit 1
            fi
            ;;
        *)
            echo "Error: Unknown option: $1"
            exit 1
            ;;
    esac
done

if (( T_LIMIT > 7 )); then
    echo -e "${YELLOW}Warning: Running with threads_limit = 7${NC}"
    T_LIMIT=7
fi

if (( D_LIMIT > 8 )); then
    echo -e "${YELLOW}Warning: Running with d_limit = 8${NC}"
    D_LIMIT=8
fi

if [ "$RUN_REFERENCE" = "false" ]; then
    if command -v python3 &>/dev/null; then
        PYTHON_CMD=python3
    elif command -v python &>/dev/null; then
        PYTHON_CMD=python
    elif command -v py &>/dev/null; then
        PYTHON_CMD=py
    else
        echo -e "${RED}Python not found.${NC}" >&2
        exit 1
    fi
fi

D_VALUES=(5 10 15 20 25 30 32 34)

D_VALUES=("${D_VALUES[@]:0:$D_LIMIT}") # Crop

if [ "$BRIEF_MODE" = "true" ]; then
    echo "d,t,reference_real_time,parallel_real_time,parallelization_factor"
fi

for d in ${D_VALUES[@]}; do # Iteration over d parameter

    CURRENT_T=0
    THREADS_COUNT=1
    
    if [ "$RUN_REFERENCE" = "true" ]; then
        reference_real_time=$( (echo "1 $d 0 1 1" | time -f'%e' $REFERENCE > /dev/null) 2>&1 )
        if [ $reference_real_time = "0.00" ]; then
            reference_real_time="0.001" # Avoid division by zero, may happen for d = 5
        fi
    else
        reference_real_time=$( $PYTHON_CMD reference_times.py $d )
        if [ $? -ne 0 ]; then
            echo "Error: Failed to get reference time for d=$d" >&2
            exit 1
        fi
    fi

    while true; do # Iteration over t parameter

        if (( CURRENT_T > T_LIMIT )); then
            break
        fi

        parallel_output=$( (echo "$THREADS_COUNT $d 0 1 1" | time -f'%e %U' $PARALLEL > /dev/null) 2>&1 )
        parallel_real_time=$(echo "$parallel_output" | awk '{print $1}')
        if [ $parallel_real_time = "0.00" ]; then
            NEGLIGIBLE="true"
            parallel_real_time="0.001" # Avoid division by zero, may happen for d = 5
        fi

        parallel_user_time=$(echo "$parallel_output" | awk '{print $2}')
        if [ $parallel_user_time = "0.00" ]; then
            NEGLIGIBLE="true"
            parallel_user_time="0.001" # Avoid division by zero, may happen for d = 5
        fi
        parallelization_factor=$(echo "scale=4; $reference_real_time / $parallel_real_time" | bc -l)
        if [[ $parallelization_factor == .* ]]; then
            parallelization_factor="0$parallelization_factor" # Ensure trailing zero before a comma
        fi
        

        if [ "$BRIEF_MODE" = "true" ]; then
            echo -e "$d,$THREADS_COUNT,$reference_real_time,$parallel_real_time,$parallelization_factor"
        else
            cpu_utilization=$(echo "scale=4; $parallel_user_time / ($parallel_real_time * $THREADS_COUNT)" | bc -l)
            if [[ $cpu_utilization == .* ]]; then
                cpu_utilization="0$cpu_utilization"
            fi
        
            echo -e "d=$d, t=$THREADS_COUNT:"
            echo -e "\tReference real time:\t$reference_real_time seconds"
            echo -e "\tParallel real time:\t$parallel_real_time seconds"
            echo -e "\tParallelization factor:\t$parallelization_factor"
            
            if [ "$NEGLIGIBLE" = "true" ]; then
                cpu_utilization="NEGLIGIBLE"
            fi

            echo -e "\tCPU utilization:\t$cpu_utilization"
        fi

        NEGLIGIBLE="false"
        THREADS_COUNT=$(( 2 * THREADS_COUNT ))
        CURRENT_T=$((CURRENT_T+1))
    done
done
