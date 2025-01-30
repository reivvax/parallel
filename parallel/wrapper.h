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

static Wrapper* init_wrapper(_Atomic uint64_t ref_counter, Wrapper* prev) {
    Wrapper* w = (Wrapper*) malloc(sizeof(Wrapper));
    ASSERT_MALLOC_SUCCEEDED(w);

    w->ref_counter = ref_counter;
    w->prev = prev;
    return w;
}

static void increment_ref_counter_n(Wrapper* w, uint64_t n) {
    atomic_fetch_add(&w->ref_counter, n);
}

/**
 * @brief Decrements `ref_counter` and deallocs wrapper if `ref_counter` is 0
 */
static void try_dealloc_wrapper_with_decrement(Wrapper* w) {
    if (!w)
        return;
    
    if (atomic_fetch_sub_explicit(&w->ref_counter, 1, memory_order_seq_cst) == 1) {
        Wrapper* prev = w->prev;
        free(w);
        try_dealloc_wrapper_with_decrement(prev);
    }
}

#endif