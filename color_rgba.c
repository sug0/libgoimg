#include "color_rgba.h"
#include "util.h"

void im_colormodel_rgba(Color_t *dst, Color_t *src)
{
    if (unlikely(!dst->color || (dst->color && dst->size != sizeof(uint32_t)))) {
        if (dst->color)
            dst->free(dst->color);
        dst->color = _xalloc(dst->alloc, sizeof(uint32_t));
        dst->size = sizeof(uint32_t);
    }
    if (unlikely(dst->c_id != GOIMG_COLOR_RGBA))
        dst->c_id = GOIMG_COLOR_RGBA;

    if (likely(src->c_id == GOIMG_COLOR_RGBA)) {
        *(uint32_t *)dst->color = *(uint32_t *)src->color;
        return;
    }

    /* lossy conversion */
    RGBA128_t c;
    src->rgba128(&c, src->color);

    *(uint32_t *)dst->color = GOIMG_DECL_RGBA(GOIMG_CC(c.r), GOIMG_CC(c.g),
                                              GOIMG_CC(c.b), GOIMG_CC(c.a));
}

void im_rgba_convert_rgba128(RGBA128_t *rgba, void *color)
{
    rgba->r = (*(uint32_t *)color >> 24);
    rgba->g = (*(uint32_t *)color >> 16) & 0xff;
    rgba->b = (*(uint32_t *)color >> 8) & 0xff;
    rgba->a = (*(uint32_t *)color & 0xff);

    rgba->r |= rgba->r << 8;
    rgba->g |= rgba->g << 8;
    rgba->b |= rgba->b << 8;
    rgba->a |= rgba->a << 8;
}

inline Color_t im_newcolor_rgba(void)
{
    return (Color_t){
        .alloc = malloc,
        .free = free,
        .c_id = GOIMG_COLOR_RGBA,
        .color = _xcalloc(calloc, 1, sizeof(uint32_t)),
        .size = sizeof(uint32_t),
        .rgba128 = im_rgba_convert_rgba128
    };
}
