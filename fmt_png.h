#ifndef GOIMG_FMT_PNG_H
#define GOIMG_FMT_PNG_H

#include "image.h"

extern void im_register_format_png(void);
extern int im_png_dec(Image_t *img, rfun_t rf, void *src);
extern int im_png_enc(Image_t *img, wfun_t wf, void *dst);

#endif
