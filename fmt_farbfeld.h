#ifndef GOIMG_FMT_FARBFELD_H
#define GOIMG_FMT_FARBFELD_H

#include "image.h"

extern int im_farbfeld_dec(Image_t *img, rfun_t rf, void *src);
extern int im_farbfeld_enc(Image_t *img, wfun_t wf, void *dst);
extern void im_farbfeld_at(Image_t *img, int x, int y, Color_t *dst);
extern void im_farbfeld_set(Image_t *img, int x, int y, Color_t *src);

#endif
