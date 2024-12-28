#include <pthread.h>

#include "common/err.h"
#include "stack.h"
#include "utils.h"

typedef struct Monitor {
    int d; // d parameter from input_data
    Stack s;
    pthread_mutex_t mutex;
    pthread_cond_t wait_for_nodes;
    int waiting_for_nodes;
    int using_stack;
    bool signalling;
} Monitor;

void monitor_init(Monitor* m, int d);

void take_args(Monitor* m, Stack* dest);

// a, b - current sumsets that are being considered, 
// args_count - number of new pair of sumsets that will be pushed on the collective stack
// addable - bool array that indicates if a parameter is addable
void give_args(Monitor* m, Sumset* a, Sumset* b, int args_count, bool* addable);