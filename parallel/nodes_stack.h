#ifndef NODES_STACK_H
#define NODES_STACK_H

#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

#include "common/sumset.h"
#include "utils.h"
#include "wrapper.h"
#include "lock_free_stack.h"

typedef struct NodeStack {
    size_t size;
    struct Node* buffer[3000];
} NodeStack;

static bool node_empty(NodeStack* s) {
    return s->size == 0;
}

static size_t node_size(NodeStack* s) {
    return s->size;
}

static void node_push(NodeStack* s, Node* n) {
    if (!s)
        return;
    
    s->buffer[s->size] = n;
    s->size++;
}

static bool node_pop(NodeStack* s) {
    if (node_empty(s)) 
        return false;

    s->size--;
    // return s->buffer[s->size];
    free(s->buffer[s->size]);
    return true;
}

static void reset_stack(NodeStack* s) {
    while (node_pop(s)) {}
}

#endif