#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <goimg/goimg.h>

#define vu32(V, I)  (((uint32_t *)(V))[I])

/*  x1,y1---[x0,y0]
 *    |        |
 *    |        |
 *    |        |
 * [x2,y2]---x3,y3
 * */
struct rect {
    int x0, y0;
    int x2, y2;
};

static void im_cut(Image_t *dst, Image_t *const src, struct rect *const r);

int main(void)
{
    int err = 0;

    Image_t newimg = {.img = NULL, .allocator = im_std_allocator},
            img = {.img = NULL, .allocator = im_std_allocator};

    /* load formats */
    im_load_defaults();

    /* decode image from stdin */
    if (!im_decode(&img, fdread, GOIO_FD(0))) {
        err = 1;
        goto done;
    }

    /* perform a 200x200 square cut,
     * on the upper left corner of the picture */
    struct rect sq = {
        .x0 = 200, .x2 = 0,
        .y0 = 0,   .y2 = 200,
    };
    im_cut(&newimg, &img, &sq);

    /* encode image to stdout as PNG */
    if (im_encode(&newimg, "PNG", fdwrite, GOIO_FD(1)) < 0)
        err = 1;

done:
    im_xfree(im_std_allocator, img.img);
    im_xfree(im_std_allocator, newimg.img);

    return err;
}

void im_cut_slow(Image_t *dst, Image_t *const src, struct rect *const r)
{
    int x, y;

    Color_t c_src = im_newcolor_from_img(src),
            c_dst = im_newcolor_nrgba();

    im_initimg_nrgba(dst, src->w, src->h, im_std_allocator);

    for (y = r->y0; y <= r->y2; y++) {
        for (x = r->x2; x <= r->x0; x++) {
            src->at(src, x, y, &c_src);
            im_colormodel_nrgba(&c_dst, &c_src);
            dst->set(dst, x, y, &c_dst);
        }
    }

    im_xfree(im_std_allocator, c_src.color);
    im_xfree(im_std_allocator, c_dst.color);
}

void im_cut_fast(Image_t *dst, Image_t *const src, struct rect *const r)
{
    int y, i, row = (r->x0 - r->x2) * sizeof(uint32_t);

    im_initimg_nrgba(dst, src->w, src->h, im_std_allocator);

    /* copy image rows */
    for (y = r->y0; y <= r->y2; y++) {
        i = src->w*y + r->x2;
        memcpy(&vu32(dst->img, i), &vu32(src->img, i), row);
    }
}

inline void im_cut(Image_t *dst, Image_t *const src, struct rect *const r)
{
    if (src->color_model == im_colormodel_nrgba) {
        im_cut_fast(dst, src, r);
        return;
    }
    im_cut_slow(dst, src, r);
}
