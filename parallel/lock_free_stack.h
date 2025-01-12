#ifndef LOCK_FREE_STACK_H
#define LOCK_FREE_STACK_H 

#include <stdlib.h>
#include <stdatomic.h>
#include <stdbool.h>

#include "wrapper.h"
#include "utils.h"

typedef struct Node {
    Wrapper* a;
    Wrapper* b;
    struct Node* next;
} Node;

typedef struct LockFreeStack {
    int tag; // https://en.wikipedia.org/wiki/ABA_problem
    Node* head;
} LockFreeStack;

Node* init_node(Wrapper* a, Wrapper* b) {
    Node* new_node = (Node*) malloc(sizeof(Node));
    ASSERT_MALLOC_SUCCEEDED(new_node);
    
    new_node->a = a;
    new_node->b = b;
    return new_node;
}

void push(_Atomic LockFreeStack* stack, Node* new_node) {
    LockFreeStack next, prev;
    
    prev = atomic_load(stack);
    do {
        new_node->next = prev.head;
        next.head = new_node;
        next.tag = prev.tag + 1;
    } while (!atomic_compare_exchange_weak(stack, &prev, next));
}

Node* pop(_Atomic LockFreeStack* stack) {
    LockFreeStack next, prev;
    
    prev = atomic_load(stack);
    do {
        if (prev.head == NULL)
            return NULL;

        next.head = prev.head->next;
        next.tag = prev.tag + 1;
    } while(!atomic_compare_exchange_weak(stack, &prev, next));

    return prev.head;
}

#endif