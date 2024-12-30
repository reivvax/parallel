#ifndef WRAPPER_H
#define WRAPPER_H

#include <stdint.h>
#include <stdatomic.h>

#include "common/sumset.h"
#include "common/err.h"
#include "utils.h"

typedef struct Wrapper {
    _Atomic uint64_t ref_counter;
    Sumset set;
    struct Wrapper* prev;
} Wrapper;

Wrapper* init_wrapper(_Atomic uint64_t ref_counter, Wrapper* prev) {
    Wrapper* w = (Wrapper*) malloc(sizeof(Wrapper));
    ASSERT_MALLOC_SUCCEEDED(w);

    w->ref_counter = ref_counter;
    w->prev = prev;
    return w;
}

void increment_ref_counter_n(Wrapper* w, uint64_t n) {
    atomic_fetch_add(&w->ref_counter, n);
}

void increment_ref_counter(Wrapper* w) {
    increment_ref_counter_n(w, 1);
}

void decrement_ref_counter(Wrapper* w) {
    atomic_fetch_sub(&w->ref_counter, 1);
}

/**
 * @brief Deallocs wrapper if `ref_counter` is 0
 */
// void try_dealloc_wrapper(Wrapper* w, pthread_mutex_t* mutex) {
//     if (!w)
//         return;
    
//     if (w->ref_counter <= 0) { // IDEALLY == 0
//         ASSERT_ZERO(pthread_mutex_lock(mutex));
//         // Critical section
//         Wrapper* prev = w->prev;
//         free(w);
//         ASSERT_ZERO(pthread_mutex_unlock(mutex));
//         try_dealloc_wrapper_with_decrement(prev);
//     }
// }

/**
 * @brief Decrements `ref_counter` and deallocs wrapper if `ref_counter` is 0
 */
void try_dealloc_wrapper_with_decrement(Wrapper* w) {
    if (!w)
        return;
    
    if (atomic_fetch_sub_explicit(&w->ref_counter, 1, memory_order_acq_rel) == 1) {
        Wrapper* prev = w->prev;
        free(w);
        try_dealloc_wrapper_with_decrement(prev);
    }
}

#endif