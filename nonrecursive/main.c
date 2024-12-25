#include <stddef.h>

#include "common/io.h"
#include "stack.h"
#include "utils.h"

void solve(InputData* input_data, Solution* best_solution) {
    Sumset* a;
    a = &input_data->a_start;
    Sumset* b;
    b = &input_data->b_start;
    
    bool removal = false;
    Stack s;
    stack_init(&s);

    do {
        Node* t = top(&s);
        if (t && t->set->prev == b) { // fix the order
            Sumset* tmp = a;
            a = b;
            b = tmp;
        }

        pop_ret(&s, &a, &removal); // current a
        LOG("POP\n");
        if (removal) {
            free(a);
            continue;
        }

        if (a->sum > b->sum) { // swap
            Sumset* tmp = a;
            a = b;
            b = tmp;
        }

        if (is_sumset_intersection_trivial(a, b)) {
            // for (size_t i = a->last; i <= input_data->d; ++i)
            for (size_t i = input_data->d; i >= a->last; --i)
                if (!does_sumset_contain(b, i)) {
                    Sumset* new_sumset = (Sumset*) malloc(sizeof(Sumset));
                    ASSERT_MALLOC_SUCCEEDED(new_sumset);
                    LOG("PUSH\n");
                    sumset_add(new_sumset, a, i);
                    push(&s, new_sumset, true); // indicator to free new_sumset
                    push(&s, new_sumset, false);
                }
        }
        else if (a->sum == b->sum && get_sumset_intersection_size(a, b) == 2) {
            LOG("END OF BRANCH\n");
            if (a->sum > best_solution->sum)
                solution_build(best_solution, input_data, a, b);
        }

    } while (!empty(&s));
    LOG("DONE\n");
}

int main() {
    
    InputData input_data;
    // input_data_read(&input_data);
    input_data_init(&input_data, 8, 10, (int[]){0}, (int[]){1, 0});

    Solution best_solution;
    solution_init(&best_solution);

    solve(&input_data, &best_solution);

    solution_print(&best_solution);
    return 0;
}
