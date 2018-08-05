#ifndef GOIMG_COLOR_RGBA_H
#define GOIMG_COLOR_RGBA_H

#include "image.h"

/* converts from a 32-bit to an 8-bit RGB(A)
 * color component */
#define GOIMG_CC(C)  (((C) >> 8) & 0xff)

/* converts from a 32-bit to a 16-bit RGB(A)
 * color component */
#define GOIMG_CC16(C)  ((C) & 0xffff)

/* represents a non alpha premultiplied RGBA color */
#define GOIMG_COLOR_NRGBA  0

/* represents a non alpha premultiplied RGBA color,
 * with each component holding 16-bits */
#define GOIMG_COLOR_NRGBA64  1

/* encodes an NRGBA color as a uint32_t */
extern uint32_t im_decl_nrgba(uint32_t r, uint32_t g, uint32_t b, uint32_t a);

/* encodes an NRGBA64 color as a uint64_t */
extern uint64_t im_decl_nrgba64(uint64_t r, uint64_t g, uint64_t b, uint64_t a);

/* NRGBA color */
extern void im_colormodel_nrgba(Color_t *dst, Color_t *src);
extern void im_nrgba_convert_rgba128(RGBA128_t *rgba, void *color);
extern Color_t im_newcolor_nrgba(void);

/* NRGBA64 color */
extern void im_colormodel_nrgba64(Color_t *dst, Color_t *src);
extern void im_nrgba64_convert_rgba128(RGBA128_t *rgba, void *color);
extern Color_t im_newcolor_nrgba64(void);

#endif

