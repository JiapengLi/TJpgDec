

## Compare with Original TJpgDec

- Target to 32bits MCU only
- Configurable Color mode
- Single entry point to load buffer
- Lower RAM memory footprint, but use more ROM (which means more features)
    - single copy jd->workbuf, maximum 4*64 bytes
    - support very small cache jd->inbuf (minimum several bytes, bufferless)
- Scale feature removed
- Very fast single pixel extraction (skip dequantization, idct and color conversion if block is irrelevant)

## TODO

- to support none x8 images
- fast extract one pixel or one block
- optimize for ARM or RISC-V architectures
- memory optimize
- random access to jpeg file?

## Build


```
make
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
--lineend=windows \
--convert-tabs \
--verbose \
--add-braces \
./src/tjpgd.c
```
