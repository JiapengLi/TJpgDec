CC = gcc
CFLAGS = -Wall -O2 -Wno-format-zero-length
SRC = ./src

BUILD_DEBUG = build-debug
BUILD_RELEASE = build-release

SRCS = main.c $(SRC)/tjpgd.c

OBJS_DEBUG = $(addprefix $(BUILD_DEBUG)/,$(notdir $(SRCS:.c=.o)))
OBJS_RELEASE = $(addprefix $(BUILD_RELEASE)/,$(notdir $(SRCS:.c=.o)))

all: jpeg_decode_debug jpeg_decode

# -----------------------------
# Debug
# -----------------------------
jpeg_decode_debug: CFLAGS += -DJD_DEBUG=1
jpeg_decode_debug: $(OBJS_DEBUG)
	@mkdir -p $(BUILD_DEBUG)
	$(CC) $(CFLAGS) -o $@ $(OBJS_DEBUG)

$(BUILD_DEBUG)/%.o: $(SRC)/%.c $(SRC)/tjpgd.h $(SRC)/tjpgdcnf.h
	@mkdir -p $(BUILD_DEBUG)
	$(CC) $(CFLAGS) -DJD_DEBUG=1 -I $(SRC) -c $< -o $@

$(BUILD_DEBUG)/main.o: main.c
	@mkdir -p $(BUILD_DEBUG)
	$(CC) $(CFLAGS) -DJD_DEBUG=1 -I $(SRC) -c $< -o $@

# -----------------------------
# Release
# -----------------------------
jpeg_decode: CFLAGS += -DJD_DEBUG=0
jpeg_decode: $(OBJS_RELEASE)
	@mkdir -p $(BUILD_RELEASE)
	$(CC) $(CFLAGS) -o $@ $(OBJS_RELEASE)

$(BUILD_RELEASE)/%.o: $(SRC)/%.c $(SRC)/tjpgd.h $(SRC)/tjpgdcnf.h
	@mkdir -p $(BUILD_RELEASE)
	$(CC) $(CFLAGS) -DJD_DEBUG=0 -I $(SRC) -c $< -o $@

$(BUILD_RELEASE)/main.o: main.c
	@mkdir -p $(BUILD_RELEASE)
	$(CC) $(CFLAGS) -DJD_DEBUG=0 -I $(SRC) -c $< -o $@

clean:
	rm -rf $(BUILD_DEBUG) $(BUILD_RELEASE)
