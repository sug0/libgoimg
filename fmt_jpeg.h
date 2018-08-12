#ifndef GOIMG_FMT_JPEG_H
#define GOIMG_FMT_JPEG_H

#include "image.h"

extern int im_jpeg_dec(Image_t *img, rfun_t rf, void *src);
extern int im_jpeg_enc(Image_t *img, wfun_t wf, void *dst);

#endif

