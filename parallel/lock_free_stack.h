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

void push(_Atomic LockFreeStack* stack, Wrapper* a, Wrapper* b) {
    LockFreeStack next, prev;

    Node* new_node = (Node*) malloc(sizeof(Node));
    ASSERT_MALLOC_SUCCEEDED(new_node);
    
    new_node->a = a;
    new_node->b = b;
    
    prev = atomic_load(stack);
    do {
        new_node->next = prev.head;
        next.head = new_node;
        next.tag = prev.tag + 1;
    } while (!atomic_compare_exchange_weak(stack, &prev, next));
}

bool pop(_Atomic LockFreeStack* stack, Wrapper** a, Wrapper** b) {
    LockFreeStack next, prev;
    
    prev = atomic_load(stack);
    do {
        if (prev.head == NULL) {
            return false;
        }
        next.head = prev.head->next;
        next.tag = prev.tag + 1;
    } while(!atomic_compare_exchange_weak(stack, &prev, next));

    *a = prev.head->a;
    *b = prev.head->b;
    free(prev.head);
    return true;
}