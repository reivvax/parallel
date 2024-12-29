#ifndef WORKER_H
#define WORKER_H

#include "stack.h"
#include "monitor.h"

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

void* worker(void* args) {
    // unpack arguments
    WorkerArgs* unpacked_args = args;

    Monitor* m = unpacked_args->m;
    pthread_mutex_t* wrapper_mutex = &m->wrapper_mutex;
    InputData* input_data = unpacked_args->input_data;
    Solution* best_solution = unpacked_args->best_solution;
    Stack* s = &unpacked_args->s;
    bool* done = unpacked_args->done;
    
    uint32_t loop_counter = 0;

    const Sumset* a;
    Wrapper* w_a;

    const Sumset* b;
    Wrapper* w_b;

    const Sumset* tmp;

    do {
        LOG("Iteration: %d", loop_counter);
        if (empty(s)) {
            // TODO, ASK THE MONITOR
            LOG("FINISHED");
            return 0;
            loop_counter = 0; // IS IT NECCESARRY? (YES (MAYBE NOT))
        }
        loop_counter++;
        // if (loop_counter++ >= ITERS_TO_SHARE_WORK) { // Share work
        //     share_work(m, s);
        //     loop_counter = 0;
        //     continue;
        // }

        Node* top = pop(s);
        w_a = top->a;
        w_b = top->b;
        a = &w_a->set;
        b = &w_b->set;
        
        if (a->sum > b->sum) {
            tmp = a;
            a = b;
            b = tmp;
        }

        // fprintf(stderr, "Address of a: %p\n", a);
        // fprintf(stderr, "Address of b: %p\n", b);
        print_sumsets(a, b);

        if (is_sumset_intersection_trivial(a, b)) {
            int elems = 0;
            for (size_t i = input_data->d; i >= a->last; --i)
                if (!does_sumset_contain(b, i)) {
                    elems++;

                    Wrapper* new_wrapper = init_wrapper(1, w_a);
                    ASSERT_MALLOC_SUCCEEDED(new_wrapper);

                    sumset_add(&new_wrapper->set, a, i);
                    Data data = (Data) {.a = new_wrapper, .b = w_b};
                    push(&s, &data);
                }

            if (elems == 0) {
                // DOES THIS BRANCH EVEN EXECUTE ANYTIME? CHECK IT, FOR NOW LEAVE IT
                printf("YES THIS BRANCH EXECUTES\n");
                decrement_ref_counter(w_a);
                decrement_ref_counter(w_b);
                try_dealloc_wrapper(w_a, wrapper_mutex);
                try_dealloc_wrapper(w_b, wrapper_mutex);
            } else {
                increment_ref_counter_n(w_a, elems - 1); // -1, as we popped from the stack
                increment_ref_counter_n(w_b, elems - 1);
            }
        }
        else {
            // The branch is finished
            if (a->sum == b->sum && get_sumset_intersection_size(a, b) == 2 && a->sum > best_solution->sum)
                solution_build(best_solution, input_data, a, b);

            // Dealloc current a and wrapper
            try_dealloc_wrapper_with_decrement(w_a, wrapper_mutex); // decrement, as we popped from the stack
            try_dealloc_wrapper_with_decrement(w_b, wrapper_mutex);
        }

        free(top);
    } while (!(*done)); // Monitor will indicate that the whole work is done

    LOG("Worker DONE\n");
    return 0;
}

#endif