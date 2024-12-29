#include "stack.h"
#include "wrapper.h"

#define ITERS_TO_SHARE_WORK 64

typedef struct WorkerArgs {
    Monitor* m;
    InputData* input_data;
    Solution* best_solution;
    Stack s;                     // initial stack for worker
    bool* done;                  // indicator whether the whole computation is done
} WorkerArgs;

void args_init(WorkerArgs* args, Monitor* m, InputData* input_data, Solution* best_solution) {
    args->m = m;
    args->input_data = input_data;
    args->best_solution = best_solution;
    stack_init(&args->s);
    args->done = &m->done;
}

// Data* init_stack_data(Sumset* a, Sumset* b, Wrapper* wrapper) {
//     Data* data = (Data*) malloc(sizeof(Data));
//     ASSERT_MALLOC_SUCCEEDED(data);

//     data->a = a;
//     data->b = b;
//     data->wrapper = wrapper;

//     return data;
// }

void* worker(void* args) {
    // unpack arguments
    WorkerArgs* unpacked_args = args;

    Monitor* m = unpacked_args->m;
    InputData* input_data = unpacked_args->input_data;
    Solution* best_solution = unpacked_args->best_solution;
    Stack* s = &unpacked_args->s;
    bool* done = unpacked_args->done;
    
    uint16_t loop_counter = 0;
    
    const Sumset* a;
    const Sumset* b;
    const Sumset* tmp;
    stack_init(&s);

    do {
        if (empty(&s)) {
            // TODO, ASK THE MONITOR
        }

        Node* top = pop(&s);
        a = top->a;
        b = top->b;
        
        if (a->sum > b->sum) {
            tmp = a;
            a = b;
            b = tmp;
        }
        
        if (is_sumset_intersection_trivial(a, b)) {
            
            if (loop_counter == ITERS_TO_SHARE_WORK) { // share work
                // TODO



                loop_counter = 0;
            } else { // normal business
                Wrapper* wrapper = init_wrapper(0, a, top->wrapper);
                int elems = 0;
                for (size_t i = input_data->d; i >= a->last; --i)
                if (!does_sumset_contain(b, i)) {
                    elems++;
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
        }
        else {
            // The branch is finished
            if (a->sum == b->sum && get_sumset_intersection_size(a, b) == 2 && a->sum > best_solution->sum)
                solution_build(best_solution, input_data, a, b);
            
            // Dealloc current a and wrapper
            free(a);
            dealloc_wrapper(top->wrapper);
        }

    } while (!(*done)); // Monitor will indicate that the whole work is done

    LOG("Worker DONE\n");
}