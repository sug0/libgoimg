#include <stdio.h>
#include <string.h>
#include "allocator.h"
#include "util.h"

static void *_im_heap_alloc(void *arg, size_t size)
{
    (void)arg;
    return malloc(size);
}

static void *_im_heap_realloc(void *arg, void *ptr, size_t size)
{
    (void)arg;
    return realloc(ptr, size);
}

static void _im_heap_free(void *arg, void *ptr)
{
    (void)arg;
    free(ptr);
}

static void _im_abort(void)
{
    fprintf(stderr, "libgoimg: failed to allocate memory, aborting...\n");
    exit(1);
}

static Allocator_t _im_std_allocator = {
    .alloc = _im_heap_alloc,
    .free = _im_heap_free,
    .realloc = _im_heap_realloc,
    .data = NULL
};

Allocator_t *im_std_allocator = &_im_std_allocator;
void (*im_abort)(void) = _im_abort;

/* -------------------------------------------------------------------------- */

void *im_xalloc(Allocator_t *allocator, size_t size)
{
    void *m = allocator->alloc(allocator->data, size);

    if (unlikely(!m))
        im_abort();

    return m;
}

void *im_xcalloc(Allocator_t *allocator, size_t nmemb, size_t size)
{
    void *m;
    size *= nmemb;
    m = im_xalloc(allocator, size);
    memset(m, 0, size);
    return m;
}

void *im_xrealloc(Allocator_t *allocator, void *ptr, size_t size)
{
    void *m = allocator->realloc(allocator->data, ptr, size);

    if (unlikely(!m))
        im_abort();

    return m;
}

inline void im_xfree(Allocator_t *allocator, void *ptr)
{
    if (likely(ptr))
        allocator->free(allocator->data, ptr);
}
