# TJpgDec (Tiny JPEG Decompressor) Variant

## Overview

This project is a variant of the original [TJpgDec](https://elm-chan.org/fsw/tjpgd/), designed for 32-bit MCUs. It features runtime-configurable color modes, reduced RAM usage, and support for additional color formats.

## Key Differences from Original TJpgDec

- **Runtime configurable color mode**
- **Lower RAM footprint** (uses more ROM for added features)
    - Single copy of `jd->workbuf`, max 4×64 bytes
    - Supports very small cache for `jd->inbuf` (bufferless operation possible)
- **Scale feature removed**
- **Fast single pixel extraction** (skips dequantization, IDCT, and color conversion if MCU/block is irrelevant)
- **Additional color formats:** Grayscale, RGB565, BGR565, RGB888, BGR888, RGBA8888, BGRA8888
- **Fewer macro variables**
- **Single entry point for buffer loading**

### Limitations

- Targeted for 32-bit MCUs; 8-bit and 16-bit MCUs may not be supported
- Strict 8×8 block output, even in 16×16 or 16×8 MCU mode
- For YUV 4:2:0 or 4:2:2, 8×8 block output sequence is a subsequence of standard 16×16 or 16×8 output

## TODO

- Clip output images to fit actual image boundaries
- Optimize for ARM and RISC-V architectures
- Further memory optimization
- Random access to JPEG files
- More readable error codes
- Make `JD_FASTDECODE = 2` functional
- Benchmarking and performance analysis

## Build Instructions

```bash
make clean all
```

> **Note:** On Windows, use MSYS2 or WSL for a compatible build environment.

## Testing

```bash
./test.sh
```

## Code Formatting

To auto-format source files, use:

```bash
astyle \
    --suffix=none \
    --style=kr \
    --indent=spaces=4 \
    --pad-oper \
    --pad-header \
    --pad-comma \
    --unpad-paren \
    --unpad-brackets \
    --align-pointer=name \
    --align-reference=name \
    --max-code-length=160 \
    --break-after-logical \
    --lineend=linux \
    --convert-tabs \
    --verbose \
    --add-braces \
    ./src/tjpgd.c main.c
```

## Acknowledgments

- [Original TJpgDec](https://elm-chan.org/fsw/tjpgd/)
- [Arm-2D](https://github.com/ARM-software/Arm-2D) (Project inspiration)
    - [Related article](https://mp.weixin.qq.com/s/qDuVUSz9FjVqmAuhFhS6rQ)
- [A good article about JPEG basics](https://www.cnblogs.com/Dreaming-in-Gottingen/p/14428152.html)


