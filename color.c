#include "color.h"
#include "util.h"

inline Color_t im_newcolor_from_img(Image_t *img)
{
    if (img->color_model == im_colormodel_nrgba)
        return im_newcolor_nrgba();
    else if (img->color_model == im_colormodel_nrgba64)
        return im_newcolor_nrgba64();
    else if (img->color_model == im_colormodel_gray)
        return im_newcolor_gray();
    else if (img->color_model == im_colormodel_cmyk)
        return im_newcolor_cmyk();
    else
        return im_newcolor_nrgba();
}

/* -------------------------------------------------------------------------- */

inline uint32_t im_decl_nrgba(uint32_t r, uint32_t g, uint32_t b, uint32_t a)
{
    return (a << 24)|(b << 16)|(g << 8)|r;
}

void im_colormodel_nrgba(Color_t *dst, Color_t *src)
{
    if (unlikely(!dst->color || (dst->color && dst->size < sizeof(uint32_t)))) {
        if (dst->color)
            im_xfree(dst->allocator, dst->color);
        dst->color = im_xalloc(dst->allocator, sizeof(uint32_t));
        dst->size = sizeof(uint32_t);
    }
    if (unlikely(dst->c_id != GOIMG_COLOR_NRGBA)) {
        dst->c_id = GOIMG_COLOR_NRGBA;
        dst->rgba128 = im_nrgba_convert_rgba128;
    }

    if (likely(src->c_id == GOIMG_COLOR_NRGBA)) {
        *(uint32_t *)dst->color = *(uint32_t *)src->color;
        return;
    }

    /* lossy conversion */
    RGBA128_t c;
    src->rgba128(&c, src->color);

    *(uint32_t *)dst->color = im_decl_nrgba(GOIMG_CC(c.r), GOIMG_CC(c.g),
                                            GOIMG_CC(c.b), GOIMG_CC(c.a));
}

void im_nrgba_convert_rgba128(RGBA128_t *rgba, void *color)
{
    rgba->a = (*(uint32_t *)color >> 24);
    rgba->b = (*(uint32_t *)color >> 16) & 0xff;
    rgba->g = (*(uint32_t *)color >> 8) & 0xff;
    rgba->r = (*(uint32_t *)color & 0xff);

    rgba->r |= rgba->r << 8;
    rgba->r *= rgba->a;
    rgba->r /= 0xff;

    rgba->g |= rgba->g << 8;
    rgba->g *= rgba->a;
    rgba->g /= 0xff;

    rgba->b |= rgba->b << 8;
    rgba->b *= rgba->a;
    rgba->b /= 0xff;

    rgba->a |= rgba->a << 8;
}

inline Color_t im_newcolor_nrgba(void)
{
    return (Color_t){
        .allocator = im_std_allocator,
        .c_id = GOIMG_COLOR_NRGBA,
        .color = im_xcalloc(im_std_allocator, 1, sizeof(uint32_t)),
        .size = sizeof(uint32_t),
        .rgba128 = im_nrgba_convert_rgba128
    };
}

inline Image_t im_newimg_nrgba(int w, int h, Allocator_t *allocator)
{
    size_t size = w * h * sizeof(uint32_t);
    allocator = allocator ?: im_std_allocator;
    return (Image_t){
        .allocator = allocator,
        .img = im_xalloc(allocator, size),
        .size = size,
        .w = w,
        .h = h,
        .color_model = im_colormodel_nrgba,
        .at = im_nrgba_at,
        .set = im_nrgba_set
    };
}

void im_nrgba_at(Image_t *img, int x, int y, Color_t *dst)
{
    if (unlikely(!dst->color || (dst->color && dst->size < sizeof(uint32_t)))) {
        if (dst->color)
            im_xfree(dst->allocator, dst->color);
        dst->color = im_xalloc(dst->allocator, sizeof(uint32_t));
        dst->size = sizeof(uint32_t);
    }

    if (unlikely(dst->c_id != GOIMG_COLOR_NRGBA)) {
        dst->c_id = GOIMG_COLOR_NRGBA;
        dst->rgba128 = im_nrgba_convert_rgba128;
    }

    *(uint32_t *)dst->color = ((uint32_t *)img->img)[y * img->w + x];
}

void im_nrgba_set(Image_t *img, int x, int y, Color_t *src)
{
    uint32_t color;

    if (unlikely(src->c_id != GOIMG_COLOR_NRGBA)) {
        RGBA128_t c;
        src->rgba128(&c, src->color);
        color = im_decl_nrgba(GOIMG_CC(c.r), GOIMG_CC(c.g),
                              GOIMG_CC(c.b), GOIMG_CC(c.a));
    } else {
        color = *(uint32_t *)src->color;
    }

    ((uint32_t *)img->img)[y * img->w + x] = color;
}

/* -------------------------------------------------------------------------- */

inline uint64_t im_decl_nrgba64(uint64_t r, uint64_t g, uint64_t b, uint64_t a)
{
    return (a << 48)|(b << 32)|(g << 16)|r;
}

void im_colormodel_nrgba64(Color_t *dst, Color_t *src)
{
    if (unlikely(!dst->color || (dst->color && dst->size < sizeof(uint64_t)))) {
        if (dst->color)
            im_xfree(dst->allocator, dst->color);
        dst->color = im_xalloc(dst->allocator, sizeof(uint64_t));
        dst->size = sizeof(uint64_t);
    }
    if (unlikely(dst->c_id != GOIMG_COLOR_NRGBA64)) {
        dst->c_id = GOIMG_COLOR_NRGBA64;
        dst->rgba128 = im_nrgba64_convert_rgba128;
    }

    if (likely(src->c_id == GOIMG_COLOR_NRGBA64)) {
        *(uint64_t *)dst->color = *(uint64_t *)src->color;
        return;
    }

    /* lossy conversion */
    RGBA128_t c;
    src->rgba128(&c, src->color);

    *(uint64_t *)dst->color = im_decl_nrgba64(GOIMG_CC16(c.r), GOIMG_CC16(c.g),
                                              GOIMG_CC16(c.b), GOIMG_CC16(c.a));
}

void im_nrgba64_convert_rgba128(RGBA128_t *rgba, void *color)
{
    rgba->a = (uint32_t)(*(uint64_t *)color >> 48);
    rgba->b = (uint32_t)((*(uint64_t *)color >> 32) & 0xffff);
    rgba->g = (uint32_t)((*(uint64_t *)color >> 16) & 0xffff);
    rgba->r = (uint32_t)((*(uint64_t *)color & 0xffff));

    rgba->r *= rgba->a;
    rgba->r /= 0xffff;

    rgba->g *= rgba->a;
    rgba->g /= 0xffff;

    rgba->b *= rgba->a;
    rgba->b /= 0xffff;
}

inline Color_t im_newcolor_nrgba64(void)
{
    return (Color_t){
        .allocator = im_std_allocator,
        .c_id = GOIMG_COLOR_NRGBA64,
        .color = im_xcalloc(im_std_allocator, 1, sizeof(uint64_t)),
        .size = sizeof(uint64_t),
        .rgba128 = im_nrgba64_convert_rgba128
    };
}

inline Image_t im_newimg_nrgba64(int w, int h, Allocator_t *allocator)
{
    size_t size = w * h * sizeof(uint64_t);
    allocator = allocator ?: im_std_allocator;
    return (Image_t){
        .allocator = allocator,
        .img = im_xalloc(allocator, size),
        .size = size,
        .w = w,
        .h = h,
        .color_model = im_colormodel_nrgba64,
        .at = im_nrgba64_at,
        .set = im_nrgba64_set
    };
}

void im_nrgba64_at(Image_t *img, int x, int y, Color_t *dst)
{
    if (unlikely(!dst->color || (dst->color && dst->size < sizeof(uint64_t)))) {
        if (dst->color)
            im_xfree(dst->allocator, dst->color);
        dst->color = im_xalloc(dst->allocator, sizeof(uint64_t));
        dst->size = sizeof(uint64_t);
    }

    if (unlikely(dst->c_id != GOIMG_COLOR_NRGBA64)) {
        dst->c_id = GOIMG_COLOR_NRGBA64;
        dst->rgba128 = im_nrgba64_convert_rgba128;
    }

    *(uint64_t *)dst->color = ((uint64_t *)img->img)[y * img->w + x];
}

void im_nrgba64_set(Image_t *img, int x, int y, Color_t *src)
{
    uint64_t color;

    if (unlikely(src->c_id != GOIMG_COLOR_NRGBA64)) {
        RGBA128_t c;
        src->rgba128(&c, src->color);
        color = im_decl_nrgba64(GOIMG_CC16(c.r), GOIMG_CC16(c.g),
                                GOIMG_CC16(c.b), GOIMG_CC16(c.a));
    } else {
        color = *(uint64_t *)src->color;
    }

    ((uint64_t *)img->img)[y * img->w + x] = color;
}

/* -------------------------------------------------------------------------- */

void im_colormodel_gray(Color_t *dst, Color_t *src)
{
    if (unlikely(!dst->color || (dst->color && dst->size < sizeof(uint8_t)))) {
        if (dst->color)
            im_xfree(dst->allocator, dst->color);
        dst->color = im_xalloc(dst->allocator, sizeof(uint8_t));
        dst->size = sizeof(uint8_t);
    }
    if (unlikely(dst->c_id != GOIMG_COLOR_GRAY)) {
        dst->c_id = GOIMG_COLOR_GRAY;
        dst->rgba128 = im_gray_convert_rgba128;
    }

    if (likely(src->c_id == GOIMG_COLOR_GRAY)) {
        *(uint8_t *)dst->color = *(uint8_t *)src->color;
        return;
    }

    /* lossy conversion */
    RGBA128_t c;
    src->rgba128(&c, src->color);

    uint8_t y = (19595*c.r + 38470*c.g + 7471*c.b + 0x8000) >> 24;

    *(uint8_t *)dst->color = y;
}

void im_gray_convert_rgba128(RGBA128_t *rgba, void *color)
{
    uint32_t y;
    
    y = *(uint8_t *)color;
    y |= y << 8;

    rgba->r = y;
    rgba->g = y;
    rgba->b = y;
    rgba->a = 0xffff;
}

inline Color_t im_newcolor_gray(void)
{
    return (Color_t){
        .allocator = im_std_allocator,
        .c_id = GOIMG_COLOR_GRAY,
        .color = im_xcalloc(im_std_allocator, 1, sizeof(uint8_t)),
        .size = sizeof(uint8_t),
        .rgba128 = im_gray_convert_rgba128
    };
}

inline Image_t im_newimg_gray(int w, int h, Allocator_t *allocator)
{
    size_t size = w * h * sizeof(uint8_t);
    allocator = allocator ?: im_std_allocator;
    return (Image_t){
        .allocator = allocator,
        .img = im_xalloc(allocator, size),
        .size = size,
        .w = w,
        .h = h,
        .color_model = im_colormodel_gray,
        .at = im_gray_at,
        .set = im_gray_set
    };
}

void im_gray_at(Image_t *img, int x, int y, Color_t *dst)
{
    if (unlikely(!dst->color || (dst->color && dst->size < sizeof(uint8_t)))) {
        if (dst->color)
            im_xfree(dst->allocator, dst->color);
        dst->color = im_xalloc(dst->allocator, sizeof(uint8_t));
        dst->size = sizeof(uint8_t);
    }

    if (unlikely(dst->c_id != GOIMG_COLOR_GRAY)) {
        dst->c_id = GOIMG_COLOR_GRAY;
        dst->rgba128 = im_gray_convert_rgba128;
    }

    *(uint8_t *)dst->color = ((uint8_t *)img->img)[y * img->w + x];
}

void im_gray_set(Image_t *img, int x, int y, Color_t *src)
{
    uint8_t color;

    if (unlikely(src->c_id != GOIMG_COLOR_GRAY)) {
        RGBA128_t c;
        src->rgba128(&c, src->color);
        color = (19595*c.r + 38470*c.g + 7471*c.b + 0x8000) >> 24;
    } else {
        color = *(uint8_t *)src->color;
    }

    ((uint8_t *)img->img)[y * img->w + x] = color;
}

/* -------------------------------------------------------------------------- */

void im_colormodel_gray16(Color_t *dst, Color_t *src)
{
    if (unlikely(!dst->color || (dst->color && dst->size < sizeof(uint16_t)))) {
        if (dst->color)
            im_xfree(dst->allocator, dst->color);
        dst->color = im_xalloc(dst->allocator, sizeof(uint16_t));
        dst->size = sizeof(uint16_t);
    }
    if (unlikely(dst->c_id != GOIMG_COLOR_GRAY16)) {
        dst->c_id = GOIMG_COLOR_GRAY16;
        dst->rgba128 = im_gray16_convert_rgba128;
    }

    if (likely(src->c_id == GOIMG_COLOR_GRAY16)) {
        *(uint16_t *)dst->color = *(uint16_t *)src->color;
        return;
    }

    /* lossy conversion */
    RGBA128_t c;
    src->rgba128(&c, src->color);

    uint16_t y = (19595*c.r + 38470*c.g + 7471*c.b + 0x8000) >> 16;

    *(uint16_t *)dst->color = y;
}

void im_gray16_convert_rgba128(RGBA128_t *rgba, void *color)
{
    rgba->r = *(uint16_t *)color;
    rgba->g = *(uint16_t *)color;
    rgba->b = *(uint16_t *)color;
    rgba->a = 0xffff;
}

inline Color_t im_newcolor_gray16(void)
{
    return (Color_t){
        .allocator = im_std_allocator,
        .c_id = GOIMG_COLOR_GRAY16,
        .color = im_xcalloc(im_std_allocator, 1, sizeof(uint16_t)),
        .size = sizeof(uint16_t),
        .rgba128 = im_gray16_convert_rgba128
    };
}

inline Image_t im_newimg_gray16(int w, int h, Allocator_t *allocator)
{
    size_t size = w * h * sizeof(uint16_t);
    allocator = allocator ?: im_std_allocator;
    return (Image_t){
        .allocator = allocator,
        .img = im_xalloc(allocator, size),
        .size = size,
        .w = w,
        .h = h,
        .color_model = im_colormodel_gray16,
        .at = im_gray16_at,
        .set = im_gray16_set
    };
}

void im_gray16_at(Image_t *img, int x, int y, Color_t *dst)
{
    if (unlikely(!dst->color || (dst->color && dst->size < sizeof(uint16_t)))) {
        if (dst->color)
            im_xfree(dst->allocator, dst->color);
        dst->color = im_xalloc(dst->allocator, sizeof(uint16_t));
        dst->size = sizeof(uint16_t);
    }

    if (unlikely(dst->c_id != GOIMG_COLOR_GRAY16)) {
        dst->c_id = GOIMG_COLOR_GRAY16;
        dst->rgba128 = im_gray16_convert_rgba128;
    }

    *(uint16_t *)dst->color = ((uint16_t *)img->img)[y * img->w + x];
}

void im_gray16_set(Image_t *img, int x, int y, Color_t *src)
{
    uint16_t color;

    if (unlikely(src->c_id != GOIMG_COLOR_GRAY16)) {
        RGBA128_t c;
        src->rgba128(&c, src->color);
        color = (19595*c.r + 38470*c.g + 7471*c.b + 0x8000) >> 16;
    } else {
        color = *(uint16_t *)src->color;
    }

    ((uint16_t *)img->img)[y * img->w + x] = color;
}

/* -------------------------------------------------------------------------- */

inline uint32_t im_decl_cmyk(uint32_t c, uint32_t m, uint32_t y, uint32_t k)
{
    return (k << 24)|(y << 16)|(m << 8)|c;
}

void im_colormodel_cmyk(Color_t *dst, Color_t *src)
{
    if (unlikely(!dst->color || (dst->color && dst->size < sizeof(uint32_t)))) {
        if (dst->color)
            im_xfree(dst->allocator, dst->color);
        dst->color = im_xalloc(dst->allocator, sizeof(uint32_t));
        dst->size = sizeof(uint32_t);
    }
    if (unlikely(dst->c_id != GOIMG_COLOR_CMYK)) {
        dst->c_id = GOIMG_COLOR_CMYK;
        dst->rgba128 = im_cmyk_convert_rgba128;
    }

    if (likely(src->c_id == GOIMG_COLOR_CMYK)) {
        *(uint32_t *)dst->color = *(uint32_t *)src->color;
        return;
    }

    /* lossy conversion */
    RGBA128_t col;
    uint32_t c, m, y, w;
    src->rgba128(&col, src->color);

    w = col.r;
    if (w < col.g) w = col.g;
    if (w < col.b) w = col.b;

    if (w == 0) {
        *(uint32_t *)dst->color = 0xff000000;
        return;
    }

    c = (w - col.r) * 0xffff / w;
    m = (w - col.g) * 0xffff / w;
    y = (w - col.b) * 0xffff / w;

    *(uint32_t *)dst->color = im_decl_cmyk(GOIMG_CC(c), GOIMG_CC(m),
                                           GOIMG_CC(y), GOIMG_CC(0xffff - w));
}

void im_cmyk_convert_rgba128(RGBA128_t *rgba, void *color)
{
    uint32_t c, m, y, k, w;

    k = (*(uint32_t *)color >> 24);
    y = (*(uint32_t *)color >> 16) & 0xff;
    m = (*(uint32_t *)color >> 8) & 0xff;
    c = (*(uint32_t *)color & 0xff);

    w = 0xffff - k*0x101;

    rgba->r = (0xffff - c*0x101) * w / 0xffff;
    rgba->g = (0xffff - m*0x101) * w / 0xffff;
    rgba->b = (0xffff - y*0x101) * w / 0xffff;
    rgba->a = 0xffff;
}

inline Color_t im_newcolor_cmyk(void)
{
    return (Color_t){
        .allocator = im_std_allocator,
        .c_id = GOIMG_COLOR_CMYK,
        .color = im_xcalloc(im_std_allocator, 1, sizeof(uint32_t)),
        .size = sizeof(uint32_t),
        .rgba128 = im_cmyk_convert_rgba128
    };
}

inline Image_t im_newimg_cmyk(int w, int h, Allocator_t *allocator)
{
    size_t size = w * h * sizeof(uint32_t);
    allocator = allocator ?: im_std_allocator;
    return (Image_t){
        .allocator = allocator,
        .img = im_xalloc(allocator, size),
        .size = size,
        .w = w,
        .h = h,
        .color_model = im_colormodel_cmyk,
        .at = im_cmyk_at,
        .set = im_cmyk_set
    };
}

void im_cmyk_at(Image_t *img, int x, int y, Color_t *dst)
{
    if (unlikely(!dst->color || (dst->color && dst->size < sizeof(uint32_t)))) {
        if (dst->color)
            im_xfree(dst->allocator, dst->color);
        dst->color = im_xalloc(dst->allocator, sizeof(uint32_t));
        dst->size = sizeof(uint32_t);
    }

    if (unlikely(dst->c_id != GOIMG_COLOR_CMYK)) {
        dst->c_id = GOIMG_COLOR_CMYK;
        dst->rgba128 = im_cmyk_convert_rgba128;
    }

    *(uint32_t *)dst->color = ((uint32_t *)img->img)[y * img->w + x];
}

void im_cmyk_set(Image_t *img, int x, int y, Color_t *src)
{
    uint32_t color;

    if (unlikely(src->c_id != GOIMG_COLOR_CMYK)) {
        RGBA128_t col;
        uint32_t c, m, y, w;
        src->rgba128(&col, src->color);

        w = col.r;
        if (w < col.g) w = col.g;
        if (w < col.b) w = col.b;

        if (w == 0) {
            color = 0xff000000;
        } else {
            c = (w - col.r) * 0xffff / w;
            m = (w - col.g) * 0xffff / w;
            y = (w - col.b) * 0xffff / w;

            color = im_decl_cmyk(GOIMG_CC(c), GOIMG_CC(m),
                                 GOIMG_CC(y), GOIMG_CC(0xffff - w));
        }
    } else {
        color = *(uint32_t *)src->color;
    }

    ((uint32_t *)img->img)[y * img->w + x] = color;
}
