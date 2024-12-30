#include <stddef.h>
#include <limits.h>
#include <pthread.h>

#include "common/io.h"
#include "worker.h"

#define MAX_THREADS 64
#define STACK_FILLING_FACTOR 2

void fill_stacks(WorkerArgs args[], Wrapper initial_wrappers[], InputData* input_data, Solution* best_solution, int threads_count) {
    Stack s;
    stack_init(&s);

    const Sumset* a;
    a = &input_data->a_start;
    initial_wrappers[0] = (Wrapper) {.ref_counter = ULLONG_MAX >> 1, .set = *a, .prev = NULL};
    Wrapper* w_a = initial_wrappers;
    
    const Sumset* b;
    b = &input_data->b_start;
    initial_wrappers[1] = (Wrapper) {.ref_counter = ULLONG_MAX >> 1, .set = *b, .prev = NULL};
    Wrapper* w_b = initial_wrappers + 1;

    Wrapper* tmp;

    Data data = (Data) {.a = w_a, .b = w_b};
    push(&s, &data); // First element
    
    // Basically the 'worker' code
    do {
        if (empty(&s))
            return; // WE ARE DONE...? SOMEHOW

        Node* top = pop(&s);
        w_a = top->a;
        w_b = top->b;
        
        if (w_a->set.sum > w_b->set.sum) {
            tmp = w_a;
            w_a = w_b;
            w_b = tmp;
        }

        a = &w_a->set;
        b = &w_b->set;

        if (is_sumset_intersection_trivial(a, b)) {
            int elems = 0;
            for (size_t i = a->last; i <= input_data->d; ++i)
                if (!does_sumset_contain(b, i)) {
                    elems++;

                    Wrapper* new_wrapper = init_wrapper(1, w_a);
                    ASSERT_MALLOC_SUCCEEDED(new_wrapper);

                    sumset_add(&new_wrapper->set, a, i);
                    Data data = (Data) {.a = new_wrapper, .b = w_b};
                    push(&s, &data);
                }
            
            if (elems == 0) {
                fprintf(stderr, "YES THIS BRANCH EXECUTES\n");
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

        free(top);
    } while (size(&s) < STACK_FILLING_FACTOR * threads_count);
    // Assure that every thread will have at least `STACK_FILLING_FACTOR` elements on its stack

    int current_stack = 0;

    // OPTIONALLY REDISTRIBUTE THOSE NODES IN MORE REASONABLE MANNER
    while (!empty(&s)) {
        Node* top = pop(&s);
        Data data = (Data) {.a = top->a, .b = top->b};
        push(&args[current_stack].s, &data);
        current_stack = (current_stack + 1) % threads_count;
    }
}

int main()
{
    InputData input_data;
    // input_data_read(&input_data);
    input_data_init(&input_data, 1, 4, (int[]){0}, (int[]){1, 0});

    Solution best_solution;
    solution_init(&best_solution);

    Monitor m;
    // monitor_init(&m, input_data.d);

    WorkerArgs args[input_data.t]; 
    for (int i = 0; i < input_data.t; ++i)
        args_init(args + i, &m, &input_data, &best_solution);

    Wrapper initial_wrappers[2];

    fill_stacks(args, initial_wrappers, &input_data, &best_solution, input_data.t);

    pthread_t threads[input_data.t];

    for (int i = 0; i < input_data.t; ++i)
        pthread_create(threads + i, NULL, worker, args + i);

    for (int i = 0; i < input_data.t; ++i) // wait for threads
        ASSERT_ZERO(pthread_join(threads[i], NULL));

    solution_print(&best_solution);
    return 0;
}
