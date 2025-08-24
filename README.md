

## Compare with Original TJpgDec

- Target to 32bits MCU only
- Runtime configurable color mode
- Single entry point to load buffer
- Lower RAM memory footprint, but use more ROM (which means more features)
    - single copy jd->workbuf, maximum 4*64 bytes
    - support very small cache jd->inbuf (minimum several bytes, bufferless)
- Scale feature removed
- Very fast single pixel extraction (skip dequantization, idct and color conversion if MCU or block is irrelevant)
- Add more color formats (Grayscale, RGB565, BGR565, RGB888, BGR888, RGBA8888, BGRA8888)

### Limitations

- Strict 8x8 block output, even in 16x16 or 16x8 MCU mode
- Under YUV 4:2:0 or 4:2:2 mode, 8x8 block output sequence is a subsequence of standard 16x16 or 16x8 output sequence

## TODO

- to support none x8 images
- optimize for ARM or RISC-V architectures
- memory optimization
- random access to jpeg file?
- More readable error code

## Build

```
make clean all
```

## Auto Format

```

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
