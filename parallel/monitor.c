#include "monitor.h"

void monitor_init(Monitor* m, int d) {
    m->d = d;
    stack_init(&m->s);
    ASSERT_ZERO(pthread_mutex_init(&m->mutex, NULL));
    ASSERT_ZERO(pthread_cond_init(&m->wait_for_nodes, NULL));
    m->waiting_for_nodes = 0;
    m->using_stack = 0;
    m->signalling = false;
}

void take_args(Monitor* m, Stack* dest) {
    
}