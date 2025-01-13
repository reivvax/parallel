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

// typedef struct Node {
//     Wrapper* a;
//     Wrapper* b;
//     struct Node* prev;
// } Node;

// typedef struct Stack {
//     struct Node* head;
//     size_t size;
// } Stack;

// static Node* init_node(Wrapper* a, Wrapper* b, Node* prev) {
//     Node* n = (Node*) malloc(sizeof(Node));
//     ASSERT_MALLOC_SUCCEEDED(n);

//     n->a = a;
//     n->b = b;
//     n->prev = prev;

//     return n;
// }

// static bool empty(Stack* s) {
//     return s->head == NULL;
// }

// static size_t size(Stack* s) {
//     return s->size;
// }

// static Node* top(Stack* s) {
//     if (!s)
//         return NULL;
//     return s->head;
// }

// static void swap_stacks(Stack* a, Stack* b) {
//     Node* tmp_n = a->head;
//     a->head = b->head;
//     b->head = tmp_n;

//     size_t tmp_s = a->size;
//     a->size = b->size;
//     b->size = tmp_s;
// }

// static void push(Stack* s, Node* n) {
//     if (!s)
//         return;
//     n->prev = s->head;
//     s->head = n;
//     s->size++;
// }

// // Remember to free the node
// static Node* pop(Stack* s) {
//     if (empty(s)) 
//         return NULL;

//     Node* res = s->head;
//     s->head = s->head->prev;
//     s->size--;
//     return res;
// }

// /**
//  * @brief Pushes all the nodes from `top` to `last` on `s` in O(1) time. `top` becomes `s->head`.
//  */
// static void push_n(Stack* s, Node* top, Node* last, int n) {
//     last->prev = s->head;
//     s->head = top;
//     s->size += n;
// }

// // Returns the n-th node from the top in O(n). If n = 1, returns head. If n > s->size, returns last element.
// static Node* get_nth(Stack* s, int n) {
//     if (!s || empty(s) || n < 1)
//         return NULL;

//     Node* res = s->head;
//     while (--n && res->prev)
//         res = res->prev;

//     return res;
// }

// /**
//  * @brief Splits the stack where `n` is placed in O(1). `n` must be inside the provided stack, 
//  * otherwise, the stack is being replaced.
//  *  
//  * @return Top of the provided stack
//  */ 
// static Node* detach(Stack* s, Node* n, int size_of_detached) {
//     if (!s)
//         return NULL;

//     Node* res = top(s);

//     s->head = n ? n->prev : NULL;
//     s->size -= size_of_detached;

//     return res;
// }

// /**
//  * @brief Takes `n` elements from `src` and pushes them to `dst` in O(n), the order of nodes is preserved. 
//  */
// static void rearrange_n(Stack* src, Stack* dst, int n) {
//     if (n == 0)
//         return;
//     if (src->size < n && empty(dst))
//         swap_stacks(src, dst);
//     else {
//         if (src->size < n)
//             n = src->size;
//         Node* last = get_nth(src, n);
//         Node* top = detach(src, last, n);
//         push_n(dst, top, last, n);
//     }
// }

// static void stack_dealloc(Stack* s) {
//     while (!empty(s)) 
//         pop(s);
// }

// static void stack_init(Stack* s) {
//     s->head = NULL;
//     s->size = 0;
// }

#endif