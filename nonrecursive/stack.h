#ifndef STACK_H
#define STACK_H

#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

#include "common/sumset.h"
#include "utils.h"
#include "wrapper.h"

typedef struct Node {
    WrapperNode* a;
    WrapperNode* b;
} Node;

typedef struct Stack {
    size_t size;
    struct Node buffer[2000];
} Stack;

static bool empty(Stack* s) {
    return s->size == 0;
}

static size_t size(Stack* s) {
    return s->size;
}

static void push(Stack* s, WrapperNode* a, WrapperNode* b) {
    if (!s)
        return;
    
    s->buffer[s->size].a = a;
    s->buffer[s->size].b = b;
    s->size++;
}

// Remember to free the node
static void pop(Stack* s, WrapperNode** a, WrapperNode** b) {
    if (empty(s)) 
        return;

    s->size--;
    *a = s->buffer[s->size].a;
    *b = s->buffer[s->size].b;
}

#endif