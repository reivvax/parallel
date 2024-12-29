#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include "common/sumset.h"

// #define NDEBUG 1

#ifndef NDEBUG
#define LOG(...)                                                                                \
    do {                                                                                        \
        fprintf(stderr, __VA_ARGS__);                                                           \
        fprintf(stderr, "\n");                                                                  \
    } while(0)
#else
#define LOG(...) ((void)0)
#endif

#ifndef NDEBUG
#define VERBOSE_LOG(...)                                                                        \
    do {                                                                                        \
        fprintf(stderr, "LOG: %s:%d: In function %s():\n\t", __FILE__, __LINE__, __func__);     \
        fprintf(stderr, __VA_ARGS__);                                                           \
        fprintf(stderr, "\n");                                                                  \
    } while(0)
#else
#define VERBOSE_LOG(...) ((void)0)
#endif

#define ASSERT_MALLOC_SUCCEEDED(expr)                                          \
    do {                                                                       \
        if ((expr) == NULL) {                                                  \
            fprintf(stderr,                                                    \
            "Memory allocation failed in %s:%d: In function %s():\n\t",        \
            __FILE__, __LINE__, __func__);                                     \
            exit(EXIT_FAILURE);                                                \
        }                                                                      \
    } while (0)

void print_sumset(const Sumset* a) {
    if (!a)
        return;
    while (a->prev) {
        fprintf(stderr, "%d ", a->last);
        a = a->prev;
    }
}

void print_sumsets(const Sumset* a, const Sumset* b) {
    print_sumset(a);
    fprintf(stderr, "\t\t");
    print_sumset(b);
    fprintf(stderr, "\n");
}

#endif