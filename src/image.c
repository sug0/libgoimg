#include <string.h>

#include "image.h"
#include "color.h"
#include "util.h"
#include "allocator.h"

/* compile with farbfeld support */
#ifdef GOIMG_COMPILE_FMT_FARBFELD
#include "fmt_farbfeld.h"
#endif

/* compile with png support -- use libpng */
#ifdef GOIMG_COMPILE_FMT_PNG
#include "fmt_png.h"
#endif

/* compile with jpeg support -- use libjpeg-turbo */
#ifdef GOIMG_COMPILE_FMT_JPEG
#include "fmt_jpeg.h"
#endif

/* this variable is used to register new color formats */
static int _color_id_counter = GOIMG_NO_DEF_COLORS - 1;

/* this array is used to store new image formats */
static ImageFormat_t _img_formats[GOIMG_NO_FMTS];
static int _img_format_i = 0;

inline void im_load_defaults(void)
{
#ifdef GOIMG_COMPILE_FMT_FARBFELD
    im_register_format_farbfeld();
#endif
#ifdef GOIMG_COMPILE_FMT_PNG
    im_register_format_png();
#endif
#ifdef GOIMG_COMPILE_FMT_JPEG
    im_register_format_jpeg();
#endif
}

inline void im_register_format(ImageFormat_t *fmt)
{
    _img_formats[_img_format_i++] = *fmt;
}

ImageFormat_t *im_get_format(char *fmt)
{
    int i;

    for (i = 0; i < _img_format_i; i++)
        if (strcmp(_img_formats[i].name, fmt) == 0)
            return &_img_formats[i];

    return NULL;
}

inline int im_register_color(void)
{
    return ++_color_id_counter;
}

ImageFormat_t *im_decode(Image_t *img, rfun_t rf, void *src)
{
    char buf[16384];
    BufferedReader_t br = {
        .rf = rf,
        .src = src,
        .r = 0,
        .w = 0,
        .buf = buf,
        .bufsz = sizeof(buf)
    };

    int i, msize, mbufsz;
    char *magic, *mbuf = NULL;
    ImageFormat_t *fmt = NULL;

    for (i = 0; i < _img_format_i; i++) {
        magic = _img_formats[i].magic;
        msize = _img_formats[i].magic_size;

        if (!mbuf) {
            mbuf = im_xalloc(im_std_allocator, msize);
            mbufsz = msize;
        } else if (mbufsz < msize) {
            mbuf = im_xrealloc(im_std_allocator, mbuf, msize);
            mbufsz = msize;
        }

        if (rbufpeek(&br, mbuf, msize) < msize)
            continue;

        if (_m_match(magic, msize, mbuf, msize)) {
            if (_img_formats[i].decode(img, rbufread, &br) == 0)
                fmt = &_img_formats[i];
            break;
        }
    }
    im_xfree(im_std_allocator, mbuf);

    return fmt;
}

int im_encode(Image_t *img, char *fmt, wfun_t wf, void *dst)
{
    int i;

    for (i = 0; i < _img_format_i; i++)
        if (strcmp(_img_formats[i].name, fmt) == 0)
            return _img_formats[i].encode(img, wf, dst);

    return -1;
}

inline void im_cpy(Image_t *dst, Image_t *src)
{
    if (unlikely(!dst->img || (dst->img && dst->size < src->size))) {
        if (dst->img)
            im_xfree(dst->allocator, dst->img);
        dst->img = im_xalloc(dst->allocator, src->size);
    }

    memcpy(dst->img, src->img, src->size);
    dst->size = src->size;
    dst->w = src->w;
    dst->h = src->h;
    dst->color_model = src->color_model;
    dst->at = src->at;
    dst->set = src->set;
}

inline void im_initimg(Image_t *img, int w, int h, cmfun_t color_model, Allocator_t *allocator)
{
    if (color_model == im_colormodel_nrgba)
        im_initimg_nrgba(img, w, h, allocator);
    else if (color_model == im_colormodel_nrgba64)
        im_initimg_nrgba64(img, w, h, allocator);
    else if (color_model == im_colormodel_rgb)
        im_initimg_rgb(img, w, h, allocator);
    else if (color_model == im_colormodel_gray)
        im_initimg_gray(img, w, h, allocator);
    else if (color_model == im_colormodel_gray16)
        im_initimg_gray16(img, w, h, allocator);
    else if (color_model == im_colormodel_cmyk)
        im_initimg_cmyk(img, w, h, allocator);
    else
        im_initimg_nrgba(img, w, h, allocator);
}

inline Image_t im_newimg(int w, int h, cmfun_t color_model, Allocator_t *allocator)
{
    Image_t img;
    im_initimg(&img, w, h, color_model, allocator);
    return img;
}
