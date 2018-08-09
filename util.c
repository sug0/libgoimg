#include "util.h"

/* match a magic number */
bool _m_match(char *magic, int msize, char *b, int bsize)
{
    while (msize-- > 0) {
        if (*magic != *b && *magic != '?')
            return false;
        magic++;
        b++;
    }
    return true;
}

inline bool __err_write(wfun_t wf, void *dst, char *buf, int size)
{
    return wf(dst, buf, size) < size;
}

inline bool __err_read(rfun_t rf, void *src, char *buf, int size)
{
    return rf(src, buf, size) < size;
}
