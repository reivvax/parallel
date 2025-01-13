#ifndef WORKER_H
#define WORKER_H

#include "stack.h"
#include "monitor.h"

#define ITERS_TO_SHARE_WORK 128
#define STACK_SIZE_TO_SHARE_WORK 32

typedef struct WorkerArgs {
    int id;
    Monitor* m;
    InputData* input_data;
    Solution best_solution;
    Stack s;                     // Initial stack for worker
    bool* done;                  // Indicator whether the whole computation is done
} WorkerArgs;

void args_init(WorkerArgs* args, int id, Monitor* m, InputData* input_data) {
    args->id = id;
    args->m = m;
    args->input_data = input_data;
    solution_init(&args->best_solution);
    args->s.size = 0;
    args->done = &m->done;
}

void* worker(void* args) {
    // Unpack arguments
    WorkerArgs* unpacked_args = args;

    int id = unpacked_args->id;
    Monitor* m = unpacked_args->m;
    InputData* input_data = unpacked_args->input_data;
    Solution* best_solution = &unpacked_args->best_solution;
    Stack* s = &unpacked_args->s;
    _Atomic bool* collective_stack_empty = &m->empty;
    bool* done = unpacked_args->done;
    
    uint32_t loop_counter = 0;
    uint32_t total_counter = 0;

    size_t max_size = 0;
    const Sumset* a;
    Wrapper* w_a;

    const Sumset* b;
    Wrapper* w_b;

    Wrapper* tmp;
    do {
        if (empty(s)) { // We are out of nodes, ask the monitor for more
            take_work(m, s, id);
            loop_counter = 0;
            continue;
        }

        loop_counter++;
        total_counter++;

        if (loop_counter >= ITERS_TO_SHARE_WORK && size(s) >= STACK_SIZE_TO_SHARE_WORK && *collective_stack_empty) { // Share work
            share_work(m, s, id);
            loop_counter = 0;
            continue;
        }

        pop(s, &w_a, &w_b);
        
        if (w_a->set.sum > w_b->set.sum) {
            tmp = w_a;
            w_a = w_b;
            w_b = tmp;
        }

        a = &w_a->set;
        b = &w_b->set;

        if (is_sumset_intersection_trivial(a, b)) {
            int elems = 0;
            // for (size_t i = input_data->d; i >= a->last; --i)
            for (size_t i = a->last; i <= input_data->d; ++i)
                if (!does_sumset_contain(b, i)) {
                    Wrapper* new_wrapper = init_wrapper(1, w_a);
                    sumset_add(&new_wrapper->set, a, i);
                    push(s, new_wrapper, w_b);
                    elems++;
                }
            if (elems == 0) {
                try_dealloc_wrapper_with_decrement(w_a); // Decrement, as we popped from the stack
                try_dealloc_wrapper_with_decrement(w_b);
            } else {
                increment_ref_counter_n(w_a, elems - 1); // -1, as we popped from the stack
                increment_ref_counter_n(w_b, elems - 1);
            }
        }
        else {
            // The branch is finished
            if (a->sum == b->sum && get_sumset_intersection_size(a, b) == 2 && a->sum > best_solution->sum)
                solution_build(best_solution, input_data, a, b);

            try_dealloc_wrapper_with_decrement(w_a); // Decrement, as we popped from the stack
            try_dealloc_wrapper_with_decrement(w_b);
        }

        if (size(s) > max_size)
            max_size = size(s);

    } while (!(*done)); // Monitor will indicate that the whole work is done, 
    // the data race is not an issue, as the data in `done` address is modified iff all the workers are waiting on condition variable

    LOG("%d: Worker DONE, loops: %d, max stack size: %ld", id, total_counter, max_size);
    return 0;
}

#endif