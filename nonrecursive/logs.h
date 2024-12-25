#ifndef NDEBUG
#define LOG(...)                                                                                \
    do {                                                                                        \
        fprintf(stderr, "LOG: %s:%d: In function %s():\n\t", __FILE__, __LINE__, __func__);     \
        fprintf(stderr, __VA_ARGS__);                                                           \
        fprintf(stderr, "\n");                                                                  \
    } while(0)
#else
#define LOG(expr) ((void)0)
#endif
