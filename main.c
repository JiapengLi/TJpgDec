#include <stdio.h>
#include "tjpgd.h"

// Dummy input/output functions for TJpgDec
size_t input_func(JDEC *jd, uint8_t *buf, size_t len) {
    FILE *fp = (FILE *)jd->device;
    if (buf) {
        return fread(buf, 1, len, fp);
    } else {
        fseek(fp, len, SEEK_CUR);
        return len;
    }
}

int output_func(JDEC *jd, void *bitmap, JRECT *rect) {
    printf("Decoded rect: (%d,%d)-(%d,%d)\n", rect->left, rect->top, rect->right, rect->bottom);
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

    res = jd_prepare(&jd, input_func, work, sizeof(work), fp);
    if (res != JDR_OK) {
        printf("Failed to prepare JPEG decoder %u\n", res);
        fclose(fp);
        return 1;
    }

    if (jd_decomp(&jd, output_func, 0) != JDR_OK) {
        printf("Failed to decode JPEG image\n");
        fclose(fp);
        return 1;
    }

    fclose(fp);
    return 0;
}
