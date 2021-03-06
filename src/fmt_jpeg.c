#include <stdio.h>
#include <string.h>

#include <jpeglib.h>
#include <jerror.h>

#include "color.h"
#include "fmt_jpeg.h"
#include "util.h"

struct _src_mgr {
    struct jpeg_source_mgr pub;

    rfun_t rf;
    void *src;
    char buf[8192];
    boolean start_of_file;
};

static void _init_source(j_decompress_ptr cinfo)
{
    struct _src_mgr *m_src = (struct _src_mgr *)cinfo->src;
    m_src->start_of_file = TRUE;
}

static boolean _fill_input_buffer(j_decompress_ptr cinfo)
{
    struct _src_mgr *m_src = (struct _src_mgr *)cinfo->src;
    int n;

    n = m_src->rf(m_src->src, m_src->buf, sizeof(m_src->buf));

    if (unlikely(n <= 0)) {
        if (m_src->start_of_file)
            ERREXIT(cinfo, JERR_INPUT_EMPTY);

        WARNMS(cinfo, JWRN_JPEG_EOF);
        m_src->buf[0] = (JOCTET)0xff;
        m_src->buf[1] = (JOCTET)JPEG_EOI;
        n = 2;
    }

    m_src->pub.next_input_byte = (JOCTET *)m_src->buf;
    m_src->pub.bytes_in_buffer = n;
    m_src->start_of_file = FALSE;

    return TRUE;
}

static void _skip_input_data(j_decompress_ptr cinfo, long n)
{
    struct jpeg_source_mgr *src = cinfo->src;

    if (likely(n > 0)) {
        while (n > (long)src->bytes_in_buffer) {
            n -= (long)src->bytes_in_buffer;
            src->fill_input_buffer(cinfo);
        }
        src->next_input_byte += (size_t)n;
        src->bytes_in_buffer -= (size_t)n;
    }
}

static void _term_source(j_decompress_ptr cinfo)
{
    /* nothing to do */
}

static void _jpeg_goio_src(j_decompress_ptr cinfo, rfun_t rf, void *src)
{
    struct _src_mgr *m_src;

    if (!cinfo->src) {
        cinfo->src = (struct jpeg_source_mgr *)
            cinfo->mem->alloc_small((j_common_ptr)cinfo,
                                    JPOOL_PERMANENT,
                                    sizeof(struct _src_mgr));
    }

    /* retrieve the pointer to the custom reader */
    m_src = (struct _src_mgr *)cinfo->src;

    /* public stuff */
    m_src->pub.init_source = _init_source;
    m_src->pub.fill_input_buffer = _fill_input_buffer;
    m_src->pub.skip_input_data = _skip_input_data;
    m_src->pub.resync_to_restart = jpeg_resync_to_restart;
    m_src->pub.term_source = _term_source;
    m_src->pub.next_input_byte = NULL;
    m_src->pub.bytes_in_buffer = 0;

    /* private stuff */
    m_src->rf = rf;
    m_src->src = src;
}

/* -------------------------------------------------------------------------- */

#define __WSZ  sizeof(m_dst->buf)

struct _dst_mgr {
    struct jpeg_destination_mgr pub;

    wfun_t wf;
    void *dst;
    char buf[8192];
};

static void _init_destination(j_compress_ptr cinfo)
{
    struct _dst_mgr *m_dst = (struct _dst_mgr *)cinfo->dest;
    m_dst->pub.next_output_byte = (JOCTET *)m_dst->buf;
    m_dst->pub.free_in_buffer = __WSZ;
}

static boolean _empty_output_buffer(j_compress_ptr cinfo)
{
    struct _dst_mgr *m_dst = (struct _dst_mgr *)cinfo->dest;

    if (unlikely(m_dst->wf(m_dst->dst, m_dst->buf, __WSZ) != __WSZ))
        ERREXIT(cinfo, JERR_FILE_WRITE);

    m_dst->pub.next_output_byte = (JOCTET *)m_dst->buf;
    m_dst->pub.free_in_buffer = __WSZ;

    return TRUE;
}

static void _term_destination(j_compress_ptr cinfo)
{
    struct _dst_mgr *m_dst = (struct _dst_mgr *)cinfo->dest;
    size_t n = __WSZ - m_dst->pub.free_in_buffer;

    if (unlikely(n > 0 && m_dst->wf(m_dst->dst, m_dst->buf, n) != n))
        ERREXIT(cinfo, JERR_FILE_WRITE);
}

#undef __WSZ

static void _jpeg_goio_dest(j_compress_ptr cinfo, wfun_t wf, void *dst)
{
    struct _dst_mgr *m_dst;

    if (!cinfo->dest) {
        cinfo->dest = (struct jpeg_destination_mgr *)
            cinfo->mem->alloc_small((j_common_ptr)cinfo,
                                    JPOOL_PERMANENT,
                                    sizeof(struct _dst_mgr));
    }

    /* retrieve the pointer to the custom writer */
    m_dst = (struct _dst_mgr *)cinfo->dest;

    /* public stuff */
    m_dst->pub.init_destination = _init_destination;
    m_dst->pub.empty_output_buffer = _empty_output_buffer;
    m_dst->pub.term_destination = _term_destination;

    /* private stuff */
    m_dst->wf = wf;
    m_dst->dst= dst;
}

static void _jpeg_setup_writer(j_compress_ptr cinfo,
                               struct jpeg_error_mgr *jerr,
                               Image_t *img,
                               int color_space, int components, int quality,
                               wfun_t wf, void *dst)
{
    /* create write struct */
    cinfo->err = jpeg_std_error(jerr);
    jpeg_create_compress(cinfo);

    /* set dst */
    _jpeg_goio_dest(cinfo, wf, dst);

    /* set image data */
    cinfo->image_width = img->w;
    cinfo->image_height = img->h;
    cinfo->in_color_space = color_space;
    cinfo->input_components = components;

    /* setup rest with default stuff */
    jpeg_set_defaults(cinfo);

    /* set quality to a reasonable
     * default value */
    jpeg_set_quality(cinfo, quality, TRUE);

    /* start compressing... */
    jpeg_start_compress(cinfo, TRUE);
}

static boolean _jpeg_setup_reader(j_decompress_ptr cinfo,
                                  struct jpeg_error_mgr *jerr,
                                  Image_t *img,
                                  rfun_t rf, void *src)
{
    /* create read struct */
    cinfo->err = jpeg_std_error(jerr);
    jpeg_create_decompress(cinfo);

    /* set dst */
    _jpeg_goio_src(cinfo, rf, src);

    /* read header */
    if (unlikely(jpeg_read_header(cinfo, TRUE) != JPEG_HEADER_OK))
        return FALSE;

    /* set image data */
    img->w = cinfo->image_width;
    img->h = cinfo->image_height;
    img->size = img->w * img->h * sizeof(RGB_t);
    img->img = im_xalloc(img->allocator, img->size);
    img->color_model = im_colormodel_rgb;
    img->at = im_rgb_at;
    img->set = im_rgb_set;

    /* set color space stuff */
    cinfo->out_color_space = JCS_RGB;
    cinfo->output_components = 3;

    /* start decompressing... */
    jpeg_start_decompress(cinfo);

    return TRUE;
}

/* -------------------------------------------------------------------------- */

int im_jpeg_dec(Image_t *img, rfun_t rf, void *src)
{
    int err = 0;

    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    /* init structs */
    memset(&cinfo, 0, sizeof(struct jpeg_decompress_struct));
    memset(&jerr, 0, sizeof(struct jpeg_error_mgr));

    /* setup reader to get img width, allocate mem, etc */
    _im_maybe_jmp_err(_jpeg_setup_reader(&cinfo, &jerr, img, rf, src));

    /* read image data */
    int y;
    JSAMPROW row;

    for (y = 0; y < img->h; y++) {
        row = (JSAMPROW)img->img + y*img->w*sizeof(RGB_t);
        jpeg_read_scanlines(&cinfo, &row, 1);
    }

done:
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    return err;
}

int im_jpeg_enc(Image_t *img, wfun_t wf, void *dst)
{
    int err = 0, color_space, components, pix_width;

    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;

    /* init structs */
    memset(&cinfo, 0, sizeof(struct jpeg_compress_struct));
    memset(&jerr, 0, sizeof(struct jpeg_error_mgr));

    /* determine color space */
    if (img->color_model == im_colormodel_gray) {
        color_space = JCS_GRAYSCALE;
        components = 1;
        pix_width = sizeof(uint8_t);
    } else if (img->color_model == im_colormodel_rgb) {
        color_space = JCS_RGB;
        components = 3;
        pix_width = sizeof(RGB_t);
#ifdef JCS_EXTENSIONS
    } else if (img->color_model == im_colormodel_nrgba) {
        color_space = JCS_EXT_RGBA;
        components = 4;
        pix_width = sizeof(uint32_t);
#endif
    } else {
        goto lossy;
    }

    _jpeg_setup_writer(&cinfo, &jerr,
                       img, color_space, components, 85, wf, dst);

    int y;
    JSAMPROW row;

    for (y = 0; y < img->h; y++) {
        row = (JSAMPROW)img->img + y*img->w*pix_width;
        jpeg_write_scanlines(&cinfo, &row, 1);
    }

    goto done;

lossy:
    _jpeg_setup_writer(&cinfo, &jerr,
                       img, JCS_RGB, 3, 85, wf, dst);

    int x;
    row = im_xalloc(im_std_allocator, sizeof(RGB_t) * img->w);

    Color_t c_src = im_newcolor_from_img(img),
            c_dst = im_newcolor_rgb();

    for (y = 0; y < img->h; y++) {
        for (x = 0; x < img->w; x++) {
            img->at(img, x, y, &c_src);
            im_colormodel_rgb(&c_dst, &c_src);
            ((RGB_t *)row)[x] = *(RGB_t *)c_dst.color;
        }
        jpeg_write_scanlines(&cinfo, &row, 1);
    }

    im_xfree(im_std_allocator, row);
    im_xfree(im_std_allocator, c_src.color);
    im_xfree(im_std_allocator, c_dst.color);

done:
    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);

    return err;
}

/* -------------------------------------------------------------------------- */

static ImageFormat_t _im_fmt_jpeg = {
    .magic = "\xff\xd8\xff",
    .magic_size = 3,
    .name = "JPEG",
    .decode = im_jpeg_dec,
    .encode = im_jpeg_enc,
};

inline void im_register_format_jpeg(void)
{
    im_register_format(&_im_fmt_jpeg);
}
