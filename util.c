#include <stdio.h>
#include <stdlib.h>

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

void *_xalloc(void *(*malloc)(size_t), size_t size)
{
    void *m = malloc(size);

    if (!m) {
        fprintf(stderr, "failed to allocate memory...\n");
        exit(1);
    }

    return m;
}

void *_xrealloc(void *(*realloc)(void *, size_t), void *m, size_t size)
{
    m = realloc(m, size);

    if (!m) {
        fprintf(stderr, "failed to allocate memory...\n");
        exit(1);
    }

    return m;
}
