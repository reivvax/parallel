#ifndef STACK_H
#define STACK_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

#include "common/sumset.h"
#include "utils.h"
#include "wrapper.h"

typedef struct Data {
    const Sumset* a;
    const Sumset* b;
    Wrapper* wrapper;
} Data;

typedef struct Node {
    const Sumset* a;
    const Sumset* b;
    Wrapper* wrapper;
    struct Node* prev;
} Node;

typedef struct Stack {
    struct Node* head;
    size_t size;
} Stack;

Node* init_node(Data* data, Node* prev) {
    Node* n = (Node*) malloc(sizeof(Node));
    ASSERT_MALLOC_SUCCEEDED(n);

    n->a = data->a;
    n->b = data->b;
    n->wrapper = data->wrapper;
    n->prev = prev;

    return n;
}

bool empty(Stack* s) {
    return s->head == NULL;
}

size_t size(Stack* s) {
    return s->size;
}

Node* top(Stack* s) {
    if (!s)
        return NULL;
    return s->head;
}

void push(Stack* s, Data* data) {
    if (!s)
        return;
    Node* n = init_node(data, s->head);
    s->size++;
    s->head = n;
}

// Remember to free the node
Node* pop(Stack* s) {
    if (empty(s)) 
        return NULL;

    Node* res = s->head;
    s->head = s->head->prev;
    s->size--;
    return res;
}

/**
 * @brief Pushes all the nodes from `top` to `last` on `s` in O(1) time. `top` becomes `s->head`.
 */
void push_n(Stack* s, Node* top, Node* last, int n) {
    last->prev = s->head;
    s->head = top;
    s->size += n;
}

// Returns the n-th node from the top. If n = 1, returns head. If n > s->size, returns NULL
Node* get_nth(Stack* s, int n) {
    if (!s || n < 1 || n > s->size)
        return NULL;

    Node* res = s->head;
    while (--n)
        res = res->prev;

    return res;
}

/**
 * @brief Splits the stack where `n` is placed. `n` must be inside the provided stack, 
 * otherwise, the stack is being replaced
 *  
 * @return Top of the provided stack
 */ 
Node* detach(Stack* s, Node* n, int size_of_detached) {
    if (!s)
        return NULL;

    Node* res = top(s);
    
    s->head = n->prev;
    s->size -= size_of_detached;

    return res;
}

/**
 * @brief Splits the stack so that detached stack is of size `n`. 
 * If `n > s->size` then the resulting stack becomes `s` and `s` becomes empty, 
 *  
 * @return Top of the provided stack
 */ 
Node* detach_n(Stack* s, int n) {
    if (!s)
        return NULL;

    Node* res = top(s);
    Node* cut = get_nth(s, n);

    if (n >= s->size) {
        s->size = 0;
        s->head = NULL;
    } else {
        s->size -= n;
        s->head = cut->prev;
    }

    return res;
}

void stack_dealloc(Stack* s) {
    while (!empty(s)) 
        pop(s);
}

void stack_init(Stack* s) {
    s->head = NULL;
    s->size = 0;
}

#endif