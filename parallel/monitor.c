#include <assert.h>

#include "monitor.h"

void monitor_init(Monitor* m, int d) {
    m->d = d;
    stack_init(&m->s);
    ASSERT_ZERO(pthread_mutex_init(&m->mutex, NULL));
    ASSERT_ZERO(pthread_mutex_init(&m->wrapper_mutex, NULL));
    ASSERT_ZERO(pthread_cond_init(&m->wait_for_work, NULL));
    m->waiting_for_work = 0;
    m->using_stack = 0;
    m->signalling = false;
    m->done = false;
}

void take_work(Monitor* m, Stack* dest) {
    ASSERT_ZERO(pthread_mutex_lock(&m->mutex));



    ASSERT_ZERO(pthread_mutex_unlock(&m->mutex));

}

void share_work(Monitor* m, Stack* s) {
    ASSERT_ZERO(pthread_mutex_lock(&m->mutex));
    // Here lies the logic of sharing work between threads
    // For now, take the number of currently waiting threads 'waiting_for_work'
    // And leave on the stack (s->size / waiting_for_work) elements if waiting_for_work > 0 else assume waiting_for_work = 1
    // Distribute the rest over waiting workers
    // Other possible approaches:
    // - Always give away half of the stack
    // - Always give away constant number of nodes 
    
    int leave = m->waiting_for_work == 0 ? 
                (s->size + 1) / 2 : (s->size + m->waiting_for_work - 1) / m->waiting_for_work; // ceiling
    int count = s->size - leave;
    Node* last = get_nth(s, count);
    assert(last != NULL);

    Node* top = detach(s, last, count);

    push_n(s, top, last, count);

    // MORE LOGIC
    ASSERT_ZER(pthread_cond_broadcast(&m->wait_for_work));

    ASSERT_ZERO(pthread_mutex_unlock(&m->mutex));
}