#ifndef STACK_H
#define STACK_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

#include "common/sumset.h"
#include "utils.h"

typedef enum NodeType {
    SUMSET,
    REMOVAL,
    SWAP
} NodeType;

typedef struct Node {
    const Sumset* set;
    NodeType type;
    struct Node* prev;
} Node;

typedef struct Stack {
    struct Node* head;
    size_t size;
} Stack;

Node* init_node(Sumset* set, NodeType type, Node* prev) {
    Node* n = (Node*) malloc(sizeof(Node));
    ASSERT_MALLOC_SUCCEEDED(n);

    n->set = set;
    n->type = type;
    n->prev = prev;

    return n;
}

bool empty(Stack* s) {
    return s->head == NULL;
}

Node* top(Stack* s) {
    if (!s)
        return NULL;
    return s->head;
}

void push(Stack* s, Sumset* set, NodeType type) {
    if (!s)
        return;
    Node* n = init_node(set, type, s->head);
    s->size++;
    s->head = n;
}

// Pop with no return values, probaly leaks memory
void pop(Stack* s) {
    if (empty(s)) 
        return;
    
    Node* tmp = s->head;
    s->head = s->head->prev;
    s->size--;
    free(tmp);
}

// Result value in res
void pop_ret(Stack* s, const Sumset** res, NodeType* type) {
    if (empty(s)) 
        return;
    if (s->head->set)
        *res = s->head->set;
    if (s->head->type == REMOVAL)
        assert(*res == s->head->set);
    
    *type = s->head->type;
    pop(s);
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