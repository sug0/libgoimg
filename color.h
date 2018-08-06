#ifndef GOIMG_COLOR_RGBA_H
#define GOIMG_COLOR_RGBA_H

#include "image.h"

/* converts from a 32-bit to an 8-bit RGB(A)
 * color component */
#define GOIMG_CC(C)  (((C) >> 8) & 0xff)

/* converts from a 32-bit to a 16-bit RGB(A)
 * color component */
#define GOIMG_CC16(C)  ((C) & 0xffff)

enum _goimg_colors {
/* represents a non alpha premultiplied RGBA color */
    GOIMG_COLOR_NRGBA,
/* represents a non alpha premultiplied RGBA color,
 * with each component holding 16-bits */
    GOIMG_COLOR_NRGBA64,
/* represents an 8-bit grayscale color */
    GOIMG_COLOR_GRAY,

    GOIMG_NO_DEF_COLORS
};

/* NRGBA color */
extern uint32_t im_decl_nrgba(uint32_t r, uint32_t g, uint32_t b, uint32_t a);
extern void im_colormodel_nrgba(Color_t *dst, Color_t *src);
extern void im_nrgba_convert_rgba128(RGBA128_t *rgba, void *color);
extern Color_t im_newcolor_nrgba(void);

/* NRGBA64 color */
extern uint64_t im_decl_nrgba64(uint64_t r, uint64_t g, uint64_t b, uint64_t a);
extern void im_colormodel_nrgba64(Color_t *dst, Color_t *src);
extern void im_nrgba64_convert_rgba128(RGBA128_t *rgba, void *color);
extern Color_t im_newcolor_nrgba64(void);
extern Image_t im_newimg_nrgba64(int w, int h, void *(*alloc)(size_t), void (*free)(void *));
extern void im_nrgba64_at(Image_t *img, int x, int y, Color_t *dst);
extern void im_nrgba64_set(Image_t *img, int x, int y, Color_t *src);

/* gray color */
extern void im_colormodel_gray(Color_t *dst, Color_t *src);
extern void im_gray_convert_rgba128(RGBA128_t *rgba, void *color);
extern Color_t im_newcolor_gray(void);

#endif

