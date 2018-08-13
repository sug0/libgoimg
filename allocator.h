#ifndef GOIMG_ALLOCATOR_H
#define GOIMG_ALLOCATOR_H

#include <stdlib.h>

/* represents an allocator with the possibility
 * to pass in a user data void* argument */
typedef struct _s_allocator Allocator_t;
struct _s_allocator {
    void *(*alloc)(void *data, size_t size);
    void *(*realloc)(void *data, void *ptr, size_t size);
    void (*free)(void *data, void *ptr);
    void *data;
};

/* the standard heap allocator */
extern Allocator_t *im_std_allocator;

/* these methods are wrappers for the allocator
 * object, that abort execution when no memory is
 * available to be allocated */
extern void *im_xalloc(Allocator_t *allocator, size_t size);
extern void *im_xcalloc(Allocator_t *allocator, size_t nmemb, size_t size);
extern void *im_xrealloc(Allocator_t *allocator, void *ptr, size_t size);
extern void im_xfree(Allocator_t *allocator, void *ptr);

#endif
