#ifndef GOIMG_FMT_GOIMG_H
#define GOIMG_FMT_GOIMG_H

#include "image.h"

extern int im_goimg_dec(Image_t *img, rfun_t rf, void *src);
extern int im_goimg_enc(Image_t *img, ImageFormat_t *fmt, wfun_t wf, void *dst);
extern void im_goimg_at(Image_t *img, int x, int y, Color_t *dst);
extern void im_goimg_set(Image_t *img, int x, int y, Color_t *src);

#endif
