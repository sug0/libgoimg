#include <stdlib.h>
#include <string.h>

#include <png.h>
#include "color.h"
#include "fmt_png.h"
#include "util.h"

#define _im_maybe_jmp_err(CHK)  if (!(CHK)) {err = -1; goto done;}

struct _png_state {
    wfun_t wf;
    void *dst;
};

static void _png_write_fn(png_structp png_ptr, png_bytep buf, png_size_t size)
{
    struct _png_state *s = (struct _png_state *)png_get_io_ptr(png_ptr);

    if (s->wf(s->dst, (char *)buf, (int)size) < 0)
        png_error(png_ptr, "write function error");
}

/* TODO */
int im_png_dec(Image_t *img, rfun_t rf, void *src)
{
    return 0;
}

int im_png_enc(Image_t *img, wfun_t wf, void *dst)
{
    int err = 0;

    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;

    Color_t c_src = {.color = NULL, .alloc = malloc, .free = free},
            c_dst = {.color = NULL, .alloc = malloc, .free = free};

    /* initialize write */
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    _im_maybe_jmp_err(png_ptr);

    /* initialize info */
    info_ptr = png_create_info_struct(png_ptr);
    _im_maybe_jmp_err(info_ptr);

    /* create jump joint when
     * an error occurs during creation of png */
    _im_maybe_jmp_err(!setjmp(png_jmpbuf(png_ptr)));

    /* set the write method -- wrap wf */
    struct _png_state s = {wf, dst};
    png_set_write_fn(png_ptr, &s, _png_write_fn, NULL);

    /* determine color type and bit depth */
    int color_type, bit_depth;

    if (img->color_model == im_colormodel_nrgba) {
        color_type = PNG_COLOR_TYPE_RGB_ALPHA;
        bit_depth = 8;
    } else if (img->color_model == im_colormodel_nrgba64) {
        color_type = PNG_COLOR_TYPE_RGB_ALPHA;
        bit_depth = 16;
    } else if (img->color_model == im_colormodel_gray) {
        color_type = PNG_COLOR_TYPE_GRAY;
        bit_depth = 8;
    } else {
        /* TODO: lossy conversion */
        _im_maybe_jmp_err(0);
        // goto lossy;
    }

    /* write header */
    png_set_IHDR(png_ptr, info_ptr,
                 img->w, img->h,
                 bit_depth, color_type, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    /* write metadata stuff */
    png_write_info(png_ptr, info_ptr);

    /* write rows */
    int y;

    for (y = 0; y < img->h; y++)
        png_write_row(png_ptr, img->img + y*img->w);

    /* write final chunk */
    png_write_end(png_ptr, NULL);
    //goto done;

//lossy:
//    int x, y, err = 0;
//    Color_t c_src = {.color = NULL, .alloc = malloc, .free = free},
//            c_dst = {.color = NULL, .alloc = malloc, .free = free};
//
//    for (y = 0; y < img->h; y++) {
//        for (x = 0; x < img->w; x++) {
//            img->at(img, x, y, &c_src);
//            im_colormodel_nrgba64(&c_dst, &c_src);
//
//            if (unlikely(wf(dst, (char *)c_dst.color, sizeof(uint64_t)) < 0)) {
//                err = -1;
//                goto done;
//            }
//        }
//    }
//
done:
    if (likely(c_src.color)) free(c_src.color);
    if (likely(c_dst.color)) free(c_dst.color);
    if (likely(info_ptr)) png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
    if (likely(png_ptr)) png_destroy_write_struct(&png_ptr, &info_ptr);

    return err;
}
