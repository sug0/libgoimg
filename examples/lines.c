#include <math.h>
#include <string.h>
#include <goimg/goimg.h>

#define DIM  6000

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
        if (x0 > x1)
            line_hi(x1, y1, x0, y0);
        else
            line_hi(x0, y0, x1, y1);
    }
}

void draw(double q, int x0, int y0, int x1, int y1)
{
    double angle = 0.0, c, s;
    const double step = M_PI/q;

    while (angle < 2*M_PI) {
        line(x0, y0, x1, y1);
        c = cos(angle);
        s = sin(angle);
        x1 = (int)(c*(double)x1 - s*(double)y1);
        y1 = (int)(s*(double)x1 + c*(double)y1);
        angle += step;
    }
}

int main(void)
{
    int err = 0;

    struct data d = {
        .img = {.img = NULL, .allocator = im_std_allocator},
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
    draw((float)(DIM << 1), DIM/2, DIM/2, DIM-1, DIM/2, &d);

    if (im_encode(&d.img, "PNG", fdwrite, GOIO_FD(1)) < 0)
        err = 1;

    im_xfree(im_std_allocator, d.img.img);
    im_xfree(im_std_allocator, d.col.color);

    return err;
}
