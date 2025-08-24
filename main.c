#include <stdio.h>
#include "tjpgd.h"



// Dummy input/output functions for TJpgDec
int32_t input_func(JDEC *jd, uint8_t *buf, int32_t len)
{
    FILE *fp = (FILE *)jd->device;

    JD_LOG("rd %d", len);
    if (buf) {
        return fread(buf, 1, (size_t)len, fp);
    } else {
        fseek(fp, len, SEEK_CUR);
        return len;
    }
}

int output_func(JDEC *jd, void *bitmap, JRECT *rect)
{
#if JD_DEBUG
    JD_LOG("Decoded rect: (%d,%d)-(%d,%d)", rect->left, rect->top, rect->right, rect->bottom);
#else
    // Output the decoded bitmap data
    uint8_t *pix = (uint8_t *)bitmap;

    int x, y, l;

    switch (jd->color) {
    case JD_GRAYSCALE:
    default:
        // Handle grayscale output
        l = 1;
        break;
    case JD_RGB565:
        // Handle RGB565 output
        l = 2;
        break;
    case JD_BGR565:
        // Handle BGR565 output
        l = 2;
        break;
    case JD_RGB888:
        // Handle RGB888 output
        l = 3;
        break;
    case JD_BGR888:
        // Handle BGR888 output
        l = 3;
        break;
    case JD_RGBA8888:
        // Handle RGBA8888 output
        l = 4;
        break;
    case JD_BGRA8888:
        // Handle BGRA8888 output
        l = 4;
        break;
    }
    printf("(%d,%d)-(%d,%d)\n", rect->left, rect->top, rect->right, rect->bottom);
    for (y = rect->top; y <= rect->bottom; y++) {
        for (x = rect->left; x <= rect->right; x++) {
            if (l == 2) {
                uint16_t v = *(uint16_t *)pix;
                int r = (v >> 11) & 0x1F;
                int g = (v >> 5) & 0x3F;
                int b = v & 0x1F;
                r = (r << 3) | (r >> 2);
                g = (g << 2) | (g >> 4);
                b = (b << 3) | (b >> 2);
                printf("(%3d,%3d,%3d) ", r, g, b);
                pix += 2;
            } else {
                int n = l;
                printf("(");
                while (n-- > 1) {
                    printf("%3d,", *pix++);
                }
                printf("%3d", *pix++);
                printf(") ");
            }
        }
        printf("\n");
    }
#endif

    return 1;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: %s <jpg_file>, \n", argv[0]);
        return 1;
    }

    FILE *fp = fopen(argv[1], "rb");
    if (!fp) {
        perror("Failed to open JPEG file");
        return 1;
    }

    uint8_t work[8192]; // Work buffer
    JDEC jd;
    JRESULT res;

    JCOLOR color;
    JRECT *rect = NULL, _rect;

    if (argc > 3) {
        int x, y, w, h;
        int ret = sscanf(argv[3], "%d,%d,%d,%d", &x, &y, &w, &h);
        if (ret == 4) {
            _rect.left = x;
            _rect.top = y;
            _rect.right = x + w - 1;
            _rect.bottom = y + h - 1;
            rect = &_rect;
        } else {
            fprintf(stderr, "Invalid rectangle format: %s\n", argv[3]);
            fclose(fp);
            return 1;
        }
    }

    if (argc > 2) {
        if (strcmp(argv[2], "grayscale") == 0) {
            color = JD_GRAYSCALE;
        } else if (strcmp(argv[2], "rgb565") == 0) {
            color = JD_RGB565;
        } else if (strcmp(argv[2], "bgr565") == 0) {
            color = JD_BGR565;
        } else if (strcmp(argv[2], "rgb888") == 0) {
            color = JD_RGB888;
        } else if (strcmp(argv[2], "bgr888") == 0) {
            color = JD_BGR888;
        } else if (strcmp(argv[2], "rgba8888") == 0) {
            color = JD_RGBA8888;
        } else if (strcmp(argv[2], "bgra8888") == 0) {
            color = JD_BGRA8888;
        } else {
            fprintf(stderr, "Unknown color format: %s\n", argv[2]);
            fclose(fp);
            return 1;
        }
    } else {
        color = JD_RGB888; // Default color format
    }

    printf("Preparing JPEG decoder...\n");
    res = jd_prepare(&jd, input_func, work, sizeof(work), fp);
    if (res != JDR_OK) {
        printf("Failed to prepare JPEG decoder %u\n", res);
        fclose(fp);
        return 1;
    }

    jd_set_color(&jd, color);

    printf("\n\n\n");

    printf("Starting JPEG decompression...\n");
    if (jd_decomp_rect(&jd, output_func, rect) != JDR_OK) {
        printf("Failed to decode JPEG image\n");
        fclose(fp);
        return 1;
    }

    fclose(fp);

    printf("\n\n\n");

    printf("sizeof(JDEC): %zu\n", sizeof(JDEC));
    printf("Memory Pool: %d\n", sizeof(work) - jd.sz_pool);
    printf("%s Total: %d\n", argv[1], sizeof(JDEC) + sizeof(work) - jd.sz_pool);

    return 0;
}
