#include <stdlib.h>
#include <string.h>

#include <png.h>
#include "color.h"
#include "fmt_png.h"
#include "util.h"

#define _im_maybe_jmp_err(CHK)  if (!(CHK)) {err = -1; goto done;}

struct _png_state_r {
    rfun_t rf;
    void *src;
};

struct _png_state_w {
    wfun_t wf;
    void *dst;
};

static void _png_read_fn(png_structp png_ptr, png_bytep buf, png_size_t size)
{
    struct _png_state_r *s = (struct _png_state_r *)png_get_io_ptr(png_ptr);

    if (s->rf(s->src, (char *)buf, (int)size) < 0)
        png_error(png_ptr, "write function error");
}

static void _png_write_fn(png_structp png_ptr, png_bytep buf, png_size_t size)
{
    struct _png_state_w *s = (struct _png_state_w *)png_get_io_ptr(png_ptr);

    if (s->wf(s->dst, (char *)buf, (int)size) < 0)
        png_error(png_ptr, "write function error");
}

int im_png_dec(Image_t *img, rfun_t rf, void *src)
{
    int err = 0;

    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;

    /* initialize write */
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    _im_maybe_jmp_err(png_ptr);

    /* initialize info */
    info_ptr = png_create_info_struct(png_ptr);
    _im_maybe_jmp_err(info_ptr);

    /* create jump joint to fall back to
     * when an error occurs while reading the png info */
    _im_maybe_jmp_err(!setjmp(png_jmpbuf(png_ptr)));

    /* set the read method -- wrap rf */
    struct _png_state_r s = {rf, src};
    png_set_read_fn(png_ptr, &s, _png_read_fn);

    /* read file info -- width, height, etc */
    png_read_info(png_ptr, info_ptr);

    int color_type, bit_depth, pix_width, passes;

    img->w = png_get_image_width(png_ptr, info_ptr);
    img->h = png_get_image_height(png_ptr, info_ptr);

    color_type = png_get_color_type(png_ptr, info_ptr);
    bit_depth = png_get_bit_depth(png_ptr, info_ptr);
    passes = png_set_interlace_handling(png_ptr);

    png_read_update_info(png_ptr, info_ptr);

    /* create jump joint to fall back to
     * when an error occurs while decoding the pix rows */
    _im_maybe_jmp_err(!setjmp(png_jmpbuf(png_ptr)));

    /* determine amount of memory to allocate,
     * based on the color_type and bit_depth vars */
    switch (color_type) {
    case PNG_COLOR_TYPE_GRAY:
        switch (bit_depth) {
        case 1:
        case 2:
        case 4:
            png_set_gray_1_2_4_to_8(png_ptr);
        case 8:
            img->size = img->w * img->h * sizeof(uint8_t);
            img->img = _xalloc(img->alloc, img->size);
            img->color_model = im_colormodel_gray;
            img->at = im_gray_at;
            img->set = im_gray_set;
            pix_width = sizeof(uint8_t);
            break;

        /* TODO: implement gray16 */
        case 16:
        default:
            _im_maybe_jmp_err(0);
        }
        break;
    case PNG_COLOR_TYPE_RGB_ALPHA:
        switch (bit_depth) {
        case 8:
            img->size = img->w * img->h * sizeof(uint32_t);
            img->img = _xalloc(img->alloc, img->size);
            img->color_model = im_colormodel_nrgba;
            img->at = im_nrgba_at;
            img->set = im_nrgba_at;
            pix_width = sizeof(uint32_t);
            break;
        case 16:
            img->size = img->w * img->h * sizeof(uint64_t);
            img->img = _xalloc(img->alloc, img->size);
            img->color_model = im_colormodel_nrgba64;
            img->at = im_nrgba64_at;
            img->set = im_nrgba64_at;
            pix_width = sizeof(uint64_t);
            break;
        default:
            _im_maybe_jmp_err(0);
        }
        break;

    /* TODO: implement missing color spaces */
    case PNG_COLOR_TYPE_RGB:
    case PNG_COLOR_TYPE_GRAY_ALPHA: /* png_set_gray_to_rgb */
    case PNG_COLOR_TYPE_PALETTE:
    default:
        _im_maybe_jmp_err(0);
    }

    /* zero out structure to make valgrind stfu */
    memset(img->img, 0, img->size);

    /* perform actual decoding */
    int y, pass;

    for (pass = 0; pass < passes; pass++) {
        for (y = 0; y < img->h; y++) {
            png_read_row(png_ptr, img->img + y*img->w*pix_width, NULL);
        }
    }

    /* finish decoding */
    png_read_end(png_ptr, info_ptr);

done:
    if (likely(info_ptr)) png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
    if (likely(png_ptr)) png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

    return err;
}

int im_png_enc(Image_t *img, wfun_t wf, void *dst)
{
    int err = 0;

    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;

    //Color_t c_src = {.color = NULL, .alloc = malloc, .free = free},
    //        c_dst = {.color = NULL, .alloc = malloc, .free = free};

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
    struct _png_state_w s = {wf, dst};
    png_set_write_fn(png_ptr, &s, _png_write_fn, NULL);

    /* determine color type and bit depth */
    int color_type, bit_depth, pix_width;

    if (img->color_model == im_colormodel_nrgba) {
        color_type = PNG_COLOR_TYPE_RGB_ALPHA;
        bit_depth = 8;
        pix_width = sizeof(uint32_t);
    } else if (img->color_model == im_colormodel_nrgba64) {
        color_type = PNG_COLOR_TYPE_RGB_ALPHA;
        bit_depth = 16;
        pix_width = sizeof(uint64_t);
    } else if (img->color_model == im_colormodel_gray) {
        color_type = PNG_COLOR_TYPE_GRAY;
        bit_depth = 8;
        pix_width = sizeof(uint8_t);
    } else {
        /* TODO: lossy conversion */
        _im_maybe_jmp_err(0);
        // goto lossy;
    }

    /* write header */
    png_set_IHDR(png_ptr, info_ptr,
                 img->w, img->h,
                 bit_depth, color_type, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    /* write metadata stuff */
    png_write_info(png_ptr, info_ptr);

    /* write rows */
    int y;

    for (y = 0; y < img->h; y++)
        png_write_row(png_ptr, img->img + y*img->w*pix_width);

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
    //if (likely(c_src.color)) free(c_src.color);
    //if (likely(c_dst.color)) free(c_dst.color);
    if (likely(info_ptr)) png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
    if (likely(png_ptr)) png_destroy_write_struct(&png_ptr, &info_ptr);

    return err;
}
