/*----------------------------------------------------------------------------/
/ TJpgDec - Tiny JPEG Decompressor
/-----------------------------------------------------------------------------/
/ The TJpgDec is a generic JPEG decompressor module for tiny embedded systems.
/ This is a free software that opened for education, research and commercial
/  developments under license policy of following terms.
/
/  Copyright (C) 2021, ChaN, all right reserved.
/  Copyright (C) 2025, JiapengLi, all right reserved.
/
/ * The TJpgDec module is a free software and there is NO WARRANTY.
/ * No restriction on use. You can use, modify and redistribute it for
/   personal, non-profit or commercial products UNDER YOUR RESPONSIBILITY.
/ * Redistributions of source code must retain the above copyright notice.
/
/----------------------------------------------------------------------------*/
#ifndef DEF_TJPGDEC
#define DEF_TJPGDEC

#include "tjpgdcnf.h"
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__GNUC__)  // GCC or compatible compiler
#include <stdint.h>
#include <stdbool.h>
#elif defined(_WIN32)
typedef unsigned char   uint8_t;
typedef unsigned short  uint16_t;
typedef short           int16_t;
typedef unsigned long   uint32_t;
typedef long            int32_t;
#else
#include <stdint.h>
#include <stdbool.h>
#endif

#if JD_FASTDECODE >= 1
typedef int16_t jd_yuv_t;
#else
typedef uint8_t jd_yuv_t;
#endif

typedef enum {
    JD_GRAYSCALE    = 0,    // 1B
    JD_RGB565       = 1,    // 2B
    JD_BGR565       = 2,    // 2B
    JD_RGB888       = 3,    // 3B
    JD_BGR888       = 4,    // 3B
    JD_RGBA8888     = 5,    // 4B
    JD_BGRA8888     = 6,    // 4B
} JCOLOR;

/* Error code */
typedef enum {
    JDR_OK = 0, /* 0: Succeeded */
    JDR_INTR,   /* 1: Interrupted by output function */
    JDR_INP,    /* 2: Device error or wrong termination of input stream */
    JDR_MEM1,   /* 3: Insufficient memory pool for the image */
    JDR_MEM2,   /* 4: Insufficient stream input buffer */
    JDR_PAR,    /* 5: Parameter error */
    JDR_FMT1,   /* 6: Data format error (may be broken data) */
    JDR_FMT2,   /* 7: Right format but not supported */
    JDR_FMT3,   /* 8: Not supported JPEG standard */
    JDR_FMT4,
    JDR_YUV,
} JRESULT;

/* Rectangular region in the output image */
typedef struct {
    uint16_t left;      /* Left end */
    uint16_t right;     /* Right end */
    uint16_t top;       /* Top end */
    uint16_t bottom;    /* Bottom end */
} JRECT;

typedef struct {
    uint8_t *huffbits;    /* Huffman bit distribution tables [id][dcac] */
    uint16_t *huffcode;   /* Huffman code word tables [id][dcac] */
    uint8_t *huffdata;    /* Huffman decoded data tables [id][dcac] */
} JHUFF;

typedef struct {
    JHUFF huff[2];          /* Huffman tables for DC/AC components */
    int32_t *qttbl;
    int16_t *dcv;
} JCOMP;

typedef struct JDEC JDEC;
typedef int32_t (*jd_infunc_t)(JDEC *, uint8_t *, int32_t);
typedef int (*jd_outfunc_t)(JDEC *, void *, JRECT *);
typedef void (*jd_yuv_scan_t)(JDEC *, JRECT *mcu_rect, JRECT *tgt_rect);
typedef void (*jd_yuv_fmt_t)(uint8_t **pix, int yy, int cb, int cr);

typedef struct JTABLE {
    uint8_t *huffbits[2][2];    /* Huffman bit distribution tables [id][dcac] */
    uint16_t *huffcode[2][2];   /* Huffman code word tables [id][dcac] */
    uint8_t *huffdata[2][2];    /* Huffman decoded data tables [id][dcac] */
    int32_t *qttbl[4];          /* Dequantizer tables [id] */
    uint8_t qtid[3];            /* Quantization table ID of each component, Y, Cb, Cr */
} JTABLE;

/* Decompressor object structure */
struct JDEC {
    int32_t dctr;               /* Number of bytes available in the input buffer */
    uint8_t *dptr;              /* Current data read ptr */
    uint8_t *inbuf;             /* Bit stream input buffer */

    uint8_t msx, msy;           /* MCU size in unit of block (width, height) */
    uint8_t ncomp;              /* Number of color components 1:grayscale, 3:color */
    uint8_t color;              /* Output color space */
    uint16_t nrst;              /* Restart interval */
    uint16_t width, height;     /* Size of the input image (pixel) */
    int16_t dcv[3];             /* Previous DC element of each component */
    JCOMP component[6];         /* maximum 6 components, Huffman tables for Y, Cb, Cr components */

#if JD_FASTDECODE == 2
    uint8_t longofs[2][2];      /* Table offset of long code [id][dcac] */
    uint16_t *hufflut_ac[2];    /* Fast huffman decode tables for AC short code [id] */
    uint8_t *hufflut_dc[2];     /* Fast huffman decode tables for DC short code [id] */
#endif

    void *workbuf;              /* Working buffer for IDCT and RGB output */
    jd_yuv_t *mcubuf;           /* Working buffer for the MCU */

    void *pool;                 /* Pointer to available memory pool */
    int32_t sz_pool;            /* Size of memory pool (bytes available) */

    jd_yuv_fmt_t yuv_fmt;
    jd_yuv_scan_t yuv_scan;

    jd_outfunc_t outfunc;
    jd_infunc_t infunc;         /* Pointer to jpeg stream input function */
    void *device;               /* Pointer to I/O device identifier for the session */
};

/* TJpgDec API functions */
JRESULT jd_prepare(JDEC *jd, jd_infunc_t infunc, void *pool, int32_t sz_pool, void *dev);
JRESULT jd_decomp(JDEC *jd, jd_outfunc_t outfunc, uint8_t scale);

JRESULT jd_set_color(JDEC *jd, JCOLOR color);
JRESULT jd_decomp_rect(JDEC *jd, jd_outfunc_t outfunc, JRECT *rect);

#ifdef __cplusplus
}
#endif

#endif /* _TJPGDEC */
