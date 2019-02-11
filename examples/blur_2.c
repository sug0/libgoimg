#include <stdio.h>
#include <stdlib.h>
#include <goimg/goimg.h>

/* blur stuff */
typedef float kernel5x5_t[5][5];

static void im_blur(Image_t *dst, Image_t *const src);
static uint32_t pix_get_nrgba(Image_t *img, Color_t *col, int x, int y);
static uint32_t convolve(Image_t *img, Color_t *col, kernel5x5_t kernel, int x, int y);

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

    im_blur(&newimg, &img);

    /* encode image to stdout as PNG */
    if (im_encode(&newimg, "PNG", fdwrite, GOIO_FD(1)) < 0)
        err = 1;

done:
    im_xfree(im_std_allocator, img.img);
    im_xfree(im_std_allocator, newimg.img);

    return err;
}

void im_blur(Image_t *dst, Image_t *const src)
{
    static kernel5x5_t blur_kern = {
        {1/256.0, 4/256.0,  6/256.0,  4/256.0,  1/256.0},
        {4/256.0, 16/256.0, 24/256.0, 16/256.0, 4/256.0},
        {6/256.0, 24/256.0, 36/256.0, 24/256.0, 6/256.0},
        {4/256.0, 16/256.0, 24/256.0, 16/256.0, 4/256.0},
        {1/256.0, 4/256.0,  6/256.0,  4/256.0,  1/256.0},
    };

    int x, y;

    Color_t col = im_newcolor_nrgba();
    im_initimg_nrgba(dst, src->w, src->h, im_std_allocator);

    for (y = 0; y < src->h; y++) {
        for (x = 0; x < src->w; x++) {
            *(uint32_t *)col.color = convolve(src, &col, blur_kern, x, y);
            dst->set(dst, x, y, &col);
        }
    }

    im_xfree(im_std_allocator, col.color);
}

inline uint32_t pix_get_nrgba(Image_t *img, Color_t *col, int x, int y)
{
    uint32_t clamp = 0xffffffff;

    if (x < 0) {
        clamp = 0x00ffffff;
        x = 0;
    } else if (x >= img->w) {
        clamp = 0x00ffffff;
        x = img->w - 1;
    }

    if (y < 0) {
        clamp = 0x00ffffff;
        y = 0;
    } else if (y >= img->h) {
        clamp = 0x00ffffff;
        y = img->h - 1;
    }

    img->at(img, x, y, col);

    return *(uint32_t *)col->color & clamp;
}

uint32_t convolve(Image_t *img, Color_t *col, kernel5x5_t kernel, int x, int y)
{
    typedef struct {
        uint8_t r, g, b, a;
    } _nrgba_t;

    int kx, ky;
    float mult;
    uint32_t pix_dat;
    _nrgba_t *pix = (_nrgba_t *)&pix_dat, accum = {0, 0, 0, 0};

    pix_dat = pix_get_nrgba(img, col, x, y);
    accum.a = pix->a;

    for (ky = -2; ky < 3; ky++) {
        for (kx = -2; kx < 3; kx++) {
            mult = kernel[kx + 2][ky + 2];
            pix_dat = pix_get_nrgba(img, col, x + kx, y + ky);
            accum.r += pix->r * mult;
            accum.g += pix->g * mult;
            accum.b += pix->b * mult;
        }
    }

    return *(uint32_t *)&accum;
}
