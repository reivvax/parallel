#include <stddef.h>

#include "common/io.h"
#include "stack.h"

void solve(InputData* input_data, Solution* best_solution) {
    Stack s = {0};

    const Sumset* a;
    Wrapper* w_a = init_wrapper(1, NULL);
    w_a->set = input_data->a_start;
    a = &w_a->set;

    const Sumset* b;
    Wrapper* w_b = init_wrapper(1, NULL);
    w_b->set = input_data->b_start;
    b = &w_b->set;

    Wrapper* tmp;

    push(&s, w_a, w_b); // First element
    size_t max_size = 0;
    size_t wrappers = 2;

    do {
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
            for (size_t i = a->last; i <= input_data->d; ++i)
                if (!does_sumset_contain(b, i)) {
                    elems++;
                    wrappers++;
                    Wrapper* new_wrapper = init_wrapper(1, w_a);

                    sumset_add(&new_wrapper->set, a, i);
                    push(&s, new_wrapper, w_b);
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
    } while (!empty(&s));
    LOG("WRAPPERS ALLOCATED: %ld", wrappers);
    LOG("MAX STACK_SIZE: %ld", max_size);
    LOG("DONE\n");
}

int main() {
    
    InputData input_data;
    input_data_read(&input_data);
    // input_data_init(&input_data, 8, 10, (int[]){0}, (int[]){1, 0});

    Solution best_solution;
    solution_init(&best_solution);

    solve(&input_data, &best_solution);

    solution_print(&best_solution);
    return 0;
}
