#ifndef SUMSET_STACK_H
#define SUMSET_STACK_H

#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

#include "common/sumset.h"
#include "utils.h"
#include "wrapper.h"

typedef struct SumsetStack {
    size_t size;
    struct WrapperNode buffer[100];
} SumsetStack;

static bool s_empty(SumsetStack* s) {
    return s->size == 0;
}

static size_t s_size(SumsetStack* s) {
    return s->size;
}

static WrapperNode* s_push(SumsetStack* s, WrapperNode* prev) {
    if (!s)
        return NULL;
    
    WrapperNode* res = &s->buffer[s->size];
    s->buffer[s->size].ref_counter = 1;
    s->buffer[s->size].prev = prev;
    
    s->size++;
    return res;
}

static WrapperNode* s_top(SumsetStack* s) {
    return &s->buffer[s->size - 1];
}

static void increment_ref_counter(WrapperNode* node, int n) {
    node->ref_counter += n;
}

static void decrement_ref_counter(WrapperNode* node) {
    node->ref_counter--;
}

static void try_dealloc_top(SumsetStack* s) {
    if (s_empty(s))
        return;
    WrapperNode* top = s_top(s);

    // top->ref_counter--;
    if (top->ref_counter <= 0) {
        top->prev->ref_counter--;
        s->size--;
    }

    while (!s_empty(s) && s_top(s)->ref_counter == 0)
        s->size--;
}

#endif