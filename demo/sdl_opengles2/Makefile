# Install
BIN = demo

# Flags
CFLAGS += -std=c89 -Wall -Wextra -pedantic -DSDL_DISABLE_IMMINTRIN_H

SRC = main.c
OBJ = $(SRC:.c=.o)

LIBS = -lSDL2 -lm

$(BIN): prepare
	$(CC) $(SRC) $(CFLAGS) -o bin/$(BIN) $(LIBS)

web: prepare
	emcc $(SRC) -Os -s USE_SDL=2 -o bin/index.html

rpi: prepare
	$(CC) $(SRC) $(CFLAGS) -o bin/$(BIN) `PKG_CONFIG_PATH=/opt/vc/lib/pkgconfig/ pkg-config --cflags --libs bcm_host brcmglesv2` `/usr/local/bin/sdl2-config --libs --cflags`

prepare:
	@mkdir -p bin
	rm -f bin/$(BIN) $(OBJS)
