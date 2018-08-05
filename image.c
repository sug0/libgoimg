#include <string.h>

#include "image.h"
#include "fmt_goimg.h"
#include "color_rgba.h"
#include "util.h"

/* this variable is used to register new color formats */
static int _color_id_counter = 0;

/* this array is used to store new image formats */
static int _img_format_i = 1;
static ImageFormat_t _img_formats[GOIMG_NO_FMTS] = {
    [0] = {
        .magic = "\x06\x00\x10\x00",
        .magic_size = 4,
        .name = "GoIMG",
        .decode = im_goimg_dec,
        .encode = im_goimg_enc,
        .color_model = im_colormodel_rgba,
        .at = im_goimg_at,
        .set = im_goimg_set
    }
};

inline void im_register_format(ImageFormat_t *fmt)
{
    _img_formats[_img_format_i++] = *fmt;
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
            return _img_formats[i].encode(img, &_img_formats[i], wf, dst);

    return -1;
}

ImageFormat_t *im_get_format(char *fmt)
{
    int i;

    for (i = 0; i < _img_format_i; i++)
        if (strcmp(_img_formats[i].name, fmt) == 0)
            return &_img_formats[i];

    return NULL;
}
