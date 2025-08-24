/*----------------------------------------------------------------------------/
/ TJpgDec - Tiny JPEG Decompressor R0.03                      (C)ChaN, 2021
/-----------------------------------------------------------------------------/
/ The TJpgDec is a generic JPEG decompressor module for tiny embedded systems.
/ This is a free software that opened for education, research and commercial
/  developments under license policy of following terms.
/
/  Copyright (C) 2021, ChaN, all right reserved.
/
/ * The TJpgDec module is a free software and there is NO WARRANTY.
/ * No restriction on use. You can use, modify and redistribute it for
/   personal, non-profit or commercial products UNDER YOUR RESPONSIBILITY.
/ * Redistributions of source code must retain the above copyright notice.
/
/-----------------------------------------------------------------------------/
/ Oct 04, 2011 R0.01  First release.
/ Feb 19, 2012 R0.01a Fixed decompression fails when scan starts with an escape seq.
/ Sep 03, 2012 R0.01b Added JD_TBLCLIP option.
/ Mar 16, 2019 R0.01c Supprted stdint.h.
/ Jul 01, 2020 R0.01d Fixed wrong integer type usage.
/ May 08, 2021 R0.02  Supprted grayscale image. Separated configuration options.
/ Jun 11, 2021 R0.02a Some performance improvement.
/ Jul 01, 2021 R0.03  Added JD_FASTDECODE option.
/                     Some performance improvement.
/ Mar 10, 2025 R0.03r Added JD_USE_INTERNAL_32BIT_PIXEL option to support ARGB8888.
/                     Added JD_SWAP_RED_AND_BLUE option.
/----------------------------------------------------------------------------*/

#include "tjpgd.h"


#if JD_FASTDECODE == 2
#define HUFF_BIT    10  /* Bit length to apply fast huffman decode */
#define HUFF_LEN    (1 << HUFF_BIT)
#define HUFF_MASK   (HUFF_LEN - 1)
#endif


/*-----------------------------------------------*/
/* Zigzag-order to raster-order conversion table */
/*-----------------------------------------------*/

static const uint8_t Zig[64] =      /* Zigzag-order to raster-order conversion table */
{
    0,  1,  8, 16,  9,  2,  3, 10, 17, 24, 32, 25, 18, 11,  4,  5,
    12, 19, 26, 33, 40, 48, 41, 34, 27, 20, 13,  6,  7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63
};



/*-------------------------------------------------*/
/* Input scale factor of Arai algorithm            */
/* (scaled up 16 bits for fixed point operations)  */
/*-------------------------------------------------*/

static const uint16_t Ipsf[64] =    /* See also aa_idct.png */
{
    (uint16_t)(1.00000 * 8192), (uint16_t)(1.38704 * 8192), (uint16_t)(1.30656 * 8192), (uint16_t)(1.17588 * 8192), (uint16_t)(1.00000 * 8192), (uint16_t)(0.78570 * 8192), (uint16_t)(0.54120 * 8192), (uint16_t)(0.27590 * 8192),
    (uint16_t)(1.38704 * 8192), (uint16_t)(1.92388 * 8192), (uint16_t)(1.81226 * 8192), (uint16_t)(1.63099 * 8192), (uint16_t)(1.38704 * 8192), (uint16_t)(1.08979 * 8192), (uint16_t)(0.75066 * 8192), (uint16_t)(0.38268 * 8192),
    (uint16_t)(1.30656 * 8192), (uint16_t)(1.81226 * 8192), (uint16_t)(1.70711 * 8192), (uint16_t)(1.53636 * 8192), (uint16_t)(1.30656 * 8192), (uint16_t)(1.02656 * 8192), (uint16_t)(0.70711 * 8192), (uint16_t)(0.36048 * 8192),
    (uint16_t)(1.17588 * 8192), (uint16_t)(1.63099 * 8192), (uint16_t)(1.53636 * 8192), (uint16_t)(1.38268 * 8192), (uint16_t)(1.17588 * 8192), (uint16_t)(0.92388 * 8192), (uint16_t)(0.63638 * 8192), (uint16_t)(0.32442 * 8192),
    (uint16_t)(1.00000 * 8192), (uint16_t)(1.38704 * 8192), (uint16_t)(1.30656 * 8192), (uint16_t)(1.17588 * 8192), (uint16_t)(1.00000 * 8192), (uint16_t)(0.78570 * 8192), (uint16_t)(0.54120 * 8192), (uint16_t)(0.27590 * 8192),
    (uint16_t)(0.78570 * 8192), (uint16_t)(1.08979 * 8192), (uint16_t)(1.02656 * 8192), (uint16_t)(0.92388 * 8192), (uint16_t)(0.78570 * 8192), (uint16_t)(0.61732 * 8192), (uint16_t)(0.42522 * 8192), (uint16_t)(0.21677 * 8192),
    (uint16_t)(0.54120 * 8192), (uint16_t)(0.75066 * 8192), (uint16_t)(0.70711 * 8192), (uint16_t)(0.63638 * 8192), (uint16_t)(0.54120 * 8192), (uint16_t)(0.42522 * 8192), (uint16_t)(0.29290 * 8192), (uint16_t)(0.14932 * 8192),
    (uint16_t)(0.27590 * 8192), (uint16_t)(0.38268 * 8192), (uint16_t)(0.36048 * 8192), (uint16_t)(0.32442 * 8192), (uint16_t)(0.27590 * 8192), (uint16_t)(0.21678 * 8192), (uint16_t)(0.14932 * 8192), (uint16_t)(0.07612 * 8192)
};



/*---------------------------------------------*/
/* Conversion table for fast clipping process  */
/*---------------------------------------------*/

#if JD_TBLCLIP

#define BYTECLIP(v) Clip8[(unsigned int)(v) & 0x3FF]

static const uint8_t Clip8[1024] = {
    /* 0..255 */
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
    32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
    64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
    96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
    128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
    160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
    192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
    224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255,
    /* 256..511 */
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    /* -512..-257 */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* -256..-1 */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

#else   /* JD_TBLCLIP */

static uint8_t BYTECLIP(int val)
{
    if (val < 0) {
        return 0;
    }
    if (val > 255) {
        return 255;
    }
    return (uint8_t)val;
}

#endif

#define CVACC_COEF   1024

static inline uint8_t ycbcr2r(int Y, int Cb, int Cr)
{
    return BYTECLIP(Y + ((int)(1.402 * CVACC_COEF) * Cr) / CVACC_COEF);
}

static inline uint8_t ycbcr2g(int Y, int Cb, int Cr)
{
    return BYTECLIP(Y - (((int)(0.344 * CVACC_COEF) * Cb + (int)(0.714 * CVACC_COEF) * Cr) / CVACC_COEF));
}

static inline uint8_t ycbcr2b(int Y, int Cb, int Cr)
{
    return BYTECLIP(Y + ((int)(1.772 * CVACC_COEF) * Cb) / CVACC_COEF);
}

/*-----------------------------------------------------------------------*/
/* Allocate a memory block from memory pool                              */
/*-----------------------------------------------------------------------*/

static void *alloc_pool(  /* Pointer to allocated memory block (NULL:no memory available) */
    JDEC *jd,               /* Pointer to the decompressor object */
    size_t ndata            /* Number of bytes to allocate */
)
{
    char *rp = 0;


    ndata = (ndata + 3) & ~3;           /* Align block size to the word boundary */

    if (jd->sz_pool >= ndata) {
        jd->sz_pool -= ndata;
        rp = (char *)jd->pool;          /* Get start of available memory pool */
        jd->pool = (void *)(rp + ndata); /* Allocate requierd bytes */
    }

    return (void *)rp;  /* Return allocated memory block (NULL:no memory to allocate) */
}




/*-----------------------------------------------------------------------*/
/* Create de-quantization and prescaling tables with a DQT segment       */
/*-----------------------------------------------------------------------*/

static JRESULT create_qt_tbl(  /* 0:OK, !0:Failed */
    JDEC *jd,               /* Pointer to the decompressor object */
    const uint8_t *data,    /* Pointer to the quantizer tables */
    size_t ndata            /* Size of input data */
)
{
    unsigned int i, zi;
    uint8_t d;
    int32_t *pb;


    while (ndata) { /* Process all tables in the segment */
        if (ndata < 65) {
            return JDR_FMT1;    /* Err: table size is unaligned */
        }
        ndata -= 65;
        d = *data++;                            /* Get table property */
        if (d & 0xF0) {
            return JDR_FMT1;    /* Err: not 8-bit resolution */
        }
        i = d & 3;                              /* Get table ID */
        pb = alloc_pool(jd, 64 * sizeof(int32_t)); /* Allocate a memory block for the table */
        if (!pb) {
            return JDR_MEM1;    /* Err: not enough memory */
        }
        jd->qttbl[i] = pb;                      /* Register the table */
        for (i = 0; i < 64; i++) {              /* Load the table */
            zi = Zig[i];                        /* Zigzag-order to raster-order conversion */
            pb[zi] = (int32_t)((uint32_t) * data++ * Ipsf[zi]); /* Apply scale factor of Arai algorithm to the de-quantizers */
        }
    }

    return JDR_OK;
}




/*-----------------------------------------------------------------------*/
/* Create huffman code tables with a DHT segment                         */
/*-----------------------------------------------------------------------*/

static JRESULT create_huffman_tbl(  /* 0:OK, !0:Failed */
    JDEC *jd,                   /* Pointer to the decompressor object */
    const uint8_t *data,        /* Pointer to the packed huffman tables */
    size_t ndata                /* Size of input data */
)
{
    unsigned int i, j, b, cls, num;
    size_t np;
    uint8_t d, *pb, *pd;
    uint16_t hc, *ph;

    /* header(1) | bits counter map (16) |  */
    while (ndata) { /* Process all tables in the segment */
        if (ndata < 17) {
            return JDR_FMT1;    /* Err: wrong data size */
        }
        ndata -= 17;
        d = *data++;                        /* Get table number and class */
        if (d & 0xEE) {
            return JDR_FMT1;    /* Err: invalid class/number */
        }
        cls = d >> 4;
        num = d & 0x0F;       /* class = dc(0)/ac(1), table number = 0/1 */
        pb = alloc_pool(jd, 16);            /* Allocate a memory block for the bit distribution table */
        if (!pb) {
            return JDR_MEM1;    /* Err: not enough memory */
        }
        jd->huffbits[num][cls] = pb;
        for (np = i = 0; i < 16; i++) {     /* Load number of patterns for 1 to 16-bit code */
            np += (pb[i] = *data++);        /* Get sum of code words for each code */
        }
        ph = alloc_pool(jd, np * sizeof(uint16_t)); /* Allocate a memory block for the code word table */
        if (!ph) {
            return JDR_MEM1;    /* Err: not enough memory */
        }
        jd->huffcode[num][cls] = ph;
        hc = 0;
        for (j = i = 0; i < 16; i++) {      /* Re-build huffman code word table */
            b = pb[i];
            while (b--) {
                ph[j++] = hc++;
            }
            hc <<= 1;
        }

        if (ndata < np) {
            return JDR_FMT1;    /* Err: wrong data size */
        }
        ndata -= np;
        pd = alloc_pool(jd, np);            /* Allocate a memory block for the decoded data */
        if (!pd) {
            return JDR_MEM1;    /* Err: not enough memory */
        }
        jd->huffdata[num][cls] = pd;
        if (cls == 0) {
            for (i = 0; i < np; i++) {          /* Load decoded data corresponds to each code word */
                d = *data++;
                if (d > 11) {
                    return JDR_FMT1;
                }
                pd[i] = d;
            }
        } else {
            for (i = 0; i < np; i++) {          /* Load decoded data corresponds to each code word */
                d = *data++;
                pd[i] = d;
            }
        }

#if JD_FASTDECODE == 2
        {
            /* Create fast huffman decode table */
            unsigned int span, td, ti;
            uint16_t *tbl_ac = 0;
            uint8_t *tbl_dc = 0;

            if (cls) {
                tbl_ac = alloc_pool(jd, HUFF_LEN * sizeof(uint16_t));   /* LUT for AC elements */
                if (!tbl_ac) {
                    return JDR_MEM1;    /* Err: not enough memory */
                }
                jd->hufflut_ac[num] = tbl_ac;
                memset(tbl_ac, 0xFF, HUFF_LEN * sizeof(uint16_t));      /* Default value (0xFFFF: may be long code) */
            } else {
                tbl_dc = alloc_pool(jd, HUFF_LEN * sizeof(uint8_t));    /* LUT for AC elements */
                if (!tbl_dc) {
                    return JDR_MEM1;    /* Err: not enough memory */
                }
                jd->hufflut_dc[num] = tbl_dc;
                memset(tbl_dc, 0xFF, HUFF_LEN * sizeof(uint8_t));       /* Default value (0xFF: may be long code) */
            }
            for (i = b = 0; b < HUFF_BIT; b++) {    /* Create LUT */
                for (j = pb[b]; j; j--) {
                    ti = ph[i] << (HUFF_BIT - 1 - b) & HUFF_MASK;   /* Index of input pattern for the code */
                    if (cls) {
                        td = pd[i++] | ((b + 1) << 8);  /* b15..b8: code length, b7..b0: zero run and data length */
                        for (span = 1 << (HUFF_BIT - 1 - b); span; span--, tbl_ac[ti++] = (uint16_t)td) ;
                    } else {
                        td = pd[i++] | ((b + 1) << 4);  /* b7..b4: code length, b3..b0: data length */
                        for (span = 1 << (HUFF_BIT - 1 - b); span; span--, tbl_dc[ti++] = (uint8_t)td) ;
                    }
                }
            }
            jd->longofs[num][cls] = i;  /* Code table offset for long code */
        }
#endif
    }

    return JDR_OK;
}

/*-----------------------------------------------------------------------*/
/* Apply Inverse-DCT in Arai Algorithm (see also aa_idct.png)            */
/*-----------------------------------------------------------------------*/

static void block_idct(
    int32_t *src,   /* Input block data (de-quantized and pre-scaled for Arai Algorithm) */
    jd_yuv_t *dst   /* Pointer to the destination to store the block as byte array */
)
{
    const int32_t M13 = (int32_t)(1.41421 * 4096), M2 = (int32_t)(1.08239 * 4096), M4 = (int32_t)(2.61313 * 4096), M5 = (int32_t)(1.84776 * 4096);
    int32_t v0, v1, v2, v3, v4, v5, v6, v7;
    int32_t t10, t11, t12, t13;
    int i;

    /* Process columns */
    for (i = 0; i < 8; i++) {
        v0 = src[8 * 0];    /* Get even elements */
        v1 = src[8 * 2];
        v2 = src[8 * 4];
        v3 = src[8 * 6];

        t10 = v0 + v2;      /* Process the even elements */
        t12 = v0 - v2;
        t11 = (v1 - v3) * M13 >> 12;
        v3 += v1;
        t11 -= v3;
        v0 = t10 + v3;
        v3 = t10 - v3;
        v1 = t11 + t12;
        v2 = t12 - t11;

        v4 = src[8 * 7];    /* Get odd elements */
        v5 = src[8 * 1];
        v6 = src[8 * 5];
        v7 = src[8 * 3];

        t10 = v5 - v4;      /* Process the odd elements */
        t11 = v5 + v4;
        t12 = v6 - v7;
        v7 += v6;
        v5 = (t11 - v7) * M13 >> 12;
        v7 += t11;
        t13 = (t10 + t12) * M5 >> 12;
        v4 = t13 - (t10 * M2 >> 12);
        v6 = t13 - (t12 * M4 >> 12) - v7;
        v5 -= v6;
        v4 -= v5;

        src[8 * 0] = v0 + v7;   /* Write-back transformed values */
        src[8 * 7] = v0 - v7;
        src[8 * 1] = v1 + v6;
        src[8 * 6] = v1 - v6;
        src[8 * 2] = v2 + v5;
        src[8 * 5] = v2 - v5;
        src[8 * 3] = v3 + v4;
        src[8 * 4] = v3 - v4;

        src++;  /* Next column */
    }

    /* Process rows */
    src -= 8;
    for (i = 0; i < 8; i++) {
        v0 = src[0] + (128L << 8);  /* Get even elements (remove DC offset (-128) here) */
        v1 = src[2];
        v2 = src[4];
        v3 = src[6];

        t10 = v0 + v2;              /* Process the even elements */
        t12 = v0 - v2;
        t11 = (v1 - v3) * M13 >> 12;
        v3 += v1;
        t11 -= v3;
        v0 = t10 + v3;
        v3 = t10 - v3;
        v1 = t11 + t12;
        v2 = t12 - t11;

        v4 = src[7];                /* Get odd elements */
        v5 = src[1];
        v6 = src[5];
        v7 = src[3];

        t10 = v5 - v4;              /* Process the odd elements */
        t11 = v5 + v4;
        t12 = v6 - v7;
        v7 += v6;
        v5 = (t11 - v7) * M13 >> 12;
        v7 += t11;
        t13 = (t10 + t12) * M5 >> 12;
        v4 = t13 - (t10 * M2 >> 12);
        v6 = t13 - (t12 * M4 >> 12) - v7;
        v5 -= v6;
        v4 -= v5;

        /* Descale the transformed values 8 bits and output a row */
#if JD_FASTDECODE >= 1
        dst[0] = (int16_t)((v0 + v7) >> 8);
        dst[7] = (int16_t)((v0 - v7) >> 8);
        dst[1] = (int16_t)((v1 + v6) >> 8);
        dst[6] = (int16_t)((v1 - v6) >> 8);
        dst[2] = (int16_t)((v2 + v5) >> 8);
        dst[5] = (int16_t)((v2 - v5) >> 8);
        dst[3] = (int16_t)((v3 + v4) >> 8);
        dst[4] = (int16_t)((v3 - v4) >> 8);
#else
        dst[0] = BYTECLIP((v0 + v7) >> 8);
        dst[7] = BYTECLIP((v0 - v7) >> 8);
        dst[1] = BYTECLIP((v1 + v6) >> 8);
        dst[6] = BYTECLIP((v1 - v6) >> 8);
        dst[2] = BYTECLIP((v2 + v5) >> 8);
        dst[5] = BYTECLIP((v2 - v5) >> 8);
        dst[3] = BYTECLIP((v3 + v4) >> 8);
        dst[4] = BYTECLIP((v3 - v4) >> 8);
#endif

        dst += 8;
        src += 8; /* Next row */
    }
}

int jd_get_hc(JHUFF *huff, uint32_t dreg, uint8_t dbit, uint8_t *val)
{
    uint8_t i, n_codes;
    uint16_t n_bits;

    const uint8_t *hb = huff->huffbits;
    const uint16_t *hc = huff->huffcode;
    const uint8_t *hd = huff->huffdata;

    /* Incremental search for all codes */
    for (i = 1; i <= 16; i++) {
        n_codes = *hb++;
        n_bits = dreg >> (32 - i);
        while (n_codes--)  {
            if (n_bits == *hc) {  /* Compare code */
                if (i > dbit) {
                    return 0;
                }
                *val = *hd;
                return i;
            }
            hc++;
            hd++;
        }
    }

    return 0;
}

int yuv_to_gray(void *buf, int yy, int cb, int cr)
{
    return 1;
}

int yuv_to_rgb888(void *buf, int yy, int cb, int cr)
{
    return 3;
}

int yuv_to_bgr888(void *buf, int yy, int cb, int cr)
{
    return 3;
}

int yuv_to_rgb565(void *buf, int yy, int cb, int cr)
{
    return 2;
}

int yuv_to_bgr565(void *buf, int yy, int cb, int cr)
{
    return 3;
}

int yuv_to_argb888(void *buf, int yy, int cb, int cr)
{
    return 4;
}

int yuv_to_abgr888(void *buf, int yy, int cb, int cr)
{
    return 4;
}

void yuv444_to_rgb888(JDEC *jd)
{
    uint8_t *pix = (uint8_t *)jd->workbuf;
    jd_yuv_t *py, *pcb, *pcr;
    int yy, cb, cr;

    /* In YUV444, each pixel has its own Y, Cb, Cr values */
    py  = jd->mcubuf;        // Y block start
    pcb = py + 64;           // Cb block start
    pcr = pcb + 64;          // Cr block start

    for (unsigned int iy = 0; iy < 8; iy++) {
        for (unsigned int ix = 0; ix < 8; ix++) {
            yy = *py++;
            cb = *pcb++ - 128;
            cr = *pcr++ - 128;

            *pix++ = ycbcr2r(yy, cb, cr);
            *pix++ = ycbcr2g(yy, cb, cr);
            *pix++ = ycbcr2b(yy, cb, cr);
        }
    }

    JD_LOG("RGB888 (YUV444):");
    pix = (uint8_t *)jd->workbuf;
    JD_RGBDUMP(pix, 3 * 64);
}


void yuv422_to_rgb888(JDEC *jd)
{
    int iy, ix, icmp;
    jd_yuv_t *py, *pcb, *pcr;
    uint8_t *pix = (uint8_t *)jd->workbuf;

    int y_block_col, y_block_row;
    int yy, cb, cr;

    // 4 blocks: Y1, Y2, Cb, Cr → 16x8 Y region
    pcb = jd->mcubuf + 64 * 2;   // Cb block starts after Y1 + Y2
    pcr = pcb + 64;              // Cr block starts after Cb

    // Loop through the two Y blocks (icmp = 0: left, 1: right)
    for (icmp = 0; icmp < 2; icmp++) {
        py = jd->mcubuf + icmp * 64;
        pix = (uint8_t *)jd->workbuf;

        // Block positions: 0=(0,0), 1=(8,0)
        y_block_col = (icmp & 1) << 3;  // 0 or 8
        y_block_row = 0;                // Only one row of Y blocks

        for (iy = 0; iy < 8; iy++) {
            for (ix = 0; ix < 8; ix++) {
                int x_abs = y_block_col + ix;
                int y_abs = y_block_row + iy;

                // Cb/Cr sampled at 2:1 horizontally, 1:1 vertically → 8x8 UV blocks
                int uv_idx = (y_abs << 3) + (x_abs >> 1);

                yy = *py++;
                cb = pcb[uv_idx] - 128;
                cr = pcr[uv_idx] - 128;

                *pix++ = ycbcr2r(yy, cb, cr);
                *pix++ = ycbcr2g(yy, cb, cr);
                *pix++ = ycbcr2b(yy, cb, cr);
            }
        }

        JD_LOG("RGB888 (YUV422):");
        pix = (uint8_t *)jd->workbuf;
        JD_RGBDUMP(pix, 3 * 64);
    }
}

void yuv400_to_rgb888(JDEC *jd)
{
    uint8_t *pix;
    jd_yuv_t *py;
    int yy, cb = 0, cr = 0;

    /* Build a RGB MCU from discrete comopnents */
    pix = (uint8_t *)jd->workbuf;
    py = jd->mcubuf;
    for (unsigned int iy = 0; iy < 8; iy++) {
        for (unsigned int ix = 0; ix < 8; ix++) {
            yy = *py++;   /* Get Y component */
            *pix++ = ycbcr2r(yy, cb, cr);
            *pix++ = ycbcr2g(yy, cb, cr);
            *pix++ = ycbcr2b(yy, cb, cr);
        }
    }

    JD_LOG("RGB888:");
    pix = (uint8_t *)jd->workbuf;
    JD_RGBDUMP(pix, 3*64);
}

void yuv420_to_rgb888(JDEC *jd)
{
    int iy, ix, icmp;
    jd_yuv_t *py, *pcb, *pcr;
    uint8_t *pix = (uint8_t *)jd->workbuf;

    int y_block_col, y_block_row;
    int yy, cb, cr;

    // 6 blocks: Y1,Y2,Y3,Y4,Cb,Cr
    pcb = jd->mcubuf + 64 * 4;     // Cb block起点
    pcr = pcb + 64;                // Cr block起点

    for (icmp = 0; icmp < 4; icmp++) {
        py = jd->mcubuf + icmp * 64;
        pix = (uint8_t *)jd->workbuf;

        // 0: (0, 0), 1: (8, 0), 2: (0, 8), 3: (8, 8)
        y_block_col = (icmp & 1) << 3;
        y_block_row = (icmp >> 1) << 3;

        for (iy = 0; iy < 8; iy++) {
            for (ix = 0; ix < 8; ix++) {
                int x_abs = y_block_col + ix;
                int y_abs = y_block_row + iy;
                int uv_idx = ((y_abs >> 1) << 3) + (x_abs >> 1);    // y/2 * 8 + x/2

                yy = *py++;
                cb = pcb[uv_idx] - 128;
                cr = pcr[uv_idx] - 128;

                *pix++ = ycbcr2r(yy, cb, cr);
                *pix++ = ycbcr2g(yy, cb, cr);
                *pix++ = ycbcr2b(yy, cb, cr);
            }
        }

        JD_LOG("RGB888:");
        pix = (uint8_t *)jd->workbuf;
        JD_RGBDUMP(pix, 3*64);
    }
}

JRESULT jd_output(JDEC *jd, jd_yuv_t *mcubuf, uint8_t n_cmp)
{
    int cmp, i;
    JCOMP *component;
    jd_yuv_t *p;
    int32_t *tmp = (int32_t *)jd->workbuf;

    for (cmp = 0; cmp < n_cmp; cmp++) {
        component = &jd->component[cmp];
        p = &jd->mcubuf[cmp * 64];
        JD_LOG("Component %d:", cmp);
        JD_LOG("P:");
        JD_INTDUMP(p, 64);
        for (i = 0; i < 64; i++) {
            if (p[i]) {
                tmp[i] = p[i] * component->qttbl[i] >> 8;
            } else {
                tmp[i] = 0;
            }
        }
        JD_LOG("TMP:");
        JD_INTDUMP(tmp, 64);
        block_idct(tmp, p);
        JD_LOG("P:");
        JD_INTDUMP(p, 64);
    }

    /* Convert to RGB */
    if ((jd->ncomp == 1) && (jd->msx == 1) && (jd->msy == 1)) {
        yuv400_to_rgb888(jd);
    } else if ((jd->ncomp == 3) && (jd->msx == 1) && (jd->msy == 1)) {
        yuv444_to_rgb888(jd);
    } else if ((jd->ncomp == 3) && (jd->msx == 2) && (jd->msy == 2)) {
        yuv420_to_rgb888(jd);
    } else if ((jd->ncomp == 3) && (jd->msx == 2) && (jd->msy == 1)) {
        yuv422_to_rgb888(jd);
    }

    return JDR_OK;
}

/*-----------------------------------------------------------------------*/
/* Analyze the JPEG image and Initialize decompressor object             */
/*-----------------------------------------------------------------------*/

#define LDB_WORD(ptr)       (uint16_t)(((uint16_t)*((uint8_t*)(ptr))<<8)|(uint16_t)*(uint8_t*)((ptr)+1))


JRESULT jd_prepare(
    JDEC *jd,               /* Blank decompressor object */
    size_t (*infunc)(JDEC *, uint8_t *, size_t), /* JPEG strem input function */
    void *pool,             /* Working buffer for the decompression session */
    size_t sz_pool,         /* Size of working buffer */
    void *dev               /* I/O device identifier for the session */
)
{
    uint8_t *seg, b;
    uint16_t marker;
    unsigned int n, i, ofs;
    size_t len;
    JRESULT rc;


    memset(jd, 0, sizeof(JDEC));    /* Clear decompression object (this might be a problem if machine's null pointer is not all bits zero) */
    jd->pool = pool;        /* Work memroy */
    jd->sz_pool = sz_pool;  /* Size of given work memory */
    jd->infunc = infunc;    /* Stream input function */
    jd->device = dev;       /* I/O device identifier */

    jd->inbuf = seg = alloc_pool(jd, JD_SZBUF);     /* Allocate stream input buffer */
    if (!seg) {
        return JDR_MEM1;
    }

    ofs = marker = 0;       /* Find SOI marker */
    do {
        if (jd->infunc(jd, seg, 1) != 1) {
            return JDR_INP;    /* Err: SOI was not detected */
        }
        ofs++;
        marker = marker << 8 | seg[0];
    } while (marker != 0xFFD8);

    for (;;) {              /* Parse JPEG segments */
        JD_LOG("\n---");

        /* Get a JPEG marker */
        if (jd->infunc(jd, seg, 4) != 4) {
            return JDR_INP;
        }

        marker = LDB_WORD(seg);     /* Marker */
        if ((marker >> 8) != 0xFF) {
            return JDR_FMT1;
        }
        marker = marker & 0xFF;

        len = LDB_WORD(seg + 2);    /* Length field */
        if (len <= 2) {
            return JDR_FMT1;
        }
        len -= 2;           /* Segent content size */
        ofs += 4 + len;     /* Number of bytes loaded */

        switch (marker) {
        case 0xC0:  /* SOF0 (baseline JPEG) */
        case 0xC4:
        case 0xDA:
        case 0xDB:
        case 0xDD:
            if (len > JD_SZBUF) {
                JD_LOG("Insufficient buffer size %lld > %d", len, JD_SZBUF);
                return JDR_MEM2;
            }
            if (jd->infunc(jd, seg, len) != len) {
                return JDR_INP;
            }

            JD_LOG("Process segment marker %02X,%lld:", marker, len);
            JD_HEXDUMP(seg, len);
            switch (marker) {
            case 0xC0:  /* SOF0 (baseline JPEG) */
                jd->width = LDB_WORD(&seg[3]);      /* Image width in unit of pixel */
                jd->height = LDB_WORD(&seg[1]);     /* Image height in unit of pixel */
                jd->ncomp = seg[5];                 /* Number of color components */
                if (jd->ncomp != 3 && jd->ncomp != 1) {
                    return JDR_FMT3;    /* Err: Supports only Grayscale and Y/Cb/Cr */
                }

                /* Check each image component */
                for (i = 0; i < jd->ncomp; i++) {
                    b = seg[7 + 3 * i];                         /* Get sampling factor */
                    if (i == 0) {   /* Y component */
                        /**/
                        if (b != 0x11 && b != 0x22 && b != 0x21) {  /* Check sampling factor */
                            return JDR_FMT3;                    /* Err: Supports only 4:4:4, 4:2:0 or 4:2:2 */
                        }
                        jd->msx = b >> 4;
                        jd->msy = b & 15;     /* Size of MCU [blocks] */
                    } else {        /* Cb/Cr component */
                        if (b != 0x11) {
                            return JDR_FMT3;    /* Err: Sampling factor of Cb/Cr must be 1 */
                        }
                    }
                    jd->qtid[i] = seg[8 + 3 * i];               /* Get dequantizer table ID for this component */
                    if (jd->qtid[i] > 3) {
                        return JDR_FMT3;    /* Err: Invalid ID */
                    }
                }

                JD_LOG("SOF0 start of frame, w: %u, h: %u, ncomp: %u, msx: %u, msy: %u, qtid:", jd->width, jd->height, jd->ncomp, jd->msx, jd->msy);
                JD_HEXDUMP(jd->qtid, jd->ncomp);
                break;

            case 0xDD:  /* DRI - Define Restart Interval */
                jd->nrst = LDB_WORD(seg);   /* Get restart interval (MCUs) */
                JD_LOG("DRI define restart interval: %u", jd->nrst);
                break;

            case 0xC4:  /* DHT - Define Huffman Tables */
                JD_LOG("DHT define huffman tables:");
                rc = create_huffman_tbl(jd, seg, len);  /* Create huffman tables */
                if (rc) {
                    return rc;
                }
                break;

            case 0xDB:  /* DQT - Define Quaitizer Tables */
                JD_LOG("DQT define quantizer tables:");
                rc = create_qt_tbl(jd, seg, len);   /* Create de-quantizer tables */
                if (rc) {
                    return rc;
                }
                break;

            case 0xDA:  /* SOS - Start of Scan */
                JD_LOG("SOS start of scan:");
                if (!jd->width || !jd->height) {
                    return JDR_FMT1;    /* Err: Invalid image size */
                }
                if (seg[0] != jd->ncomp) {
                    return JDR_FMT3;    /* Err: Wrong color components */
                }

                /* Check if all tables corresponding to each components have been loaded */
                for (i = 0; i < jd->ncomp; i++) {
                    b = seg[2 + 2 * i]; /* Get huffman table ID */
                    if (b != 0x00 && b != 0x11) {
                        return JDR_FMT3;    /* Err: Different table number for DC/AC element */
                    }
                    n = i ? 1 : 0;                          /* Component class */
                    if (!jd->huffbits[n][0] || !jd->huffbits[n][1]) {   /* Check huffman table for this component */
                        return JDR_FMT1;                    /* Err: Nnot loaded */
                    }
                    if (!jd->qttbl[jd->qtid[i]]) {          /* Check dequantizer table for this component */
                        return JDR_FMT1;                    /* Err: Not loaded */
                    }
                }

                /* Allocate working buffer for MCU and pixel output */
                n = jd->msy * jd->msx;                      /* Number of Y blocks in the MCU */
                if (!n) {
                    return JDR_FMT1;    /* Err: SOF0 has not been loaded */
                }

                /* Y */
                for (i = 0; i < n; i++) {
                    jd->component[i].huff[0].huffbits = jd->huffbits[0][0];
                    jd->component[i].huff[0].huffcode = jd->huffcode[0][0];
                    jd->component[i].huff[0].huffdata = jd->huffdata[0][0];
                    jd->component[i].huff[1].huffbits = jd->huffbits[0][1];
                    jd->component[i].huff[1].huffcode = jd->huffcode[0][1];
                    jd->component[i].huff[1].huffdata = jd->huffdata[0][1];
                    jd->component[i].qttbl = jd->qttbl[jd->qtid[0]];
                    jd->component[i].dcv = &jd->dcv[0];

                    JD_LOG("huff[%d]", i);
                }

                /* CrCb */
                if (jd->ncomp == 3) {
                    for (i = 0; i < 2; i++) {
                        jd->component[n + i].huff[0].huffbits = jd->huffbits[1][0];
                        jd->component[n + i].huff[0].huffcode = jd->huffcode[1][0];
                        jd->component[n + i].huff[0].huffdata = jd->huffdata[1][0];
                        jd->component[n + i].huff[1].huffbits = jd->huffbits[1][1];
                        jd->component[n + i].huff[1].huffcode = jd->huffcode[1][1];
                        jd->component[n + i].huff[1].huffdata = jd->huffdata[1][1];
                        jd->component[n + i].qttbl = jd->qttbl[jd->qtid[i + 1]];
                        jd->component[n + i].dcv = &jd->dcv[i + 1];

                        JD_LOG("huff[%d]", n + i);
                    }
                }

                len = n * 64 * 2 + 64;                      /* Allocate buffer for IDCT and RGB output */
                if (len < 256) {
                    len = 256;    /* but at least 256 byte is required for IDCT */
                }
                if (len < n * 64 * 4) {
                    len = n * 64 * 4;    /* support ARGB8888 */
                }
                jd->workbuf = alloc_pool(jd, len);          /* and it may occupy a part of following MCU working buffer for RGB output */
                if (!jd->workbuf) {
                    return JDR_MEM1;    /* Err: not enough memory */
                }
                jd->mcubuf = alloc_pool(jd, (n + 2) * 64 * sizeof(jd_yuv_t));   /* Allocate MCU working buffer */
                if (!jd->mcubuf) {
                    return JDR_MEM1;    /* Err: not enough memory */
                }

                /* Align stream read offset to JD_SZBUF */
                if (ofs %= JD_SZBUF) {
                    jd->dctr = jd->infunc(jd, seg + ofs, (size_t)(JD_SZBUF - ofs));
                }
                jd->dptr = seg + ofs - (JD_FASTDECODE ? 0 : 1);
                JD_HEXDUMP(jd->dptr, jd->dctr);

                return JDR_OK;      /* Initialization succeeded. Ready to decompress the JPEG image. */
            }
            break;
        case 0xC1:  /* SOF1 */
        case 0xC2:  /* SOF2 */
        case 0xC3:  /* SOF3 */
        case 0xC5:  /* SOF5 */
        case 0xC6:  /* SOF6 */
        case 0xC7:  /* SOF7 */
        case 0xC9:  /* SOF9 */
        case 0xCA:  /* SOF10 */
        case 0xCB:  /* SOF11 */
        case 0xCD:  /* SOF13 */
        case 0xCE:  /* SOF14 */
        case 0xCF:  /* SOF15 */
        case 0xD9:  /* EOI */
            return JDR_FMT3;    /* Unsuppoted JPEG standard (may be progressive JPEG) */
        default:    /* Unknown segment (comment, exif or etc..) */
            JD_LOG("Skip segment marker %02X,%lld", marker, len);
            /* Skip segment data (null pointer specifies to remove data from the stream) */
            if (jd->infunc(jd, NULL, len) != len) {
                return JDR_INP;
            }
            break;
        }
    }
}

JRESULT jd_decomp(JDEC *jd, jd_outfunc_t outfunc, uint8_t scale)
{
    size_t dc = jd->dctr;
    uint8_t *dp = jd->dptr;
    uint8_t last_d = 0, d = 0, dbit = 0, cnt = 0, cmp = 0, cls = 0, bl0, bl1, val, zeros;
    uint32_t dreg = 0;
    int ebits, dcac;
    uint8_t bits_threshold = 15, n_y, n_cmp;
    int block_id = 0, mcu_id = 0, total_mcus;
    bool next_huff = true;

    JCOMP *component = &jd->component[cmp];
    jd_yuv_t *mcubuf = &jd->mcubuf[cmp * 64];

    n_y = jd->msy * jd->msx; /* Number of Y blocks in the MCU */
    if (jd->ncomp == 1) {
        n_cmp = n_y;
    } else if (jd->ncomp == 3) {
        n_cmp = n_y + 2;
    } else {
        return JDR_FMT1;    /* Err: Supports only Grayscale and Y/Cb/Cr */
    }

    memset(mcubuf + 1, 0, 63 * sizeof(jd_yuv_t));

    total_mcus = (jd->width + 8 * jd->msx - 1) / (8 * jd->msx) * ((jd->height + 8 * jd->msy - 1) / (8 * jd->msy));

    /* n_y: 1, 2, 4, ncomp: 1, 3 */
    while (1) {
        if (dc == 0) {
            dp = jd->inbuf; /* Top of input buffer */
            dc = jd->infunc(jd, dp, JD_SZBUF);
            if (!dc) {
                JD_LOG("No more data, %d", dbit);
                return 0 - (int)JDR_INP;    /* Err: read error or wrong stream termination */
            }
        }

        if (dbit > 24) {
            JD_LOG("No more buffer");
            return 0 - (int)JDR_FMT4;    /* Err: read error or wrong stream termination */
        }

        last_d = d;
        d = *dp;
        dp++;
        dc--;

        if (d == 0xFF) {
            continue;
        } else if (last_d == 0xFF) {
            JD_LOG("Found marker %02X", d);
            if (d == 0x00) {
                JD_LOG("Padding byte");
                dreg = (dreg & ~(0xFFFFFFFF >> dbit)) | ((uint32_t)0xFF << (32 - dbit - 8));
                dbit += 8;
            } else {
                if (d == 0xD9) {
                    JD_LOG("EOI marker");
                    bits_threshold = 0;
                } else {
                    if ((d >= 0xD0) && (d <= 0xD7)) {
                        JD_LOG("RST marker %02X", d);
                    }
                    continue;
                }
            }
        } else {
            dreg = (dreg & ~(0xFFFFFFFF >> dbit)) | ((uint32_t)d << (32 - dbit - 8));
            dbit += 8;
        }

        JD_LOG("Buffer: %08X %u %02X", dreg, dbit, d);

        while (dbit > bits_threshold) {
            if (next_huff) {
                cls = !!cnt;

                JD_LOG("mcu %u/%u, cmp %u, block %u, %s table, cls %d, cnt %d, dreg %08X, dbit %u",
                       mcu_id, total_mcus, cmp, block_id, cls == 0 ? "DC" : "AC", cls, cnt, dreg, dbit);

                bl0 = jd_get_hc(&component->huff[cls], dreg, dbit, &val);
                if (!bl0) {
                    JD_LOG("bl0 Huffman code too short: %08X %u", dreg, dbit);
                    return JDR_FMT1;
                }

                dbit -= bl0;
                dreg <<= bl0;
                next_huff = false;

                JD_LOG("processing huff, bl0 %d, val %02X", bl0, val);

                if (val != 0) {
                    continue;
                }
            }

            if (!next_huff) {
                JD_LOG("processing bits, cnt %d val %02X", cnt, val);

                if ((val != 0) || (cnt == 0)) {
                    zeros = val >> 4;
                    bl1 = val & 0x0F;

                    if (dbit < bl1) {
                        JD_LOG("bl1 Huffman code too short: %08X %u < %u", dreg, dbit, bl1);
                        return JDR_FMT1;
                    }

                    cnt += zeros;

                    if (bl1) {
                        ebits = (int)(dreg >> (32 - bl1));
                        if (!(dreg & 0x80000000)) {
                            ebits -= (1 << bl1) - 1;    /* Restore negative value if needed */
                        }
                    } else {
                        ebits = 0;
                    }
                    if ((cnt == 0) || bl1) {
                        if (cnt == 0) {
                            /* DC component */
                            dcac = *component->dcv + ebits;
                            *component->dcv = dcac;
                        } else {
                            dcac = ebits;
                            /* AC component */
                        }

                        /* reverse zigzag */
                        mcubuf[Zig[cnt]] = dcac;

                        dbit -= bl1;
                        dreg <<= bl1;
                        cnt += 1;
                    }
                } else {
                    /* EOB detected */
                    zeros = 0;
                    cnt = 64;
                    ebits = 0;
                }

                JD_LOG("Found Huffman code: %08X %u | %u %02X %d %d %d", dreg, dbit, bl0, val, *component->dcv, ebits, dcac);
                if (cnt == 64) {
                    cnt = 0;

                    JD_INTDUMP(mcubuf, 64);
                    JD_LOG("");

                    block_id++;
                    cmp++;
                    if (cmp >= n_cmp) {
                        /* TODO: dequantize -> idct -> output */
                        jd_output(jd, jd->mcubuf, n_cmp);

                        cmp = 0;
                        mcu_id++;
                        if (mcu_id >= total_mcus) {
                            JD_LOG("All MCUs processed (%u padding bits: %X)", dbit, dreg >> (32 - dbit));
                            return JDR_OK;
                        }
                    }
                    component = &jd->component[cmp];
                    mcubuf = &jd->mcubuf[cmp * 64];

                    memset(mcubuf + 1, 0, 63 * sizeof(jd_yuv_t));
                }
                next_huff = true;
            }
        }
    }
}

const char *jd_code2bin(char *buf, int code, int bits)
{
    int i;

    for (i = 0; i < bits; i++) {
        buf[bits - 1 - i] = (code & (1 << i)) ? '1' : '0';
    }
    buf[bits] = '\0';

    return buf;
}

void jd_log(JDEC *jd)
{
    int i, j, cls, num, total_codes;
    uint8_t *hb;
    uint16_t *hc;
    uint8_t *hd;
    char buf[32];

    int32_t *p;

    JD_LOG("\n\n---");

    for (cls = 0; cls < 2; cls++) {
        for (num = 0; num < 2; num++) {
            hb = jd->huffbits[num][cls];
            hc = jd->huffcode[num][cls];
            hd = jd->huffdata[num][cls];
            if (hb == NULL || hc == NULL || hd == NULL) {
                continue;
            }

            JD_LOG("\nTable Number: %d, Class: %s, Huffman Table [%d][%d]:", num, cls ? "AC" : "DC", num, cls);
            JD_LOG("index, bits, code(hex), code(bin), data");
            j = 0;
            for (i = 0; i < 16; i++) {
                total_codes = hb[i];
                while (total_codes--)  {
                    JD_LOG("%3d, %2d, %04X, %17s, %02X", j, i + 1, hc[j], jd_code2bin(buf, hc[j], i + 1), hd[j]);
                    j++;
                }
            }
        }
    }

    for (i = 0; i < 4; i++) {
        if (jd->qttbl[i]) {
            p = jd->qttbl[i];
            JD_LOG("\nQuantization Table ID: %d", i);
            JD_INTDUMP(p, 64);
        }
    }

    JD_LOG("\nIpsf:");
    JD_INTDUMP(Ipsf, 64);

    JD_LOG("\nZigZag:");
    JD_INTDUMP(Zig, 64);
}
