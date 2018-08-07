#include <string.h>

#include "image.h"
#include "color.h"
#include "util.h"

#include "fmt_farbfeld.h"
#include "fmt_png.h"

/* this variable is used to register new color formats */
static int _color_id_counter = GOIMG_NO_DEF_COLORS - 1;

/* this array is used to store new image formats */
static ImageFormat_t _img_formats[GOIMG_NO_FMTS] = {
    /* farbfeld */
    [0] = {
        .magic = "farbfeld????????",
        .magic_size = 16,
        .name = "farbfeld",
        .decode = im_farbfeld_dec,
        .encode = im_farbfeld_enc,
    },
    /* PNG */
    [1] = {
        .magic = "\x89PNG\r\n\x1a\n",
        .magic_size = 8,
        .name = "PNG",
        .decode = im_png_dec,
        .encode = im_png_enc,
    }
};
static int _img_format_i = sizeof(_img_formats)/sizeof(_img_formats[0]);

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
    char buf[8192];
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
            mbuf = _xalloc(malloc, msize);
            mbufsz = msize;
        } else if (mbufsz < msize) {
            mbuf = _xrealloc(realloc, mbuf, msize);
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
    free(mbuf);

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
