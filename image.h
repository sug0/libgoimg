#ifndef GOIMG_IMAGE_H
#define GOIMG_IMAGE_H

#include <stdlib.h>
#include <stdint.h>
#include "goio.h"
#include "allocator.h"

/* save up to GOIMG_NO_FMTS number of image
 * formats in the internal array */
#ifndef GOIMG_NO_FMTS
    #define GOIMG_NO_FMTS  64
#endif

/* typedefs for the struct types;
 * their respective descriptions are below */
typedef struct _s_im_image Image_t;
typedef struct _s_imgformat ImageFormat_t;
typedef struct _s_rgba128 RGBA128_t;
typedef struct _s_color Color_t;

/* a color model function -- takes 
 * converts a color to another color space */
typedef void (*cmfun_t)(Color_t *dst, Color_t *src);

/* decodes an image from a GOIO reader */
typedef int (*decfun_t)(Image_t *img, rfun_t rf, void *src);

/* encodes an image to a GOIO writer */
typedef int (*encfun_t)(Image_t *img, wfun_t wf, void *dst);

/* saves in 'dst' the color in the image 'img' at the
 * coordinates ('x', 'y') */
typedef void (*im_atfun_t)(Image_t *img, int x, int y, Color_t *dst);

/* sets the color at the coordinates ('x', 'y') in 'img' to 'src' */
typedef void (*im_setfun_t)(Image_t *img, int x, int y, Color_t *src);

/* represents an image to be encoded or 
 * decoded */
struct _s_im_image {
    Allocator_t *allocator;

    /* decoded parameters */
    void *img;
    size_t size;
    int w, h;

    cmfun_t color_model;
    im_atfun_t at;
    im_setfun_t set;
};

/* represents a rgba color with 32-bit
 * components */
struct _s_rgba128 {
    uint32_t r, g, b, a;
};

/* represents a generic color */
struct _s_color {
    Allocator_t *allocator;

    int c_id;
    void *color;
    size_t size;

    void (*rgba128)(RGBA128_t *rgba, void *color);
};

/* structure used to save an image format */
struct _s_imgformat {
    char *magic;
    int magic_size;
    char *name;
    decfun_t decode;
    encfun_t encode;
};

/* makes a format available to 'im_decode' and 'im_encode' */
extern void im_register_format(ImageFormat_t *fmt);

/* returns a new unique 'c_id' to be used by a particular
 * color format */
extern int im_register_color(void);

/* decodes an image of a known format; returns NULL on error,
 * or a pointer to the appropriately decoded image
 * format on success */
extern ImageFormat_t *im_decode(Image_t *img, rfun_t rf, void *src);

/* encodes an image into a known format; returns a negative
 * integer on error, and 0 on success */
extern int im_encode(Image_t *img, char *fmt, wfun_t wf, void *dst);

/* allocates and returns a new image with the given params */
extern Image_t im_newimg(int w, int h, cmfun_t color_model, Allocator_t *allocator);

/* allocate an exact copy of 'src' using the
 * allocator of 'dst'*/
extern void im_cpy(Image_t *dst, Image_t *src);

/* returns a pointer to an image format 'fmt',
 * or NULL, if no such format is registered; the
 * returned pointer references static memory,
 * so it should not be attempted to be free'd */
extern ImageFormat_t *im_get_format(char *fmt);

/* load the compiled formats -- this
 * function should be called before any
 * sort of image decoding/encoding has taken place */
extern void im_load_defaults(void);

#endif
