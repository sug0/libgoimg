#include <stdio.h>

#include <jpeglib.h>
#include <jerror.h>

#include "color.h"
#include "fmt_jpeg.h"

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

    if (n <= 0) {
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

    if (n > 0) {
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

struct _dst_mgr {
    struct jpeg_destination_mgr pub;

    wfun_t wf;
    void *dst;
    char buf[8192];
};

static void _jpeg_goio_dest(j_compress_ptr cinfo, wfun_t wf, void *dst)
{
    /* implement this */
}

/* -------------------------------------------------------------------------- */

int im_jpeg_dec(Image_t *img, rfun_t rf, void *src)
{
    return 0;
}

int im_jpeg_enc(Image_t *img, wfun_t wf, void *dst)
{
    return 0;
}
