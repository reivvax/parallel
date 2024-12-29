#ifndef MONITOR_H
#define MONITOR_H

#include <pthread.h>

#include "common/err.h"
#include "stack.h"
#include "wrapper.h"
#include "utils.h"

typedef struct Monitor {
    int d;                          // d parameter from input_data
    Stack s;                        // collective stack
    pthread_mutex_t mutex;
    pthread_mutex_t wrapper_mutex;
    pthread_cond_t wait_for_work;   // who is waiting
    uint8_t waiting_for_work;       // how much of them
    uint8_t using_stack;            // probably unnecessary
    bool signalling;
    bool done;                      // indicator whether the whole computation is done
} Monitor;

void monitor_init(Monitor* m, int d);

void take_work(Monitor* m, Stack* dest);

void share_work(Monitor* m, Stack* src);

#endif