#include "color.h"
#include "util.h"

inline uint32_t im_decl_nrgba(uint32_t r, uint32_t g, uint32_t b, uint32_t a)
{
    return (a << 24)|(b << 16)|(g << 8)|r;
}

void im_colormodel_nrgba(Color_t *dst, Color_t *src)
{
    if (unlikely(!dst->color || (dst->color && dst->size < sizeof(uint32_t)))) {
        if (dst->color)
            dst->free(dst->color);
        dst->color = _xalloc(dst->alloc, sizeof(uint32_t));
        dst->size = sizeof(uint32_t);
    }
    if (unlikely(dst->c_id != GOIMG_COLOR_NRGBA))
        dst->c_id = GOIMG_COLOR_NRGBA;

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
        .alloc = malloc,
        .free = free,
        .c_id = GOIMG_COLOR_NRGBA,
        .color = _xcalloc(calloc, 1, sizeof(uint32_t)),
        .size = sizeof(uint32_t),
        .rgba128 = im_nrgba_convert_rgba128
    };
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
            dst->free(dst->color);
        dst->color = _xalloc(dst->alloc, sizeof(uint64_t));
        dst->size = sizeof(uint64_t);
    }
    if (unlikely(dst->c_id != GOIMG_COLOR_NRGBA64))
        dst->c_id = GOIMG_COLOR_NRGBA64;

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
        .alloc = malloc,
        .free = free,
        .c_id = GOIMG_COLOR_NRGBA64,
        .color = _xcalloc(calloc, 1, sizeof(uint64_t)),
        .size = sizeof(uint64_t),
        .rgba128 = im_nrgba64_convert_rgba128
    };
}

/* -------------------------------------------------------------------------- */

void im_colormodel_gray(Color_t *dst, Color_t *src)
{
    if (unlikely(!dst->color || (dst->color && dst->size < sizeof(uint8_t)))) {
        if (dst->color)
            dst->free(dst->color);
        dst->color = _xalloc(dst->alloc, sizeof(uint8_t));
        dst->size = sizeof(uint8_t);
    }
    if (unlikely(dst->c_id != GOIMG_COLOR_NRGBA64))
        dst->c_id = GOIMG_COLOR_NRGBA64;

    if (likely(src->c_id == GOIMG_COLOR_GRAY)) {
        *(uint64_t *)dst->color = *(uint64_t *)src->color;
        return;
    }

    /* lossy conversion */
    RGBA128_t c;
    src->rgba128(&c, src->color);

    uint8_t y = (19595*c.r + 38470*c.g + 7471*c.b + 0x8000) >> 2;

    *(uint64_t *)dst->color = y;
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
        .alloc = malloc,
        .free = free,
        .c_id = im_register_color(),
        .color = _xcalloc(calloc, 1, sizeof(uint8_t)),
        .size = sizeof(uint8_t),
        .rgba128 = im_gray_convert_rgba128
    };
}
