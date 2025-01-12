#ifndef WORKER_H
#define WORKER_H

#include <pthread.h>

#include "lock_free_stack.h"

// #define ITERS_TO_SHARE_WORK 256 // 2^8
// #define MAX_MONITOR_SIZE 256
// #define STACK_SIZE_TO_SHARE_WORK 10

typedef struct WorkerArgs {
    int id;
    _Atomic LockFreeStack* s;               // One collective stack
    pthread_cond_t* wait_for_work;
    pthread_mutex_t* mutex;
    int* waiting_for_work;
    bool* done;
    InputData* input_data;
    Solution best_solution;
} WorkerArgs;

void args_init(WorkerArgs* args, int id, _Atomic LockFreeStack* s, pthread_cond_t* wait_for_work, pthread_mutex_t* mutex, int* waiting_for_work, bool* done, InputData* input_data) {
    args->id = id;
    args->s = s;
    args->wait_for_work = wait_for_work;
    args->mutex = mutex;
    args->waiting_for_work = waiting_for_work;
    args->done = done;
    args->input_data = input_data;
    solution_init(&args->best_solution);
}

void* worker(void* args) {
    // Unpack arguments
    WorkerArgs* unpacked_args = args;

    // int id = unpacked_args->id;
    
    // Synchronization arguments
    _Atomic LockFreeStack* s = unpacked_args->s;
    pthread_cond_t* wait_for_work = unpacked_args->wait_for_work;
    pthread_mutex_t* mutex = unpacked_args->mutex;
    int* waiting_for_work = unpacked_args->waiting_for_work;
    bool* done = unpacked_args->done;

    // Data arguments
    InputData* input_data = unpacked_args->input_data;
    Solution* best_solution = &unpacked_args->best_solution;

    uint64_t total_counter = 0;

    const Sumset* a;
    Wrapper* w_a;

    const Sumset* b;
    Wrapper* w_b;

    Wrapper* tmp;
    do {
        total_counter++;
    
        bool got_element = pop(s, &w_a, &w_b);

        if (!got_element) { // Stack is empty
            ASSERT_ZERO(pthread_mutex_lock(mutex));
            if (*waiting_for_work == input_data->t - 1) { // Computation done
                *done = true;
                ASSERT_ZERO(pthread_cond_broadcast(wait_for_work));
                break;
            }

            (*waiting_for_work)++;
            ASSERT_ZERO(pthread_cond_wait(wait_for_work, mutex));
            (*waiting_for_work)--;
            ASSERT_ZERO(pthread_mutex_unlock(mutex));
            
            if (*done)
                break;

            continue;
        }

        if (w_a->set.sum > w_b->set.sum) {
            tmp = w_a;
            w_a = w_b;
            w_b = tmp;
        }

        a = &w_a->set;
        b = &w_b->set;

        if (is_sumset_intersection_trivial(a, b)) {
            int elems = 0;
            for (size_t i = input_data->d; i >= a->last; --i)
                if (!does_sumset_contain(b, i)) {
                    elems++;

                    Wrapper* new_wrapper = init_wrapper(1, w_a);

                    sumset_add(&new_wrapper->set, a, i);
                    push(s, new_wrapper, w_b);
                    ASSERT_ZERO(pthread_cond_signal(wait_for_work));
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
    } while (true); 

    LOG("%d: Worker DONE, loops: %d, max stack size: %ld", id, total_counter, max_size);
    return 0;
}

#endif