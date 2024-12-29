#ifndef WRAPPER_H
#define WRAPPER_H

#include <stdint.h>
// #include <pthread.h>


#include "common/sumset.h"
#include "utils.h"

typedef struct Wrapper {
    _Atomic uint64_t ref_counter;
    Sumset set;    // pointer to the set that will be dealloacted as soon as ref_counter hits 0
    struct Wrapper* prev;
} Wrapper;

Wrapper* init_wrapper(_Atomic uint64_t ref_counter, Wrapper* prev) {
    Wrapper* w = (Wrapper*) malloc(sizeof(Wrapper));
    ASSERT_MALLOC_SUCCEEDED(w);

    w->ref_counter = ref_counter;
    w->prev = prev;
    return w;
}

void increment_ref_counter(Wrapper* w) {
    w->ref_counter++;
}

void increment_ref_counter_n(Wrapper* w, uint64_t n) {
    w->ref_counter += n;
}

void decrement_ref_counter(Wrapper* w) {
    w->ref_counter--;
}

// Deallocs wrapper if ref_counter is (BELOW?) 0
void try_dealloc_wrapper(Wrapper** w_ptr, pthread_mutex_t* mutex) {
    if (!w_ptr || !(*w_ptr))
        return;
    
    Wrapper* w = *w_ptr;
    if (w->ref_counter <= 0) { // IDEALLY == 0
        ASSERT_ZERO(pthread_mutex_lock(mutex));
        // Critical section
        Wrapper* prev = w->prev;
        free(*w_ptr);
        *w_ptr = NULL;          // So that no one frees it again
        ASSERT_ZERO(pthread_mutex_unlock(mutex));
        try_dealloc_wrapper(prev, mutex);
    }
}

// Deallocs wrapper if ref_counter is (BELOW?) 0
void try_dealloc_wrapper_with_decrement(Wrapper** w_ptr, pthread_mutex_t* mutex) {
    decrement_ref_counter(*w_ptr);
    try_dealloc_wrapper(w_ptr, mutex);
}

void set_ref_counter(Wrapper* w, uint64_t ref_counter) {
    if (w)
        w->ref_counter = ref_counter;
}

#endif