#ifndef GOIMG_COLOR_H
#define GOIMG_COLOR_H

#include "image.h"
#include "allocator.h"

/* converts from a 32-bit to an 8-bit RGB(A)
 * color component */
#define GOIMG_CC(C)  (((C) >> 8) & 0xff)

/* converts from a 32-bit to a 16-bit RGB(A)
 * color component */
#define GOIMG_CC16(C)  ((C) & 0xffff)

/* defines a CMYK color based on percentage values */
#define im_decl_cmyk_p(C, M, Y, K)       \
    im_decl_cmyk((uint8_t)((C) * 255.0), \
                 (uint8_t)((M) * 255.0), \
                 (uint8_t)((Y) * 255.0), \
                 (uint8_t)((K) * 255.0)) \

enum _goimg_colors {
/* represents a non alpha premultiplied RGBA color */
    GOIMG_COLOR_NRGBA,
/* represents a non alpha premultiplied RGBA color,
 * with each component holding 16-bits */
    GOIMG_COLOR_NRGBA64,
/* represents an 8-bit grayscale color */
    GOIMG_COLOR_GRAY,
/* represents a 16-bit grayscale color */
    GOIMG_COLOR_GRAY16,
/* represents an opaque CMYK color */
    GOIMG_COLOR_CMYK,

    GOIMG_NO_DEF_COLORS
};

/* guesses the most appropriate color to use
 * on a decoded image, based on the color model */
extern Color_t im_newcolor_from_img(Image_t *img);

/* NRGBA color */
extern uint32_t im_decl_nrgba(uint32_t r, uint32_t g, uint32_t b, uint32_t a);
extern void im_colormodel_nrgba(Color_t *dst, Color_t *src);
extern void im_nrgba_convert_rgba128(RGBA128_t *rgba, void *color);
extern Color_t im_newcolor_nrgba(void);
extern Image_t im_newimg_nrgba(int w, int h, Allocator_t *allocator);
extern void im_nrgba_at(Image_t *img, int x, int y, Color_t *dst);
extern void im_nrgba_set(Image_t *img, int x, int y, Color_t *src);

/* NRGBA64 color */
extern uint64_t im_decl_nrgba64(uint64_t r, uint64_t g, uint64_t b, uint64_t a);
extern void im_colormodel_nrgba64(Color_t *dst, Color_t *src);
extern void im_nrgba64_convert_rgba128(RGBA128_t *rgba, void *color);
extern Color_t im_newcolor_nrgba64(void);
extern Image_t im_newimg_nrgba64(int w, int h, Allocator_t *allocator);
extern void im_nrgba64_at(Image_t *img, int x, int y, Color_t *dst);
extern void im_nrgba64_set(Image_t *img, int x, int y, Color_t *src);

/* gray color */
extern void im_colormodel_gray(Color_t *dst, Color_t *src);
extern void im_gray_convert_rgba128(RGBA128_t *rgba, void *color);
extern Color_t im_newcolor_gray(void);
extern Image_t im_newimg_gray(int w, int h, Allocator_t *allocator);
extern void im_gray_at(Image_t *img, int x, int y, Color_t *dst);
extern void im_gray_set(Image_t *img, int x, int y, Color_t *src);

/* gray16 color */
extern void im_colormodel_gray16(Color_t *dst, Color_t *src);
extern void im_gray16_convert_rgba128(RGBA128_t *rgba, void *color);
extern Color_t im_newcolor_gray16(void);
extern Image_t im_newimg_gray16(int w, int h, Allocator_t *allocator);
extern void im_gray16_at(Image_t *img, int x, int y, Color_t *dst);
extern void im_gray16_set(Image_t *img, int x, int y, Color_t *src);

/* CMYK color */
extern uint32_t im_decl_cmyk(uint32_t c, uint32_t m, uint32_t y, uint32_t k);
extern void im_colormodel_cmyk(Color_t *dst, Color_t *src);
extern void im_cmyk_convert_rgba128(RGBA128_t *rgba, void *color);
extern Color_t im_newcolor_cmyk(void);
extern Image_t im_newimg_cmyk(int w, int h, Allocator_t *allocator);
extern void im_cmyk_at(Image_t *img, int x, int y, Color_t *dst);
extern void im_cmyk_set(Image_t *img, int x, int y, Color_t *src);

#endif
