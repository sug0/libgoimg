#ifndef GOIMG_COLOR_RGBA_H
#define GOIMG_COLOR_RGBA_H

#include "image.h"

#define GOIMG_DECL_RGBA(R, G, B, A)  (((R) << 24)|((G) << 16)|((B) << 8)|(A))

/* uses a uint32_t to save a color, i.e.
 * uint32_t color = (r << 24)|(g << 16)|(b << 8)|a; */
#define GOIMG_COLOR_RGBA  0

extern void im_colormodel_rgba(Color_t *dst, Color_t *src);
extern void im_rgba_convert_rgba128(RGBA128_t *rgba, void *color);
extern Color_t im_newcolor_rgba(void);

#endif

