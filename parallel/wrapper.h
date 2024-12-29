#include <stdint.h>

#include "common/sumset.h"

typedef struct Wrapper {
    uint8_t counter;
    Sumset* to_dealloc;   // pointer to the set that will be dealloacted as soon as counter hits 0
    Wrapper* prev;
} Wrapper;

Wrapper* init_wrapper(uint8_t counter, Sumset* sumset, Wrapper* prev) {
    Wrapper* w = (Wrapper*) malloc(sizeof(Wrapper));
    ASSERT_MALLOC_SUCCEEDED(w);
    
    w->counter = counter;
    w->to_dealloc = sumset;
    w->prev = prev;
    return w;
}

bool dealloc_wrapper(Wrapper* w) {
    w->counter--;
    if (w->counter == 0) {
        free(w->to_dealloc);
        free(w);
        dealloc_wrapper(w->prev);
    }
}

void set_counter(Wrapper* w, uint8_t counter) {
    w->counter = counter;
}