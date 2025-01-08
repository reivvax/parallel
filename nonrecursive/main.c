#include <stddef.h>

#include "common/io.h"
#include "stack.h"
#include "sumset_stack.h"

void solve(InputData* input_data, Solution* best_solution) {
    Stack s = {0};
    SumsetStack ss = {0};
    WrapperNode* w_a;
    WrapperNode* w_b;

    if (input_data->a_start.sum < input_data->b_start.sum) {
        w_b = s_push(&ss, NULL);
        w_a = s_push(&ss, NULL);
    } else {
        w_b = s_push(&ss, NULL);
        w_a = s_push(&ss, NULL);
    }
    
    w_b->s = input_data->b_start;
    w_a->s = input_data->a_start;
    WrapperNode* tmp;
    
    const Sumset* a;
    const Sumset* b;

    push(&s, w_a, w_b); // First element
    size_t max_s_size = 0;
    size_t max_ss_size = 0;

    do {
        pop(&s, &w_a, &w_b);

        if (w_a->s.sum > w_b->s.sum) {
            tmp = w_a;
            w_a = w_b;
            w_b = tmp;
        }

        a = &w_a->s;
        b = &w_b->s;

        if (is_sumset_intersection_trivial(a, b)) {
            int elems = 0;
            for (size_t i = a->last; i <= input_data->d; ++i)
                if (!does_sumset_contain(b, i)) {
                    elems++;
                    WrapperNode* pushed = s_push(&ss, w_a);
                    sumset_add(&pushed->s, a, i); // init pushed->s
                    push(&s, pushed, w_b);
                }

            increment_ref_counter(w_a, elems);
        } else {
            // The branch is finished
            if (a->sum == b->sum && get_sumset_intersection_size(a, b) == 2 && a->sum > best_solution->sum)
                solution_build(best_solution, input_data, a, b);

            LOG("TRYING TO DEALLOCATE, STACK SIZE: %ld", ss.size);
            decrement_ref_counter(w_a);
            try_dealloc_top(&ss);
            LOG("AFTER DEALLOCATE, STACK SIZE: %ld", ss.size);

        }

        if (s_size(&ss) > max_ss_size)
            max_ss_size = s_size(&ss);
        if (size(&s) > max_s_size)
            max_s_size = size(&s);

    } while (!empty(&s));

    LOG("MAX S SIZE: %ld", max_s_size);
    LOG("MAX SS SIZE: %ld", max_ss_size);

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
