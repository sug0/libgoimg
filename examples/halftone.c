#include <goimg/goimg.h>

struct mask {
    int w, h;
    uint16_t m;
    uint8_t *pix;
};

uint8_t bayer_pix[] = {
     1,  9,  3, 11,
    13,  5, 15,  7,
     4, 12,  2, 10,
    16,  8, 14,  6,
};

struct mask bayer_mask = {
    .w = 4, .h = 4,
    .m = 16, .pix = bayer_pix,
};

inline uint8_t pix_threshold(uint16_t pix)
{
    return (pix > 255) ? 255 : 0;
}

inline void clamp(Image_t *img, int x, int y, int *cx, int *cy)
{
    *cx = (x >= img->w) ? img->w - 1 : x;
    *cy = (y >= img->h) ? img->h - 1 : y;
}

void apply_mask(struct mask *mask, int x, int y,
                Image_t *i_dst, Color_t *c_dst, Image_t *i_src, Color_t *c_src)
{
    int i, j, cx, cy;
    uint16_t pix;

    for (i = 0; i < mask->h; i++) {
        for (j = 0; j < mask->w; j++) {
            clamp(i_src, x+j, y+i, &cx, &cy);
            i_src->at(i_src, cx, cy, c_src);
            im_colormodel_gray(c_dst, c_src);
            pix = *(uint8_t *)c_dst->color + mask->pix[i*mask->w + j]*mask->m;
            *(uint8_t *)c_dst->color = pix_threshold(pix);
            i_dst->set(i_dst, cx, cy, c_dst);
        }
    }
}

int main(void)
{
    int x, y, err = 0;

    Image_t img_src = {.img = NULL, .allocator = im_std_allocator},
            img_dst = {.img = NULL, .allocator = im_std_allocator};

    Color_t c_src = {.color = NULL, .allocator = im_std_allocator},
            c_dst = {.color = NULL, .allocator = im_std_allocator};

    im_load_defaults();

    if (!im_decode(&img_src, fdread, GOIO_FD(0))) {
        err = 1;
        goto done;
    }

    img_dst = im_newimg_gray(img_src.w, img_src.h, im_std_allocator);
    c_src = im_newcolor_from_img(&img_src);
    c_dst = im_newcolor_gray();

    for (y = 0; y < img_src.h; y += bayer_mask.h)
        for (x = 0; x < img_src.w; x += bayer_mask.w)
            apply_mask(&bayer_mask, x, y,
                       &img_dst, &c_dst,
                       &img_src, &c_src);

    if (im_encode(&img_dst, "PNG", fdwrite, GOIO_FD(1)) < 0)
        err = 1;

done:
    im_xfree(im_std_allocator, c_src.color);
    im_xfree(im_std_allocator, c_dst.color);
    im_xfree(im_std_allocator, img_src.img);
    im_xfree(im_std_allocator, img_dst.img);

    return err;
}
