#include <stddef.h>

#include "common/io.h"
#include "stack.h"

void solve(InputData* input_data, Solution* best_solution) {
    const Sumset* a;
    a = &input_data->a_start;
    const Sumset* b;
    b = &input_data->b_start;
    const Sumset* tmp;
    NodeType current_type = SUMSET;
    Stack s;
    stack_init(&s);

    do {
        pop_ret(&s, &a, &current_type); // current a
        switch (current_type) {
            case REMOVAL:
                LOG("POP REMOVAL");
                tmp = a;
                a = a->prev;
                free((Sumset*)tmp);
                break;
            
            case SWAP:
                LOG("UNSWAP");
                tmp = a;
                a = b;
                b = tmp;
                break;

            case SUMSET:
                LOG("POP SUMSET");
                if (a->sum > b->sum) {
                    tmp = a;
                    a = b;
                    b = tmp;
                    LOG("SWAP");
                    push(&s, NULL, SWAP);
                }
                // printf("a address: %p\n", a);
                // printf("b address: %p\n", b);
                // print_sumsets(a, b);
                
                if (is_sumset_intersection_trivial(a, b)) {
                    int elems = 0;
                    for (size_t i = input_data->d; i >= a->last; --i)
                        if (!does_sumset_contain(b, i)) {
                            elems++;
                            Sumset* new_sumset = (Sumset*) malloc(sizeof(Sumset));
                            ASSERT_MALLOC_SUCCEEDED(new_sumset);
                            sumset_add(new_sumset, a, i);
                            push(&s, new_sumset, REMOVAL); // indicator to free new_sumset
                            push(&s, new_sumset, SUMSET);
                        }
                    LOG("PUSHED %d X REMOVAL / SUMET", elems);
                }
                else if (a->sum == b->sum && get_sumset_intersection_size(a, b) == 2) {
                    LOG("END OF BRANCH");
                    if (a->sum > best_solution->sum)
                        solution_build(best_solution, input_data, a, b);
                }
        }
    } while (!empty(&s));

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
