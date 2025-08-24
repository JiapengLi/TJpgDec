# TJpgDec (Tiny JPEG Decompressor) Variant

[![C Standard](https://img.shields.io/badge/C-C99-blue.svg)]()
[![Platform](https://img.shields.io/badge/platform-32--bit%20MCU-green.svg)]()

## Overview

This project is an enhanced variant of the original [TJpgDec](https://elm-chan.org/fsw/tjpgd/) by ChaN, specifically optimized for 32-bit microcontrollers. It features runtime-configurable color modes, significantly reduced RAM usage, and support for multiple output color formats, making it ideal for resource-constrained embedded systems.

### Key Features

- **Memory Efficient**: Minimal RAM footprint with configurable buffer sizes
- **Color Flexibility**: Runtime-configurable output formats (7 color modes supported)
- **Performance Optimized**: Fast single pixel extraction with selective processing
- **MCU Targeted**: Designed specifically for 32-bit embedded systems

## Technical Differences from Original TJpgDec

### Memory Optimizations
- **Runtime configurable color modes** - No compile-time color format dependency
- **Reduced RAM footprint** (trades ROM for RAM efficiency)
  - Single copy of `jd->workbuf`, maximum 4×64 bytes
  - Supports minimal cache for `jd->inbuf` (bufferless operation possible)
  - Optimized working buffer management

### Performance Enhancements
- **Fast single pixel extraction** - Skips dequantization, IDCT, and color conversion for irrelevant MCU/blocks
- **Streamlined API** - Single entry point for buffer loading
- **Reduced macro complexity** - Fewer configuration macros required

### Extended Color Support
Supports 7 output color formats:
- **Grayscale** (1 byte/pixel)
- **RGB565/BGR565** (2 bytes/pixel)
- **RGB888/BGR888** (3 bytes/pixel)
- **RGBA8888/BGRA8888** (4 bytes/pixel)

### Architectural Changes
- **Scale feature removed** - Simplified implementation focused on 1:1 decoding
- **Block-aligned output** - Strict 8×8 block processing for consistency

## System Requirements

### Build Environment
- GCC or compatible C compiler
- Make build system
- **Windows users**: MSYS2 or WSL required

## Quick Start

### 1. Build the Project

```bash
make clean all
```

### 2. Run Tests

```bash
./test.sh
```

### 3. Basic Usage Example

```c
#include "src/tjpgd.h"

// Initialize decompressor
JDEC jdec;
uint8_t work[3100];  // Working buffer

// Prepare for decompression
JRESULT res = jd_prepare(&jdec, input_func, work, sizeof(work), &device);

// Set output color format
jd_set_color(&jdec, JD_RGB888);

// Decompress image
res = jd_decomp(&jdec, output_func, 0);
```

## Configuration

### Compile-Time Options (tjpgdcnf.h)

| Option | Default | Description |
|--------|---------|-------------|
| `JD_SZBUF` | 256 | Input buffer size (bytes) |
| `JD_TBLCLIP` | 1 | Use lookup table for clipping (faster, +1KB ROM) |
| `JD_FASTDECODE` | 1 | Optimization level (0=basic, 1=32-bit optimized, 2=+huffman LUT) |
| `JD_DEBUG` | 1 | Enable debug output and logging |

### Runtime Color Modes

```c
typedef enum {
    JD_GRAYSCALE = 0,  // 1 byte/pixel
    JD_RGB565    = 1,  // 2 bytes/pixel
    JD_BGR565    = 2,  // 2 bytes/pixel
    JD_RGB888    = 3,  // 3 bytes/pixel
    JD_BGR888    = 4,  // 3 bytes/pixel
    JD_RGBA8888  = 5,  // 4 bytes/pixel
    JD_BGRA8888  = 6,  // 4 bytes/pixel
} JCOLOR;
```

## Limitations & Known Issues

- **Platform Support**: Optimized for 32-bit MCUs; 8/16-bit MCUs may not be supported
- **Block Processing**: Strict 8×8 block output, even in 16×16 or 16×8 MCU modes
- **YUV Subsampling**: For 4:2:0/4:2:2, output sequence is a subsequence of standard MCU order
- **Scaling**: Scale feature removed for simplified implementation
- **Boundary Clipping**: Output images may exceed actual image boundaries

## Development Roadmap

### High Priority
- [ ] Image boundary clipping
- [ ] Make `JD_FASTDECODE = 2` functional
- [ ] More descriptive error codes
- [ ] Memory usage optimization

### Future Enhancements
- [ ] ARM/RISC-V specific optimizations
- [ ] Random access JPEG decoding
- [ ] Performance benchmarking suite
- [ ] Advanced MCU sampling support
- [ ] Progressive JPEG support

## Development Tools

### Code Formatting

Auto-format source files using astyle:

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

### Project Structure

```text
TJpgDec/
├── src/                    # Core library source
│   ├── tjpgd.c            # Main decoder implementation
│   ├── tjpgd.h            # Public API header
│   └── tjpgdcnf.h         # Configuration header
├── samples/               # Sample output files
├── images/                # Test images and assets
├── build-*/              # Build output directories
├── main.c                # Demo application
├── Makefile              # Build configuration
└── test.sh               # Test runner script
```

## Contributing

1. Follow the existing code style (K&R with 4-space indents)
2. Test changes with `./test.sh` before submitting
3. Update documentation for API changes
4. Consider memory impact for embedded targets

## License

This project maintains the same open license as the original TJpgDec:

- Free for personal, non-profit, and commercial use
- No warranty provided
- Redistributions must retain copyright notices

## Acknowledgments

- [**Original TJpgDec**](https://elm-chan.org/fsw/tjpgd/) by ChaN - Foundation library
- [**Arm-2D Project**](https://github.com/ARM-software/Arm-2D) - Design inspiration
  - [Related technical article](https://mp.weixin.qq.com/s/qDuVUSz9FjVqmAuhFhS6rQ)
- [**JPEG Algorithm Reference**](https://www.cnblogs.com/Dreaming-in-Gottingen/p/14428152.html) - Technical background


