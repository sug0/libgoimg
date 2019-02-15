#include <math.h>
#include <goimg/goimg.h>

static void im_swirl(Image_t *dst, Image_t *const src, double amt, double size);

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

    /* swirl image pixels! */
    im_swirl(&newimg, &img, 60, 1000);

    /* encode image to stdout as PNG */
    if (im_encode(&newimg, "PNG", fdwrite, GOIO_FD(1)) < 0)
        err = 1;

done:
    im_xfree(im_std_allocator, img.img);
    im_xfree(im_std_allocator, newimg.img);

    return err;
}

inline void distort(int *u, int *v, int x, int y, int cx, int cy, double amt, double size)
{
    const double x0 = (double)(x - cx);
    const double y0 = (double)(y - cy);

    const double r = amt * exp(-(x0*x0+y0*y0)/(size*size));
    const double s = sin(r);
    const double c = cos(r);

    *u = (int)(c*x0 + s*y0) + cx;
    *v = (int)(-s*x0 + c*y0) + cy;
}

inline void clamp(int *u, int *v, int w, int h)
{
    if (*u < 0)  *u = 0;
    if (*u >= w) *u = w - 1;
    if (*v < 0)  *v = 0;
    if (*v >= h) *v = h - 1;
}

void im_swirl(Image_t *dst, Image_t *const src, double amt, double size)
{
    int x, y, u, v;

    Color_t c_src = im_newcolor_from_img(src),
            c_dst = im_newcolor_nrgba();

    im_initimg_nrgba(dst, src->w, src->h, im_std_allocator);

    const int cx = src->w/2;
    const int cy = src->h/2;

    for (y = 0; y < src->h; y++) {
        for (x = 0; x < src->w; x++) {
            distort(&u, &v, x, y, cx, cy, amt, size);
            clamp(&u, &v, src->w, src->h);
            src->at(src, u, v, &c_src);
            im_colormodel_nrgba(&c_dst, &c_src);
            dst->set(dst, x, y, &c_dst);
        }
    }

    im_xfree(im_std_allocator, c_src.color);
    im_xfree(im_std_allocator, c_dst.color);
}
