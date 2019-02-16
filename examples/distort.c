#include <stdlib.h>
#include <math.h>
#include <thpool.h>
#include <goimg/goimg.h>

/*
 * thread pool available at
 * https://github.com/Pithikos/C-Thread-Pool
 * */

#ifdef _WIN32
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#   define GET_PROCS()  ({SYSTEM_INFO i; GetSystemInfo(&i); i.dwNumberOfProcessors;})
#else
#   include <sys/sysinfo.h>
#   define GET_PROCS()  get_nprocs()
#endif

double amt, size;
void im_distort(Image_t *dst, Image_t *const src);

struct th_arg {
    int y;
    Image_t *src;
    Image_t *dst;
};

int main(int argc, char *argv[])
{
    if (argc < 3)
        return 1;

    int err = 0;

    amt = atof(argv[1]);
    size = atof(argv[2]);

    Image_t newimg = {.img = NULL, .allocator = im_std_allocator},
            img = {.img = NULL, .allocator = im_std_allocator};

    /* load formats */
    im_load_defaults();

    /* decode image from stdin */
    if (!im_decode(&img, fdread, GOIO_FD(0))) {
        err = 1;
        goto done;
    }

    /* swirl image pixels! */
    im_distort(&newimg, &img);

    /* encode image to stdout as farbfeld */
    if (im_encode(&newimg, "farbfeld", fdwrite, GOIO_FD(1)) < 0)
        err = 1;

done:
    im_xfree(im_std_allocator, img.img);
    im_xfree(im_std_allocator, newimg.img);

    return err;
}

inline void distort(int *u, int *v, int x, int y, int cx, int cy)
{
    const double x0 = (double)(x - cx);
    const double y0 = (double)(y - cy);

    //const double r = ((amt/x0)+(amt/y0))*tan(fabs(x0-y0)*size)*exp(M_PI*x0/y0);
    const double r = sinh(cos(x-y)/y*2*M_PI)+sqrt(fabs(M_PI-(x+y)))/(amt*size);
    //const double r = (sin(255.0*M_PI*cos(x0*y0))*exp(M_PI/amt))/size + sqrt(fabs(x0-amt));
    //const double r = amt*cos(size-amt)*(x0>y0?sin:cos)(tan(size)*M_PI-x-y);
    const double s = sin(r);
    const double c = cos(r);

    *u = (int)(c*x0 + s*y0) + cx;
    *v = (int)(-s*x0 + c*y0) + cy;
}

inline void clamp(int *u, int *v, int w, int h)
{
    if (*u < 0)  *u = 0;
    if (*u >= w) *u = w - 1;
    if (*v < 0)  *v = 0;
    if (*v >= h) *v = h - 1;
}

void worker(void *args)
{
    struct th_arg *a = (struct th_arg *)args;

    Color_t c_src = im_newcolor_from_img(a->src),
            c_dst = im_newcolor_nrgba64();

    int x, u, v;
    const int cx = a->src->w/2;
    const int cy = a->src->h/2;

    for (x = 0; x < a->src->w; x++) {
        distort(&u, &v, x, a->y, cx, cy);
        clamp(&u, &v, a->src->w, a->src->h);
        a->src->at(a->src, u, v, &c_src);
        im_colormodel_nrgba64(&c_dst, &c_src);
        a->dst->set(a->dst, x, a->y, &c_dst);
    }

    im_xfree(im_std_allocator, c_src.color);
    im_xfree(im_std_allocator, c_dst.color);
    im_xfree(im_std_allocator, args);
}

inline void im_distort_singlecore(Image_t *dst, Image_t *const src)
{
    int x, y, u, v;

    const int cx = src->w/2;
    const int cy = src->h/2;

    Color_t c_src = im_newcolor_from_img(src),
            c_dst = im_newcolor_nrgba64();

    im_initimg_nrgba64(dst, src->w, src->h, im_std_allocator);

    for (y = 0; y < src->h; y++) {
        for (x = 0; x < src->w; x++) {
            distort(&u, &v, x, y, cx, cy);
            clamp(&u, &v, src->w, src->h);
            src->at(src, u, v, &c_src);
            im_colormodel_nrgba64(&c_dst, &c_src);
            dst->set(dst, x, y, &c_dst);
        }
    }

    im_xfree(im_std_allocator, c_src.color);
    im_xfree(im_std_allocator, c_dst.color);
}

void im_distort(Image_t *dst, Image_t *const src)
{
    const int nprocs = GET_PROCS();

    if (nprocs == 1) {
        im_distort_singlecore(dst, src);
        return;
    }

    int y;
    threadpool pool = thpool_init(nprocs);

    im_initimg_nrgba64(dst, src->w, src->h, im_std_allocator);

    for (y = 0; y < src->h; y++) {
        struct th_arg *a = im_xalloc(im_std_allocator, sizeof(struct th_arg));
        a->y = y;
        a->src = src;
        a->dst = dst;
        thpool_add_work(pool, worker, a);
    }

    thpool_wait(pool);
    thpool_destroy(pool);
}
