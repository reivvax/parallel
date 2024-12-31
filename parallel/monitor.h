#ifndef MONITOR_H
#define MONITOR_H

#include <pthread.h>

#include "common/err.h"
#include "stack.h"
#include "utils.h"

typedef struct Monitor {
    int t;                          // t parameter from input_data
    int d;                          // d parameter from input_data
    Stack s;                        // collective stack
    _Atomic bool empty;             // indicator whether collective stack is empty
    pthread_mutex_t mutex;
    pthread_cond_t give_work;       // who is giving work
    pthread_cond_t wait_for_work;   // who is waiting
    uint8_t waiting_to_give_work;   // how much of them
    uint8_t waiting_for_work;       // how much of them
    int work_amount;                // how much work can be taken from collective stack at the moment
    bool signalling;                // is the monitor signalling condition
    bool done;                      // indicator whether the whole computation is done
} Monitor;

void monitor_init(Monitor* m, int t, int d);

void take_work(Monitor* m, Stack* dst, int id);

void share_work(Monitor* m, Stack* src, int id);

#endif