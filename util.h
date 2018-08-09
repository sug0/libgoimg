#ifndef GOIMG_UTIL_H
#define GOIMG_UTIL_H

#include <stdbool.h>
#include <goio.h>

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

#define _err_read(RF, SRC, BUF, SIZE) \
    unlikely(__err_read((RF), (SRC), (BUF), (SIZE)))

#define _err_write(WF, DST, BUF, SIZE) \
    unlikely(__err_write((WF), (DST), (BUF), (SIZE)))

extern bool _m_match(char *magic, int msize, char *b, int bsize);
extern bool __err_read(rfun_t rf, void *src, char *buf, int size);
extern bool __err_write(wfun_t wf, void *dst, char *buf, int size);

#endif
