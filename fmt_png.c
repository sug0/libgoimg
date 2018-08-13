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
    size_t r = 0;
    int n;

    do {
        n = s->rf(s->src, (char *)(buf + r), (int)size);

        if (unlikely(n < 0))
            png_error(png_ptr, "read function error");

        size -= n;
        r += n;
    } while (size > 0);
}

static void _png_write_fn(png_structp png_ptr, png_bytep buf, png_size_t size)
{
    struct _png_state_w *s = (struct _png_state_w *)png_get_io_ptr(png_ptr);

    if (_err_write(s->wf, s->dst, (char *)buf, (int)size))
        png_error(png_ptr, "write function error");
}

int im_png_dec(Image_t *img, rfun_t rf, void *src)
{
    int err = 0;

    png_bytepp row_pointers = NULL;
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;

    /* initialize write */
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    _im_maybe_jmp_err(png_ptr);

    /* initialize info */
    info_ptr = png_create_info_struct(png_ptr);
    _im_maybe_jmp_err(info_ptr);

    /* create jump point to fall back to
     * when an error occurs while decoding png */
    _im_maybe_jmp_err(!setjmp(png_jmpbuf(png_ptr)));

    /* set the read method -- wrap rf */
    struct _png_state_r s = {rf, src};
    png_set_read_fn(png_ptr, &s, _png_read_fn);

    /* read file info -- width, height, etc */
    png_read_info(png_ptr, info_ptr);

    int color_type, bit_depth, pix_width, row_bytes;

    img->w = png_get_image_width(png_ptr, info_ptr);
    img->h = png_get_image_height(png_ptr, info_ptr);

    color_type = png_get_color_type(png_ptr, info_ptr);
    bit_depth = png_get_bit_depth(png_ptr, info_ptr);

    /* determine amount of memory to allocate,
     * based on the color_type and bit_depth vars */
    switch (color_type) {
    case PNG_COLOR_TYPE_GRAY:
        switch (bit_depth) {
        case 1:
        case 2:
        case 4:
            png_set_expand_gray_1_2_4_to_8(png_ptr);
        case 8:
            img->color_model = im_colormodel_gray;
            img->at = im_gray_at;
            img->set = im_gray_set;
            break;
        case 16:
            img->color_model = im_colormodel_gray16;
            img->at = im_gray16_at;
            img->set = im_gray16_set;
            break;
        default:
            _im_maybe_jmp_err(0);
        }
        break;
    case PNG_COLOR_TYPE_RGB:
        switch (bit_depth) {
        case 8:
            png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
            break;
        case 16:
            png_set_filler(png_ptr, 0xffff, PNG_FILLER_AFTER);
            break;
        default:
            _im_maybe_jmp_err(0);
        }
    case PNG_COLOR_TYPE_RGB_ALPHA:
        switch (bit_depth) {
        case 8:
            img->color_model = im_colormodel_nrgba;
            img->at = im_nrgba_at;
            img->set = im_nrgba_at;
            break;
        case 16:
            img->color_model = im_colormodel_nrgba64;
            img->at = im_nrgba64_at;
            img->set = im_nrgba64_at;
            break;
        default:
            _im_maybe_jmp_err(0);
        }
        break;

    /* TODO: implement missing color spaces */
    case PNG_COLOR_TYPE_GRAY_ALPHA: /* png_set_gray_to_rgb */
    case PNG_COLOR_TYPE_PALETTE:
    default:
        _im_maybe_jmp_err(0);
    }

    /* remaining info stuff */
    png_read_update_info(png_ptr, info_ptr);

    /* alloc image */
    row_bytes = png_get_rowbytes(png_ptr, info_ptr);
    pix_width = row_bytes/img->w;
    img->size = row_bytes * img->h;
    img->img = im_xcalloc(img->allocator, 1, img->size);

    /* initialize row pointers */
    int y;
    png_bytep imgdata = img->img;
    row_pointers = im_xalloc(im_std_allocator, img->h * sizeof(png_bytepp));

    for (y = 0; y < img->h; y++)
        row_pointers[y] = imgdata + y*img->w*pix_width;

    /* perform actual decoding */
    png_read_image(png_ptr, row_pointers);

    /* finish decoding */
    png_read_end(png_ptr, info_ptr);

done:
    if (likely(info_ptr)) png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
    if (likely(png_ptr)) png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    if (likely(row_pointers)) free(row_pointers);

    return err;
}

int im_png_enc(Image_t *img, wfun_t wf, void *dst)
{
    int err = 0;

    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;

    /* initialize write */
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    _im_maybe_jmp_err(png_ptr);

    /* initialize info */
    info_ptr = png_create_info_struct(png_ptr);
    _im_maybe_jmp_err(info_ptr);

    /* create jump point when
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
    } else if (img->color_model == im_colormodel_gray16) {
        color_type = PNG_COLOR_TYPE_GRAY;
        bit_depth = 16;
        pix_width = sizeof(uint16_t);
    } else {
        /* do lossy conversion */
        goto lossy;
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
    png_bytep imgdata = img->img;

    for (y = 0; y < img->h; y++)
        png_write_row(png_ptr, imgdata + y*img->w*pix_width);

    goto done;

lossy:
    /* write header */
    png_set_IHDR(png_ptr, info_ptr,
                 img->w, img->h,
                 8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    /* write metadata stuff */
    png_write_info(png_ptr, info_ptr);

    int x;
    uint32_t *row = im_xalloc(im_std_allocator, sizeof(uint32_t) * img->w);

    Color_t c_src = im_newcolor_from_img(img),
            c_dst = im_newcolor_nrgba();

    for (y = 0; y < img->h; y++) {
        for (x = 0; x < img->w; x++) {
            img->at(img, x, y, &c_src);
            im_colormodel_nrgba(&c_dst, &c_src);
            row[x] = *(uint32_t *)c_dst.color;
        }
        png_write_row(png_ptr, (png_bytep)row);
    }

    free(row);
    free(c_src.color);
    free(c_dst.color);

done:
    /* write final chunk */
    png_write_end(png_ptr, NULL);

    if (likely(info_ptr)) png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
    if (likely(png_ptr)) png_destroy_write_struct(&png_ptr, &info_ptr);

    return err;
}

/* -------------------------------------------------------------------------- */

static ImageFormat_t _im_fmt_png = {
    .magic = "\x89PNG\r\n\x1a\n",
    .magic_size = 8,
    .name = "PNG",
    .decode = im_png_dec,
    .encode = im_png_enc,
};

inline void im_register_format_png(void)
{
    im_register_format(&_im_fmt_png);
}
