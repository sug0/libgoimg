#include <string.h>
#include <goimg/goimg.h>

#define DIM 1900
#define SQ  1500

struct data {
    Image_t img;
    Color_t col;
} *dd;

inline void line_lo(int x0, int y0, int x1, int y1)
{
    const int dx = x1 - x0;
    int dy = y1 - y0;
    int yi = 1;

    if (dy < 0) {
        yi *= -1;
        dy *= -1;
    }

    int d = 2*dy - dx;
    int x, y = y0;

    for (x = x0; x < x1; x++) {
        if (x >= 0 && x < dd->img.w
        &&  y >= 0 && y < dd->img.h)
            dd->img.set(&dd->img, x, y, &dd->col);

        if (d > 0) {
            y += yi;
            d -= 2*dx;
        }

        d += 2*dy;
    }
}

inline void line_hi(int x0, int y0, int x1, int y1)
{
    int dx = x1 - x0;
    const int dy = y1 - y0;
    int xi = 1;

    if (dx < 0) {
        xi *= -1;
        dx *= -1;
    }

    int d = 2*dx - dy;
    int x = x0, y;

    for (y = y0; y < y1; y++) {
        if (x >= 0 && x < dd->img.w
        &&  y >= 0 && y < dd->img.h)
            dd->img.set(&dd->img, x, y, &dd->col);

        if (d > 0) {
            x += xi;
            d -= 2*dy;
        }

        d += 2*dx;
    }
}

inline int iabs(int x)
{
    const int y = x >> 31;
    return (x ^ y) - y;
}

inline void line(int x0, int y0, int x1, int y1)
{
    if (iabs(y1 - y0) < iabs(x1 - x0)) {
        if (x0 > x1)
            line_lo(x1, y1, x0, y0);
        else
            line_lo(x0, y0, x1, y1);
    } else {
        if (y0 > y1)
            line_hi(x1, y1, x0, y0);
        else
            line_hi(x0, y0, x1, y1);
    }
}

void triangle(int t, int x0, int y0, int x1, int y1, int x2, int y2)
{
    if (t <= 0) return;

    line(x0,y0, x1,y1);
    line(x1,y1, x2,y2);
    line(x2,y2, x0,y0);

    x0 += 2; y0 += 2;
    x1 += 2; y1 += 2;
    x2 += 2; y2 += 2;

    triangle(t-1, x0,y0, x1,y1, x2,y2);
}

int main(void)
{
    int err = 0;

    struct data d = {
        .img = {.allocator = im_std_allocator},
        .col = im_newcolor_nrgba(),
    };
    dd = &d;

    im_load_defaults();
    im_initimg_nrgba(&d.img, DIM, DIM, im_std_allocator);

    /* white bg */
    memset(d.img.img, 0xff, d.img.size);

    /* black */
    *(uint32_t *)d.col.color = im_decl_nrgba(0, 0, 0, 255);

    /* draw image */
    triangle(200, 10,SQ-11, SQ-11,SQ-11, (SQ/2)-11, 10);

    if (im_encode(&d.img, "PNG", fdwrite, GOIO_FD(1)) < 0)
        err = 1;

    im_xfree(im_std_allocator, d.img.img);
    im_xfree(im_std_allocator, d.col.color);

    return err;
}
