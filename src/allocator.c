#include <stdio.h>
#include <stdint.h>
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

inline void *im_xalloc(Allocator_t *allocator, size_t size)
{
    if (unlikely(!allocator->realloc))
        size += sizeof(size_t);

    void *m = allocator->alloc(allocator->data, size);

    if (unlikely(!m))
        im_abort();

    /* save size as fallback when no realloc exists */
    if (unlikely(!allocator->realloc)) {
        *(size_t *)m = size - sizeof(size_t);
        m = (intptr_t *)m + sizeof(size_t);
    }

    return m;
}

inline void *im_xcalloc(Allocator_t *allocator, size_t nmemb, size_t size)
{
    void *m;
    size *= nmemb;
    m = im_xalloc(allocator, size);
    memset(m, 0, size);
    return m;
}

inline void *im_xrealloc(Allocator_t *allocator, void *ptr, size_t size)
{
    /* fallback */
    if (unlikely(!allocator->realloc)) {
        void *const actual_ptr = (intptr_t *)ptr - sizeof(size_t);
        const size_t old_size = *(size_t *)actual_ptr;
        void *new_ptr = im_xalloc(allocator, size);
        memcpy(new_ptr, ptr, old_size);
        im_xfree(allocator, actual_ptr);
        return new_ptr;
    }

    void *m = allocator->realloc(allocator->data, ptr, size);

    if (unlikely(!m))
        im_abort();

    return m;
}

inline void im_xfree(Allocator_t *allocator, void *ptr)
{
    if (likely(ptr)) {
        if (unlikely(!allocator->realloc))
            ptr = (intptr_t *)ptr - sizeof(size_t);
        allocator->free(allocator->data, ptr);
    }
}
