#ifndef WRAPPER_H
#define WRAPPER_H

#include <stdint.h>
#include <stdatomic.h>

#include "common/sumset.h"
#include "common/err.h"

typedef struct WrapperNode {
    uint64_t ref_counter;
    Sumset s;
    struct WrapperNode* prev;
} WrapperNode;

// typedef struct Wrapper {
//     uint64_t ref_counter;
//     Sumset set;
//     struct Wrapper* prev_set;
// } Wrapper;

// static Wrapper* init_wrapper(_Atomic uint64_t ref_counter, Wrapper* prev_set) {
//     Wrapper* w = (Wrapper*) malloc(sizeof(Wrapper));
//     ASSERT_MALLOC_SUCCEEDED(w);

//     w->ref_counter = ref_counter;
//     w->prev_set = prev_set;
//     return w;
// }

// static void increment_ref_counter_n(Wrapper* w, uint64_t n) {
//     w->ref_counter += n;
// }

// /**
//  * @brief Decrements `ref_counter` and deallocs wrapper if `ref_counter` is 0
//  */
// static void try_dealloc_wrapper_with_decrement(Wrapper* w, size_t* count) {
//     if (!w)
//         return;
//     w->ref_counter--;
//     if (w->ref_counter == 0) {
//         (*count)--;
//         Wrapper* prev_set = w->prev_set;
//         free(w);
//         try_dealloc_wrapper_with_decrement(prev_set, count);
//     }
// }

#endif