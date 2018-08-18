# libgoimg

A C library that aims to provide the ease of operation Go enables for image processing.

It is extremely modular in the sense that you can leave out all formats that ship with
it by default, implementing your own decoding and encoding routines for your custom format,
or even your custom image and color routines for custom color spaces.

# Status

This library is most likely not ready for production, but you can
safely use it for your personal projects. Right now you can
encode and decode the farbfeld, PNG and JPEG formats, with the
later two requiring `libpng` and `libjpeg(-turbo)` respectively.

# Building

This library comes with a simple python build script I wrote. The syntax for the
command line arguments it accepts is:

`python build.py [-i:INSTALL_PATH_PREFIX] [FMT_1+FMT_2+...]`

To build with support for all formats do:

`python build.py png+jpeg+farbfeld`

To install it after building:

`sudo python build.py -i:/usr/local png+jpeg+farbfeld`

To build with none of these formats:

`python build.py`

# Linking against this library

I chose to only build a static library, because the code base is simple enough,
and it catered to my use cases. However, hacking the python script to
build a shared library shouldn't be too hard.

# Documentation

You can find what each function does reading the header files. I tried documenting
them as best as I could. To simplify things however, I'll provide a few examples
on the following subsections.

## Converting an image to JPEG

```c
#include <stdio.h>
#include <goimg/goimg.h>

int fpread(void *src, char *buf, int size)
{
    return fread(buf, 1, size, (FILE *)src);
}

int fpwrite(void *dst, char *buf, int size)
{
    return fwrite(buf, 1, size, (FILE *)dst);
}

int main(void)
{
    int err = 0;
    Image_t img = {.img = NULL, .allocator = im_std_allocator};

    im_load_defaults();

    if (!im_decode(&img, fpread, stdin)) {
        err = 1;
        goto done;
    }

    if (im_encode(&img, "JPEG", fpwrite, stdout) < 0)
        err = 1;

done:
    im_xfree(im_std_allocator, img.img);

    return err;
}
```

## Drawing a circle with a custom allocator

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#include <goio.h>
#include "image.h"
#include "color.h"

#define DIM  5000
#define M    ((DIM)/2)

#define NO_ADDR  8

#define COLOR_WHITE  255
#define COLOR_BLACK  0

struct mem {
    struct {
        void *addr;
        FILE *fp;
        size_t size;
    } addrs[NO_ADDR];
};

int sq(int x);
uint8_t get_color(int x, int y);

void *mem_alloc(void *data, size_t size);
void *mem_realloc(void *data, void *addr, size_t size);
void mem_free(void *data, void *addr);

int main(void)
{
    int x, y, err = 0;

    struct mem mem;
    Allocator_t allocator = {
        .alloc = mem_alloc,
        .realloc = mem_realloc,
        .free = mem_free,
        .data = &mem
    };

    /* zero out allocator memory */
    memset(&mem, 0, sizeof(struct mem));

    Color_t c = im_newcolor_gray();
    Image_t img = im_newimg_gray(DIM, DIM, &allocator);

    for (y = 0; y < DIM; y++) {
        for (x = 0; x < DIM; x++) {
            *(uint8_t *)c.color = get_color(x, y);
            img.set(&img, x, y, &c);
        }
    }

    if (im_encode(&img, "PNG", fdwrite, GOIO_FD(1)) < 0)
        err = 1;

    im_xfree(im_std_allocator, c.color);
    im_xfree(&allocator, img.img);

    return err;
}

inline int sq(int x)
{
    return x*x;
}

inline uint8_t get_color(int x, int y)
{
    return (sq(x - M) + sq(y - M) < sq(M)) ? COLOR_WHITE : COLOR_BLACK;
}

void *_allocate(struct mem_addr *mem, size_t size)
{
    void *addr;

    if (!mem)
        return NULL;

    if (!mem->fp) {
        mem->fp = tmpfile();

        if (!mem->fp)
            return NULL;
    }

    /* initialize memory area */
    fseek(mem->fp, size, SEEK_SET);
    fputc('\0', mem->fp);
    fseek(mem->fp, 0, SEEK_SET);

    /* map region */
    addr = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, fileno(mem->fp), 0);

    if (addr == MAP_FAILED) {
        fclose(mem->fp);
        return NULL;
    }

    /* save the state */
    mem->addr = addr;
    mem->size = size;

    return addr;
}

void *mem_alloc(void *data, size_t size)
{
    struct mem *mem = (struct mem *)data;
    int i;
    
    /* find free space */
    for (i = 0; i < NO_ADDR; i++)
        if (!mem->addrs[i].addr)
            return _allocate(&mem->addrs[i], size);

    return NULL;
}

void *mem_realloc(void *data, void *addr, size_t size)
{
    struct mem *mem = (struct mem *)data;
    struct mem_addr *mem_addr = NULL;
    int i;

    for (i = 0; i < NO_ADDR; i++) {
        if (!mem_addr && !mem->addrs[i].addr)
            mem_addr = &mem->addrs[i];

        if (mem->addrs[i].addr == addr) {
            munmap(addr, mem->addrs[i].size);
            return _allocate(&mem->addrs[i], size);
        }
    }

    return _allocate(mem_addr, size);
}

void mem_free(void *data, void *addr)
{
    struct mem *mem = (struct mem *)data;
    int i;

    for (i = 0; i < NO_ADDR; i++) {
        if (mem->addrs[i].addr == addr) {
            munmap(addr, mem->addrs[i].size);
            fclose(mem->addrs[i].fp);
            memset(&mem->addrs[i], 0, sizeof(struct mem_addr));
            return;
        }
    }
}
```
