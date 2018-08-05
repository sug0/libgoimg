#include <stdlib.h>
#include <string.h>

#include "color.h"
#include "fmt_farbfeld.h"
#include "util.h"

struct _s_bufwriter {
    void *buf;
    int avail;
};

static int _s_bufwrite(void *dst, char *buf, int size)
{
    struct _s_bufwriter *s = (struct _s_bufwriter *)dst;

    if (unlikely(!s->avail))
        return 0;

    if (unlikely(size > s->avail))
        size = s->avail;

    memcpy(s->buf, buf, size);
    s->avail -= size;
    s->buf += size;

    return size;
}

int im_farbfeld_dec(Image_t *img, rfun_t rf, void *src)
{
    char magic[16];

    /* read the magic */
    if (unlikely(rf(src, magic, 16) < 0))
        return -1;

    /* read dims */
    img->w = ntohl(*(uint32_t *)(magic + 8));
    img->h = ntohl(*(uint32_t *)(magic + 12));

    img->size = img->w * img->h * sizeof(uint64_t);
    img->img = _xalloc(img->alloc, img->size);
    img->color_model = im_colormodel_nrgba64;

    struct _s_bufwriter s = {img->img, img->size};

    return (unlikely(rwcpy(_s_bufwrite, &s, rf, src) < 0)) ? -1 : 0;
}

int im_farbfeld_enc(Image_t *img, ImageFormat_t *fmt, wfun_t wf, void *dst)
{
    /* write the magic */
    if (unlikely(wf(dst, "farbfeld", 8) < 0))
        return -1;

    /* write dimensions */
    uint32_t dim;

    dim = htonl((uint32_t)img->w);
    if (unlikely(wf(dst, (char *)&dim, sizeof(uint32_t)) < 0))
        return -1;

    dim = htonl((uint32_t)img->h);
    if (unlikely(wf(dst, (char *)&dim, sizeof(uint32_t)) < 0))
        return -1;

    /*
     * write the pixel data
     * */

    if (likely(img->color_model == im_colormodel_nrgba64))
        return (unlikely(wf(dst, (char *)img->img, img->size) < 0)) ? -1 : 0;

    /* lossy */
    int x, y, err = 0;
    Color_t c_src = {.color = NULL, .alloc = malloc, .free = free},
            c_dst = {.color = NULL, .alloc = malloc, .free = free};

    for (y = 0; y < img->h; y++) {
        for (x = 0; x < img->w; x++) {
            fmt->at(img, x, y, &c_src);
            im_colormodel_nrgba64(&c_dst, &c_src);

            if (unlikely(wf(dst, (char *)c_dst.color, sizeof(uint64_t)) < 0)) {
                err = -1;
                goto done;
            }
        }
    }

done:
    if (likely(c_src.color))
        free(c_src.color);
    if (likely(c_dst.color))
        free(c_dst.color);

    return err;
}

void im_farbfeld_at(Image_t *img, int x, int y, Color_t *dst)
{
    if (unlikely(!dst->color || (dst->color && dst->size < sizeof(uint64_t)))) {
        if (dst->color)
            dst->free(dst->color);
        dst->color = _xalloc(dst->alloc, sizeof(uint64_t));
        dst->size = sizeof(uint64_t);
    }

    if (unlikely(dst->c_id != GOIMG_COLOR_NRGBA64))
        dst->c_id = GOIMG_COLOR_NRGBA64;

    *(uint64_t *)dst->color = ((uint64_t *)img->img)[y * img->w + x];
}

void im_farbfeld_set(Image_t *img, int x, int y, Color_t *src)
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
