#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include "goio.h"

#define RWCPY_BUFSIZ  4096

int *goio_fd(int fd)
{
    static int fdstk[GOIO_FD_NUM];
    static int n = 1;

    int i, *ptr;

    for (i = 0; i < n; i++)
        if (fdstk[i] == fd)
            return &fdstk[i];

    ptr = &fdstk[n++];
    *ptr = fd;

    return ptr;
}

int rwcpy(wfun_t wf, void *dst, rfun_t rf, void *src)
{
    static char buf[RWCPY_BUFSIZ];
    return rwcpy_r(wf, dst, rf, src, buf, RWCPY_BUFSIZ);
}

int rwcpy_r(wfun_t wf, void *dst, rfun_t rf, void *src, char *buf, int size)
{
    int nr, nw, written = 0;

    for (;;) {
        /* read from source */
        nr = rf(src, buf, size);
        if (nr < 0)
            return -written;
        if (nr == 0)
            return written;

        /* write to destiny */
        nw = wf(dst, buf, nr);
        if (nw < 0)
            return -written;
        if (nw == 0)
            return written;

        /* short write */
        if (nw != nr)
            return -written;

        written += nw;
    }
}

static int _rbuffill(BufferedReader_t *br)
{
#define GOIO_ITER_MAX  100
    if (br->w >= br->bufsz)
        return -1;

    if (br->r > 0) {
        memcpy(br->buf, br->buf + br->r, br->w);
        br->w -= br->r;
        br->r = 0;
    }

    int i, n;

    for (i = GOIO_ITER_MAX; i > 0; i--) {
        n = br->rf(br->src, br->buf + br->w, br->bufsz - br->w);

        if (n < 0)
            return -1;

        br->w += n;

        if (n != 0)
            break;
    }

    return (i == GOIO_ITER_MAX) ? 0 : -1;
#undef GOIO_ITER_MAX
}

int rbufpeek(void *src, char *buf, int npeek)
{
    BufferedReader_t *br = (BufferedReader_t *)src;
    int avail, n = 0;

    if (npeek > br->bufsz)
        return -1;

    for (;;) {
        avail = br->w - br->r;
        if (avail < npeek && avail < br->bufsz && n == 0)
            n = _rbuffill(br);
        else
            break;
    }

    avail = br->w - br->r;
    n = (avail < npeek) ? avail : npeek;

    memcpy(buf, br->buf + br->r, n);

    return n;
}

int rbufread(void *src, char *buf, int size)
{
    BufferedReader_t *br = (BufferedReader_t *)src;
    int n;

    if (size == 0)
        return 0;

    if (br->r == br->w) {
        if (size >= br->bufsz)
            return br->rf(br->src, buf, size);

        br->r = br->w = 0;
        n = br->rf(br->src, br->buf, br->bufsz);

        if (n < 0)
            return -1;
        if (n == 0)
            return 0;

        br->w += n;
    }

    int sz = br->w - br->r;
    n = (size < sz) ? size : sz;

    memcpy(buf, br->buf + br->r, n);
    br->r += n;

    return n;
}

int multiread(void *src, char *buf, int size)
{
    MultiReader_t **mr = (MultiReader_t **)src;
    int n;

    while ((*mr)->rf) {
        n = (*mr)->rf((*mr)->src, buf, size);

        if (n < 0)
            return -1;
        if (n > 0)
            return n;

        (*mr)++;
    }

    return 0;
}

int multiwrite(void *dst, char *buf, int size)
{
    MultiWriter_t *mw;
    int n;

    for (mw = (MultiWriter_t *)dst; mw->wf; mw++) {
        n = mw->wf(mw->dst, buf, size);
        if (n != size)
            return -n;
    }

    return size;
}

int teeread(void *src, char *buf, int size)
{
    TeeReader_t *t = (TeeReader_t *)src;
    int n;

    n = t->rf(t->src, buf, size);

    if (n <= 0)
        return n;

    return t->wf(t->dst, buf, n);
}

int fdread(void *src, char *buf, int size)
{
    return read(*(int *)src, buf, size);
}

int fdwrite(void *dst, char *buf, int size)
{
    return write(*(int *)dst, buf, size);
}
