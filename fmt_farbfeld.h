#ifndef GOIMG_FMT_FARBFELD_H
#define GOIMG_FMT_FARBFELD_H

#include "image.h"

extern void im_register_format_farbfeld(void);
extern int im_farbfeld_dec(Image_t *img, rfun_t rf, void *src);
extern int im_farbfeld_enc(Image_t *img, wfun_t wf, void *dst);

#endif
