#include <stddef.h>
#include <pthread.h>

#include "common/io.h"
#include "common/sumset.h"

#define MAX_THREADS 64 

typedef struct WorkerArgs {
    InputData* input_data;
    Solution* best_solution;
} WorkerArgs;

void args_init(WorkerArgs* args, InputData* input_data, Solution* best_solution) {
    args->input_data = input_data;
    args->best_solution = best_solution;
}

void* worker(void* arg) {
    return NULL;
}

void monitor(InputData* input_data, Solution* best_solution) {

}

int main()
{
    InputData input_data;
    // input_data_read(&input_data);
    input_data_init(&input_data, 8, 10, (int[]){0}, (int[]){1, 0});

    Solution best_solution;
    solution_init(&best_solution);

    WorkerArgs args;
    args_init(&args, &input_data, &best_solution);

    pthread_t threads[MAX_THREADS];

    for (int i = 0; i < input_data.t; i++)
        pthread_create(threads + i, NULL, worker, NULL);

    monitor(&input_data, &best_solution);

    solution_print(&best_solution);
    return 0;
}
