/*----------------------------------------------*/
/* TJpgDec System Configurations R0.03          */
/*----------------------------------------------*/

/* Specifies size of stream input buffer */
#ifndef JD_SZBUF
#   define JD_SZBUF                     256
#endif

/* Specifies color depth
/  32: ARGB8888 (32-bit/pix)
/  24: RGB888 (24-bit/pix)
/  16: RGB565 (16-bit/pix)
/  8: Grayscale (8-bit/pix)
*/
#ifndef JD_COLOR_DEPTH
#define JD_COLOR_DEPTH                  24
#endif

/* Specifies output pixel format.
/  0: RGB888 (24-bit/pix)
/  1: RGB565 (16-bit/pix)
/  2: Grayscale (8-bit/pix)
*/
// #define JD_FORMAT        2

#if     JD_COLOR_DEPTH == 8
#   define JD_FORMAT                    2
#   define JD_USE_INTERNAL_32BIT_PIXEL  0
#elif   JD_COLOR_DEPTH == 16
#   define JD_FORMAT                    1
#   define JD_USE_INTERNAL_32BIT_PIXEL  0
#elif   JD_COLOR_DEPTH == 24
#   define JD_FORMAT                    0
#   define JD_USE_INTERNAL_32BIT_PIXEL  0
#elif   JD_COLOR_DEPTH == 32
#   define JD_FORMAT                    0
#   define JD_USE_INTERNAL_32BIT_PIXEL  1
#endif

/* Switches output descaling feature.
/  0: Disable
/  1: Enable
*/
#ifndef JD_USE_SCALE
#   define JD_USE_SCALE                 0
#endif

/* Use table conversion for saturation arithmetic. A bit faster, but increases 1 KB of code size.
/  0: Disable
/  1: Enable
*/
#ifndef JD_TBLCLIP
#   define JD_TBLCLIP                   1
#endif

/* Optimization level
/  0: Basic optimization. Suitable for 8/16-bit MCUs.
/  1: + 32-bit barrel shifter. Suitable for 32-bit MCUs.
/  2: + Table conversion for huffman decoding (wants 6 << HUFF_BIT bytes of RAM)
*/
#ifndef JD_FASTDECODE
#   define JD_FASTDECODE                1
#endif

/* Swap Red and Blue channels when outputting to adapt different target displays.
/  0: Disable
/  1: Enable
*/
#ifndef JD_SWAP_RED_AND_BLUE
#   define JD_SWAP_RED_AND_BLUE         0
#endif

// Do not change this, it is the minimum size in bytes of the workspace needed by the decoder
#if JD_FASTDECODE == 0
#define TJPGD_WORKSPACE_SIZE 3100
#elif JD_FASTDECODE == 1
#define TJPGD_WORKSPACE_SIZE 3500
#elif JD_FASTDECODE == 2
#define TJPGD_WORKSPACE_SIZE (3500 + 6144)
#endif

#define JD_DEBUG                        1

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
#endif
