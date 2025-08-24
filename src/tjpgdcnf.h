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
#ifndef DEF_TJPGDCNF
#define DEF_TJPGDCNF

/* Specifies size of stream input buffer */
#ifndef JD_SZBUF
#   define JD_SZBUF                     256
#endif

/* Use table conversion for saturation arithmetic. A bit faster, but increases 1 KB of code size.
/  0: Disable
/  1: Enable
*/
#ifndef JD_TBLCLIP
#   define JD_TBLCLIP                   1
#endif

/* Optimization level
/  0: N/A
/  1: + 32-bit barrel shifter. Suitable for 32-bit MCUs.
/  2: + Table conversion for huffman decoding (wants 6 << HUFF_BIT bytes of RAM)
*/
#ifndef JD_FASTDECODE
#   define JD_FASTDECODE                1
#endif

/* Debugging options
/  0: Disable
/  1: Enable
*/
#ifndef JD_DEBUG
#   define JD_DEBUG                     1
#endif

#if JD_DEBUG
#include <stdio.h>
#define JD_LOG(x...)                    do {printf(x); printf("\n");} while(0)
#define JD_HEXDUMP(x, y)             do { \
    for (int i = 0; i < y; i++) { \
        if (i && i % 16 == 0) printf("\n"); \
        printf("%02X ", ((uint8_t*)x)[i]); \
    } \
    printf("\n"); \
} while(0)
#define JD_INTDUMP(x, y)             do { \
    for (int i = 0; i < y; i++) { \
        if (i && i % 8 == 0) printf("\n"); \
        printf("%5d ", x[i]); \
    } \
    printf("\n"); \
} while(0)
#define JD_RGBDUMP(x, y)             do { \
    for (int i = 0; i < y / 3; i++) { \
        if (i && i % 8 == 0) printf("\n"); \
        printf("(%3d,%3d,%3d) ", x[i * 3], x[i * 3 + 1], x[i * 3 + 2]); \
    } \
    printf("\n"); \
} while(0)
#else
#define JD_LOG(x...)                do {} while(0)
#define JD_HEXDUMP(x, y)            do {} while(0)
#define JD_INTDUMP(x, y)            do {} while(0)
#define JD_RGBDUMP(x, y)            do {} while(0)
#endif

#endif
