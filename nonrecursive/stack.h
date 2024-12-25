#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "common/sumset.h"
#include "utils.h"

typedef struct Node {
    Sumset* set;
    bool removal;
    struct Node* prev;
} Node;

typedef struct Stack {
    struct Node* head;
} Stack;

Node* init_node(Sumset* set, bool removal, Node* prev) {
    Node* n = (Node*) malloc(sizeof(Node));
    ASSERT_MALLOC_SUCCEEDED(n);

    n->set = set;
    n->removal = removal;
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

void push(Stack* s, Sumset* set, bool removal) {
    if (!s || !set)
        return;
    Node* n = init_node(set, removal, s->head);

    s->head = n;
}

// Pop with no return values, probaly leaks memory
void pop(Stack* s) {
    if (empty(s)) 
        return;
    
    Node* tmp = s->head;
    s->head = s->head->prev;
    free(tmp);
}

// Result value in res
void pop_ret(Stack* s, Sumset** res, bool* removal) {
    if (empty(s)) 
        return;

    *res = s->head->set;
    *removal = s->head->removal;
    pop(s);
}

void stack_dealloc(Stack* s) {
    while (!empty(s)) 
        pop(s);
}

void stack_init(Stack* s) {
    s->head = NULL;
}