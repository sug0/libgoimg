#ifndef GOIO_H
#define GOIO_H

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

/* the final element in a Multi*_t array */
#define GOIO_MEND  {NULL, NULL}

/* use up to GOIO_FD_NUM file descriptors to avoid 
 * creating a new variable */
#define GOIO_FD(NO)  goio_fd((NO))
#define GOIO_FD_NUM  32

extern int *goio_fd(int fd);

/* -------------------------------------------------------------------------- */

/* the reading function; returns the number of bytes read, if
 * no error occurred, otherwise 0 on EOF or a negative int
 * for other cases; up to 'size' bytes are read into 'buf' */
typedef int (*rfun_t)(void *src, char *buf, int size);

/* the writing function; returns the number of bytes written, if
 * no error occurred, otherwise 0 on EOF or a negative int
 * for other cases; up to 'size' bytes are written from 'buf' */
typedef int (*wfun_t)(void *dst, char *buf, int size);

/* -------------------------------------------------------------------------- */

/* copy all the contents of 'src' to 'dst',
 * using 'rf and 'wf' respectively,
 * until we encounter an error or EOF */
extern int rwcpy(wfun_t wf, void *dst, rfun_t rf, void *src);

/* same as above but use the provided buffer 'buf'
 * while performing the copying operation,
 * preserving thread safety */
extern int rwcpy_r(wfun_t wf, void *dst, rfun_t rf, void *src,
                   char *buf, int size);

/* -------------------------------------------------------------------------- */

/* buffers a reader */
typedef struct _s_goio_rbuf {
    rfun_t rf;
    void *src;
    int r, w;
    char *buf;
    int bufsz;
} BufferedReader_t;

/* whatever's read with 'rf' from 'src'
 * is written into 'dst' with 'wf' */
typedef struct _s_goio_teereader {
    wfun_t wf;
    void *dst;
    rfun_t rf;
    void *src;
} TeeReader_t;

/* streams a write across all writers */
typedef struct _s_goio_multiwriter {
    wfun_t wf;
    void *dst;
} MultiWriter_t;

/* concatenates together all the readers */
typedef struct _s_goio_multireader {
    rfun_t rf;
    void *src;
} MultiReader_t;

/* -------------------------------------------------------------------------- */

/* rfun_t for BufferedReader_t;
 * 'src' is a pointer to a BufferedReader_t */
extern int rbufread(void *src, char *buf, int size);

/* rfun_t for a file descriptor;
 * 'src' is a pointer to an int containing a file descritptor */
extern int fdread(void *src, char *buf, int size);

/* rfun_t for TeeReader_t;
 * 'src' is a pointer to a TeeReader_t */
extern int teeread(void *src, char *buf, int size);

/* rfun_t for MultiReader_t;
 * 'src' is a pointer to a pointer of GOIO_MEND terminated
 * array of MultiReader_t */
extern int multiread(void *src, char *buf, int size);

/* -------------------------------------------------------------------------- */

/* wfun_t for a file descritptor;
 * 'dst' is a pointer to an int containing a file descritptor */
extern int fdwrite(void *dst, char *buf, int size);

/* wfun_t for a MultiWriter_t;
 * 'dst' is a pointer to a GOIO_MEND terminated array of MultiWriter_t */
extern int multiwrite(void *dst, char *buf, int size);

/* -------------------------------------------------------------------------- */

/* used to peek the first 'npeek' bytes
 * of a reader, which are read into 'buf' */
extern int rbufpeek(void *src, char *buf, int npeek);

#ifdef __cplusplus
}
#endif


#endif
