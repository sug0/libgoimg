#ifndef GOIMG_UTIL_H
#define GOIMG_UTIL_H

#include <stdbool.h>

#if defined(_WIN32) || defined(_WIN64)
    #include <winsock.h>
#elif defined(__CYGWIN__) && !defined(_WIN32)
    #include <arpa/inet.h>
#else
    #include <arpa/inet.h>
#endif

#ifdef __GNUC__
    #define unlikely(X)  __builtin_expect(!!(X), 0)
    #define likely(X)    __builtin_expect(!!(X), 1)
#else
    #define unlikely(X)  (X)
    #define likely(X)    (X)
#endif

extern bool _m_match(char *magic, int msize, char *b, int bsize);
extern void *_xalloc(void *(*malloc)(size_t), size_t size);
extern void *_xcalloc(void *(*calloc)(size_t, size_t), size_t nmemb, size_t size);
extern void *_xrealloc(void *(*realloc)(void *, size_t), void *m, size_t size);

#endif
