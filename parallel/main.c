#include <stddef.h>
#include <pthread.h>

#include "common/io.h"
#include "monitor.h"
#include "worker.h"

#define MAX_THREADS 64

void fill_stacks(WorkerArgs args[], InputData* input_data, Solution* best_solution, int threads_count) {
    Stack s;
    stack_init(&s);
    int counter = 0;
    bool first = true;
    const Sumset* a;
    a = &input_data->a_start;
    const Sumset* b;
    b = &input_data->b_start;
    const Sumset* tmp;

    // Basically the 'worker' code
    do {
        if (!first && empty(&s))
            return; // WE ARE DONE...? SOMEHOW

        first = false;
        Node* top = pop(&s);
        a = top->a;
        b = top->b;
        
        if (a->sum > b->sum) {
            tmp = a;
            a = b;
            b = tmp;
        }

        if (is_sumset_intersection_trivial(a, b)) {
            // build to_add array that indicates which elements to add
            // bool to_add[a->last - input_data->d];
            // for (size_t i = a->last; i >= input_data->d; ++i) {
            //     if (!does_sumset_contain(b, i))
            //         to_add[i] = true;
            //     else
            //         to_add[i] = false;
            // }

            Wrapper* wrapper = init_wrapper(0, a, top->wrapper);
            int elems = 0;
            for (size_t i = input_data->d; i >= a->last; --i)
                if (!does_sumset_contain(b, i)) {
                    elems++;
                    counter++;

                    Sumset* new_sumset = (Sumset*) malloc(sizeof(Sumset));
                    ASSERT_MALLOC_SUCCEEDED(new_sumset);

                    sumset_add(new_sumset, a, i);
                    Data data = (Data) {.a = new_sumset, .b = b, .wrapper = wrapper};
                    push(&s, &data);
                }
            
            if (elems == 0) {
                // DOES THIS BRANCH EVEN EXECUTE ANYTIME? CHECK IT, FOR NOW LEAVE IT
                printf("YES THIS BRANCH EXECUTES\n");
                // free(a);
                // dealloc_wrapper(top->wrapper);
            } else {
                set_counter(wrapper, elems);
            }
        }
        else {
            // The branch is finished
            if (a->sum == b->sum && get_sumset_intersection_size(a, b) == 2 && a->sum > best_solution->sum)
                solution_build(best_solution, input_data, a, b);
            
            // Dealloc current a and wrapper
            free(a);
            dealloc_wrapper(top->wrapper);
        }
    } while (counter < threads_count); // Assure that every thread has elems on its stack

    int current_stack = 0;

    // OPTIONALLY REDISTRIBUTE THOSE NODES IN MORE REASONABLE MANNER
    while (empty(&s)) {
        Node* top = pop(&s);
        Data data = (Data) {.a = top->a, .b = top->b, .wrapper = top->wrapper};
        push(&args[current_stack].s, &data);
        current_stack = (current_stack + 1) % threads_count;
    }
}

int main()
{
    InputData input_data;
    // input_data_read(&input_data);
    input_data_init(&input_data, 8, 10, (int[]){0}, (int[]){1, 0});

    Solution best_solution;
    solution_init(&best_solution);

    Monitor m;
    monitor_init(&m, input_data.d);

    WorkerArgs args[MAX_THREADS]; 
    for (int i = 0; i < input_data.t; ++i)
        args_init(args + i, &m, &input_data, &best_solution);

    fill_stacks(args, &input_data, &best_solution, input_data.d);

    pthread_t threads[MAX_THREADS];

    for (int i = 0; i < input_data.t; ++i)
        pthread_create(threads + i, NULL, worker, args + i);

    for (int i = 0; i < input_data.t; ++i) // wait for threads
        ASSERT_ZERO(pthread_join(threads + i, NULL));

    solution_print(&best_solution);
    return 0;
}
