#include <stdio.h>
#include "tjpgd.h"



// Dummy input/output functions for TJpgDec
size_t input_func(JDEC *jd, uint8_t *buf, size_t len) {
    FILE *fp = (FILE *)jd->device;

    printf("rd %zu\n", len);
    if (buf) {
        return fread(buf, 1, len, fp);
    } else {
        fseek(fp, len, SEEK_CUR);
        return len;
    }
}

int output_func(JDEC *jd, void *bitmap, JRECT *rect) {
    printf("Decoded rect: (%d,%d)-(%d,%d)\n", rect->left, rect->top, rect->right, rect->bottom);

    // Output the decoded bitmap data
    uint8_t *data = (uint8_t *)bitmap;
    int x, y, w, h;

    w = rect->right - rect->left + 1;
    h = rect->bottom - rect->top + 1;
    for (y = 0; y < h; y++) {
        for (x = 0; x < w; x++) {
            uint8_t pixel = data[y * w + x];
            printf("(%02d,%02d): %d, ", rect->left + x, rect->top + y, pixel);
        }
    }

    printf("\n");
    return 1; // Continue decoding
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <jpg_file>\n", argv[0]);
        return 1;
    }

    FILE *fp = fopen(argv[1], "rb");
    if (!fp) {
        perror("Failed to open JPEG file");
        return 1;
    }

    uint8_t work[4096]; // Work buffer
    JDEC jd;
    JRESULT res;

    printf("Preparing JPEG decoder...\n");
    res = jd_prepare(&jd, input_func, work, sizeof(work), fp);
    if (res != JDR_OK) {
        printf("Failed to prepare JPEG decoder %u\n", res);
        fclose(fp);
        return 1;
    }

    jd_log(&jd);
    // jd_test(&jd);

    // printf("\n\n\n");

    printf("Starting JPEG decompression...\n");
    if (jd_decomp(&jd, output_func, 0) != JDR_OK) {
        printf("Failed to decode JPEG image\n");
        fclose(fp);
        return 1;
    }

    fclose(fp);
    return 0;
}
