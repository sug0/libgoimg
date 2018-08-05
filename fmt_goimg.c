#include <stdlib.h>
#include <string.h>

#include "color_rgba.h"
#include "fmt_goimg.h"
#include "util.h"

struct _s_bufwriter {
    void *buf;
    int avail;
};

static int _s_bufwrite(void *dst, char *buf, int size)
{
    struct _s_bufwriter *s = (struct _s_bufwriter *)dst;

    if (!s->avail)
        return 0;

    if (size > s->avail)
        size = s->avail;

    memcpy(s->buf, buf, size);
    s->avail -= size;
    s->buf += size;

    return size;
}

int im_goimg_dec(Image_t *img, rfun_t rf, void *src)
{
    /* read the magic */
    if (rf(src, (char *)&img->size, 4) < 0)
        return -1;

    /* read dims */
    uint32_t dim;

    if (rf(src, (char *)&img->w, sizeof(uint32_t)) < 0)
        return -1;
    img->w = dim;

    if (rf(src, (char *)&img->h, sizeof(uint32_t)) < 0)
        return -1;
    img->h = dim;

    img->size = img->w * img->h * 4;
    img->img = _xalloc(img->alloc, img->size);

    struct _s_bufwriter s = {img->img, img->size};

    return (rwcpy(_s_bufwrite, &s, rf, src) < 0) ? -1 : 0;
}

int im_goimg_enc(Image_t *img, ImageFormat_t *fmt, wfun_t wf, void *dst)
{
    /* write the magic */
    if (wf(dst, "\x06\x00\x10\x00", 4) < 0)
        return -1;

    /* write dimensions */
    uint32_t dim;

    dim = img->w;
    if (wf(dst, (char *)&dim, sizeof(uint32_t)) < 0)
        return -1;

    dim = img->h;
    if (wf(dst, (char *)&dim, sizeof(uint32_t)) < 0)
        return -1;

    /*
     * write the pixel data
     * */

    if (fmt->color_model == im_colormodel_rgba)
        return (wf(dst, (char *)img->img, img->size) < 0) ? -1 : 0;

    /* lossy */
    int x, y, err = 0;
    Color_t c_src = {.color = NULL, .alloc = malloc, .free = free},
            c_dst = {.color = NULL, .alloc = malloc, .free = free};

    for (y = 0; y < img->h; y++) {
        for (x = 0; x < img->w; x++) {
            fmt->at(img, x, y, &c_src);
            im_colormodel_rgba(&c_dst, &c_src);

            if (wf(dst, (char *)c_dst.color, 4) < 0) {
                err = -1;
                goto done;
            }
        }
    }

done:
    if (c_src.color)
        free(c_src.color);
    if (c_dst.color)
        free(c_dst.color);

    return err;
}

void im_goimg_at(Image_t *img, int x, int y, Color_t *dst)
{
    if (!dst->color || (dst->color && dst->size != sizeof(uint32_t))) {
        if (dst->color)
            dst->free(dst->color);
        dst->color = _xalloc(dst->alloc, sizeof(uint32_t));
        dst->size = sizeof(uint32_t);
    }
    if (dst->c_id != GOIMG_COLOR_RGBA)
        dst->c_id = GOIMG_COLOR_RGBA;
    *(uint32_t *)dst->color = ((uint32_t *)img->img)[y * img->w + x];
}

void im_goimg_set(Image_t *img, int x, int y, Color_t *src)
{
}
