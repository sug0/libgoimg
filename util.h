#ifndef GOIMG_UTIL_H
#define GOIMG_UTIL_H

#include <stdbool.h>

#ifdef __GNUC__
    #define unlikely(X)  __builtin_expect((X), 0)
#else
    #define unlikely(X)  (X)
#endif

#ifdef __GNUC__
    #define likely(X)  __builtin_expect((X), 1)
#else
    #define likely(X)  (X)
#endif

extern bool _m_match(char *magic, int msize, char *b, int bsize);
extern void *_xalloc(void *(*malloc)(size_t), size_t size);
extern void *_xrealloc(void *(*realloc)(void *, size_t), void *m, size_t size);

#endif
