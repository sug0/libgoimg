#include "color.h"
#include "util.h"

inline void im_initcolor_from_img(Image_t *img, Color_t *col)
{
    if (img->color_model == im_colormodel_nrgba)
        im_initcolor_nrgba(col);
    else if (img->color_model == im_colormodel_nrgba64)
        im_initcolor_nrgba64(col);
    else if (img->color_model == im_colormodel_rgb)
        im_initcolor_rgb(col);
    else if (img->color_model == im_colormodel_gray)
        im_initcolor_gray(col);
    else if (img->color_model == im_colormodel_gray16)
        im_initcolor_gray16(col);
    else if (img->color_model == im_colormodel_cmyk)
        im_initcolor_cmyk(col);
    else
        im_initcolor_nrgba(col);
}

inline Color_t im_newcolor_from_img(Image_t *img)
{
    Color_t col;
    im_initcolor_from_img(img, &col);
    return col;
}

/* -------------------------------------------------------------------------- */

inline RGB_t im_decl_rgb(uint8_t r, uint8_t g, uint8_t b)
{
    return (RGB_t){r, g, b};
}

void im_colormodel_rgb(Color_t *dst, Color_t *src)
{
    if (unlikely(!dst->color || (dst->color && dst->size < sizeof(RGB_t)))) {
        if (dst->color)
            im_xfree(dst->allocator, dst->color);
        dst->color = im_xalloc(dst->allocator, sizeof(RGB_t));
        dst->size = sizeof(RGB_t);
    }
    if (unlikely(dst->c_id != GOIMG_COLOR_RGB)) {
        dst->c_id = GOIMG_COLOR_RGB;
        dst->rgba128 = im_rgb_convert_rgba128;
    }

    if (likely(src->c_id == GOIMG_COLOR_RGB)) {
        *(RGB_t *)dst->color = *(RGB_t *)src->color;
        return;
    }

    /* lossy conversion */
    RGBA128_t c;
    RGB_t *rgb = dst->color;
    src->rgba128(&c, src->color);

    rgb->r = GOIMG_CC(c.r);
    rgb->g = GOIMG_CC(c.g);
    rgb->b = GOIMG_CC(c.b);
}

void im_rgb_convert_rgba128(RGBA128_t *rgba, void *color)
{
    RGB_t *rgb = color;

    rgba->r = rgb->r;
    rgba->g = rgb->g;
    rgba->b = rgb->b;

    rgba->r |= rgba->r << 8;
    rgba->g |= rgba->g << 8;
    rgba->b |= rgba->b << 8;

    rgba->a = 0xffff;
}

inline void im_initcolor_rgb(Color_t *col)
{
    col->allocator = im_std_allocator;
    col->c_id = GOIMG_COLOR_RGB;
    col->color = im_xcalloc(im_std_allocator, 1, sizeof(RGB_t));
    col->size = sizeof(RGB_t);
    col->rgba128 = im_rgb_convert_rgba128;
}

inline Color_t im_newcolor_rgb(void)
{
    Color_t col;
    im_initcolor_rgb(&col);
    return col;
}

inline void im_initimg_rgb(Image_t *img, int w, int h, Allocator_t *allocator)
{
    size_t size = w * h * sizeof(RGB_t);
    img->allocator = allocator ? allocator : im_std_allocator;
    img->img = im_xalloc(allocator, size);
    img->size = size;
    img->w = w;
    img->h = h;
    img->color_model = im_colormodel_rgb;
    img->at = im_rgb_at;
    img->set = im_rgb_set;
}

inline Image_t im_newimg_rgb(int w, int h, Allocator_t *allocator)
{
    Image_t img;
    im_initimg_rgb(&img, w, h, allocator);
    return img;
}

void im_rgb_at(Image_t *img, int x, int y, Color_t *dst)
{
    if (unlikely(!dst->color || (dst->color && dst->size < sizeof(RGB_t)))) {
        if (dst->color)
            im_xfree(dst->allocator, dst->color);
        dst->color = im_xalloc(dst->allocator, sizeof(RGB_t));
        dst->size = sizeof(RGB_t);
    }

    if (unlikely(dst->c_id != GOIMG_COLOR_RGB)) {
        dst->c_id = GOIMG_COLOR_RGB;
        dst->rgba128 = im_rgb_convert_rgba128;
    }

    *(RGB_t *)dst->color = ((RGB_t *)img->img)[y * img->w + x];
}

void im_rgb_set(Image_t *img, int x, int y, Color_t *src)
{
    RGB_t rgb;

    if (unlikely(src->c_id != GOIMG_COLOR_RGB)) {
        RGBA128_t c;
        src->rgba128(&c, src->color);
        rgb.r = GOIMG_CC(c.r);
        rgb.g = GOIMG_CC(c.g);
        rgb.b = GOIMG_CC(c.b);
    } else {
        rgb = *(RGB_t *)src->color;
    }

    ((RGB_t *)img->img)[y * img->w + x] = rgb;
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
    //rgba->r *= rgba->a;
    //rgba->r /= 0xff;

    rgba->g |= rgba->g << 8;
    //rgba->g *= rgba->a;
    //rgba->g /= 0xff;

    rgba->b |= rgba->b << 8;
    //rgba->b *= rgba->a;
    //rgba->b /= 0xff;

    rgba->a |= rgba->a << 8;
}

inline void im_initcolor_nrgba(Color_t *col)
{
    col->allocator = im_std_allocator;
    col->c_id = GOIMG_COLOR_NRGBA;
    col->color = im_xcalloc(im_std_allocator, 1, sizeof(uint32_t));
    col->size = sizeof(uint32_t);
    col->rgba128 = im_nrgba_convert_rgba128;
}

inline Color_t im_newcolor_nrgba(void)
{
    Color_t col;
    im_initcolor_nrgba(&col);
    return col;
}

inline void im_initimg_nrgba(Image_t *img, int w, int h, Allocator_t *allocator)
{
    size_t size = w * h * sizeof(uint32_t);
    img->allocator = allocator ? allocator : im_std_allocator;
    img->img = im_xalloc(allocator, size);
    img->size = size;
    img->w = w;
    img->h = h;
    img->color_model = im_colormodel_nrgba;
    img->at = im_nrgba_at;
    img->set = im_nrgba_set;
}

inline Image_t im_newimg_nrgba(int w, int h, Allocator_t *allocator)
{
    Image_t img;
    im_initimg_nrgba(&img, w, h, allocator);
    return img;
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

inline void im_initcolor_nrgba64(Color_t *col)
{
    col->allocator = im_std_allocator;
    col->c_id = GOIMG_COLOR_NRGBA64;
    col->color = im_xcalloc(im_std_allocator, 1, sizeof(uint64_t));
    col->size = sizeof(uint64_t);
    col->rgba128 = im_nrgba64_convert_rgba128;
}

inline Color_t im_newcolor_nrgba64(void)
{
    Color_t col;
    im_initcolor_nrgba64(&col);
    return col;
}

inline void im_initimg_nrgba64(Image_t *img, int w, int h, Allocator_t *allocator)
{
    size_t size = w * h * sizeof(uint64_t);
    img->allocator = allocator ? allocator : im_std_allocator;
    img->img = im_xalloc(allocator, size);
    img->size = size;
    img->w = w;
    img->h = h;
    img->color_model = im_colormodel_nrgba64;
    img->at = im_nrgba64_at;
    img->set = im_nrgba64_set;
}

inline Image_t im_newimg_nrgba64(int w, int h, Allocator_t *allocator)
{
    Image_t img;
    im_initimg_nrgba64(&img, w, h, allocator);
    return img;
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

inline void im_initcolor_gray(Color_t *col)
{
    col->allocator = im_std_allocator;
    col->c_id = GOIMG_COLOR_GRAY;
    col->color = im_xcalloc(im_std_allocator, 1, sizeof(uint8_t));
    col->size = sizeof(uint8_t);
    col->rgba128 = im_gray_convert_rgba128;
}

inline Color_t im_newcolor_gray(void)
{
    Color_t col;
    im_initcolor_gray(&col);
    return col;
}

inline void im_initimg_gray(Image_t *img, int w, int h, Allocator_t *allocator)
{
    size_t size = w * h * sizeof(uint8_t);
    img->allocator = allocator ? allocator : im_std_allocator;
    img->img = im_xalloc(allocator, size);
    img->size = size;
    img->w = w;
    img->h = h;
    img->color_model = im_colormodel_gray;
    img->at = im_gray_at;
    img->set = im_gray_set;
}

inline Image_t im_newimg_gray(int w, int h, Allocator_t *allocator)
{
    Image_t img;
    im_initimg_gray(&img, w, h, allocator);
    return img;
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

inline void im_initcolor_gray16(Color_t *col)
{
    col->allocator = im_std_allocator;
    col->c_id = GOIMG_COLOR_GRAY16;
    col->color = im_xcalloc(im_std_allocator, 1, sizeof(uint16_t));
    col->size = sizeof(uint16_t);
    col->rgba128 = im_gray16_convert_rgba128;
}

inline Color_t im_newcolor_gray16(void)
{
    Color_t col;
    im_initcolor_gray16(&col);
    return col;
}

inline void im_initimg_gray16(Image_t *img, int w, int h, Allocator_t *allocator)
{
    size_t size = w * h * sizeof(uint16_t);
    img->allocator = allocator ? allocator : im_std_allocator;
    img->img = im_xalloc(allocator, size);
    img->size = size;
    img->w = w;
    img->h = h;
    img->color_model = im_colormodel_gray16;
    img->at = im_gray16_at;
    img->set = im_gray16_set;
}

inline Image_t im_newimg_gray16(int w, int h, Allocator_t *allocator)
{
    Image_t img;
    im_initimg_gray16(&img, w, h, allocator);
    return img;
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

inline void im_initcolor_cmyk(Color_t *col)
{
    col->allocator = im_std_allocator;
    col->c_id = GOIMG_COLOR_CMYK;
    col->color = im_xcalloc(im_std_allocator, 1, sizeof(uint32_t));
    col->size = sizeof(uint32_t);
    col->rgba128 = im_cmyk_convert_rgba128;
}

inline Color_t im_newcolor_cmyk(void)
{
    Color_t col;
    im_initcolor_cmyk(&col);
    return col;
}

inline void im_initimg_cmyk(Image_t *img, int w, int h, Allocator_t *allocator)
{
    size_t size = w * h * sizeof(uint32_t);
    img->allocator = allocator ? allocator : im_std_allocator;
    img->img = im_xalloc(allocator, size);
    img->size = size;
    img->w = w;
    img->h = h;
    img->color_model = im_colormodel_cmyk;
    img->at = im_cmyk_at;
    img->set = im_cmyk_set;
}

inline Image_t im_newimg_cmyk(int w, int h, Allocator_t *allocator)
{
    Image_t img;
    im_initimg_cmyk(&img, w, h, allocator);
    return img;
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
