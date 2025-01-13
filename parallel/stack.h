#ifndef STACK_H
#define STACK_H

#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

#include "common/sumset.h"
#include "utils.h"
#include "wrapper.h"

typedef struct Node {
    Wrapper* a;
    Wrapper* b;
} Node;

typedef struct Stack {
    size_t size;
    struct Node buffer[4000];
} Stack;

static bool empty(Stack* s) {
    return s->size == 0;
}

static size_t size(Stack* s) {
    return s->size;
}

static void push(Stack* s, Wrapper* a, Wrapper* b) {
    if (!s)
        return;
    
    s->buffer[s->size].a = a;
    s->buffer[s->size].b = b;
    s->size++;
}

static void pop(Stack* s, Wrapper** a, Wrapper** b) {
    if (empty(s)) 
        return;

    s->size--;
    *a = s->buffer[s->size].a;
    *b = s->buffer[s->size].b;
}

static void rearrange_n(Stack* src, Stack* dst, int n) {
    Wrapper* a;
    Wrapper* b;

    while(!empty(src) && n--) {
        pop(src, &a, &b);
        push(dst, a, b);
    }
}

#endif