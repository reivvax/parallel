#include <stddef.h>
#include <limits.h>
#include <pthread.h>

#include "common/io.h"
#include "worker.h"

#define STACK_FILLING_FACTOR 2

bool fill_stacks(_Atomic LockFreeStack* s, Wrapper* w_a, Wrapper* w_b, InputData* input_data, Solution* best_solution, int threads_count) {
    const Sumset* a;
    a = &w_a->set;

    const Sumset* b;
    b = &w_b->set;
    
    Wrapper* tmp;

    push(s, w_a, w_b); // First element
    size_t stack_size = 1;

    // Basically the 'worker' code
    do {
        bool got_element = pop(s, &w_a, &w_b);
        
        if (!got_element)
            return true;

        stack_size--;
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
                    stack_size++;
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

    } while (stack_size < STACK_FILLING_FACTOR * threads_count);

    return false;
}

int main()
{
    LOG("START");
    InputData input_data;
    input_data_read(&input_data);
    // input_data_init(&input_data, 8, 20, (int[]){0}, (int[]){1, 0});

    Solution best_solution;
    solution_init(&best_solution);

    _Atomic LockFreeStack s = {0, NULL};
    pthread_cond_t wait_for_work;
    pthread_cond_init(&wait_for_work, NULL);
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);
    int waiting_for_work = 0;
    bool threads_done = false;

    WorkerArgs args[input_data.t]; 
    for (int i = 0; i < input_data.t; ++i)
        args_init(args + i, i, &s, &wait_for_work, &mutex, &waiting_for_work, &threads_done, &input_data);

    Wrapper* w_a = init_wrapper(1, NULL);
    w_a->set = input_data.a_start;
    Wrapper* w_b = init_wrapper(1, NULL);
    w_b->set = input_data.b_start;

    bool solved = fill_stacks(&s, w_a, w_b, &input_data, &best_solution, input_data.t);

    if (!solved) {
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
