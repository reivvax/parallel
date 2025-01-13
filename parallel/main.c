#include <stddef.h>
#include <limits.h>
#include <pthread.h>

#include "common/io.h"
#include "worker.h"

#define STACK_FILLING_FACTOR 10

bool fill_stacks(WorkerArgs args[], Wrapper* w_a, Wrapper* w_b, InputData* input_data, Solution* best_solution, int threads_count) {
    Stack s = {0};

    const Sumset* a;
    a = &w_a->set;

    const Sumset* b;
    b = &w_b->set;
    
    Wrapper* tmp;

    push(&s, w_a, w_b); // First element
    size_t max_size = 0;

    // Basically the 'worker' code
    do {
        if (empty(&s))
            return true; // We are already done

        pop(&s, &w_a, &w_b);
        
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
                    Wrapper* new_wrapper = init_wrapper(1, w_a);
                    sumset_add(&new_wrapper->set, a, i);
                    push(&s, new_wrapper, w_b);
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

        if (size(&s) > max_size)
            max_size = size(&s);
    } while (size(&s) < STACK_FILLING_FACTOR * threads_count);
    // Assure that every thread will have at least `STACK_FILLING_FACTOR` elements on its stack

    int current_stack = 0;
    Wrapper* temp_a;
    Wrapper* temp_b;
    while (!empty(&s)) {
        pop(&s, &temp_a, &temp_b);
        push(&args[current_stack].s, temp_a, temp_b);
        current_stack = (current_stack + 1) % threads_count;
    }

    return false;
}

int main()
{
    InputData input_data;
    input_data_read(&input_data);
    // input_data_init(&input_data, 8, 20, (int[]){0}, (int[]){1, 0});

    Solution best_solution;
    solution_init(&best_solution);

    Monitor m = {0};
    monitor_init(&m, input_data.t, input_data.d);
    
    WorkerArgs args[input_data.t]; 
    for (int i = 0; i < input_data.t; ++i) {
        args[i] = (WorkerArgs) {
            .id = 0,
            .m = NULL,
            .input_data = NULL,
            .best_solution = {0},
            .s = {0},
            .done = NULL
        };
        args_init(args + i, i, &m, &input_data);
    }
    Wrapper* w_a = init_wrapper(1, NULL);
    w_a->set = input_data.a_start;
    Wrapper* w_b = init_wrapper(1, NULL);
    w_b->set = input_data.b_start;

    bool done = fill_stacks(args, w_a, w_b, &input_data, &best_solution, input_data.t);

    if (!done) {
        pthread_t threads[input_data.t];

        for (int i = 0; i < input_data.t; ++i)
            pthread_create(threads + i, NULL, worker, args + i);

        for (int i = 0; i < input_data.t; ++i) // wait for threads
            ASSERT_ZERO(pthread_join(threads[i], NULL));
    }

    Solution* res = &best_solution;
    for (int i = 0; i < input_data.t; ++i)
        if (res->sum < args[i].best_solution.sum)
            res = &args[i].best_solution;
    
    solution_print(res);
    return 0;
}
