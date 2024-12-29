#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

#include "common/sumset.h"
#include "utils.h"

typedef struct Data {
    Sumset* a;
    Sumset* b;
    Wrapper* wrapper;
} Data;

typedef struct Node {
    Sumset* a;
    Sumset* b;
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

    Node* RES = s->head;
    s->head = s->head->prev;
    s->size--;
    return RES;
}

void stack_dealloc(Stack* s) {
    while (!empty(s)) 
        pop(s);
}

void stack_init(Stack* s) {
    s->head = NULL;
    s->size = 0;
}