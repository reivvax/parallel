#ifndef WRAPPER_H
#define WRAPPER_H

#include <stdint.h>

#include "common/sumset.h"

typedef struct Wrapper {
    uint8_t counter;
    const Sumset* to_dealloc;   // pointer to the set that will be dealloacted as soon as counter hits 0
    struct Wrapper* prev;
} Wrapper;

Wrapper* init_wrapper(uint8_t counter, const Sumset* sumset, Wrapper* prev) {
    Wrapper* w = (Wrapper*) malloc(sizeof(Wrapper));
    ASSERT_MALLOC_SUCCEEDED(w);
    
    w->counter = counter;
    w->to_dealloc = sumset;
    w->prev = prev;
    return w;
}

// Returns true, if wrapper was actually deallocated
bool try_dealloc_wrapper(Wrapper* w) {
    if (!w)
        return false;

    w->counter--; // MUTEX IT OUT
    if (w->counter == 0) {
        free((Sumset*) w->to_dealloc); // discard const qualifier
        Wrapper* prev = w->prev;
        free(w);
        try_dealloc_wrapper(prev);
        return true;
    }
    return false;
}

void set_counter(Wrapper* w, uint8_t counter) {
    if (w)
        w->counter = counter;
}

#endif