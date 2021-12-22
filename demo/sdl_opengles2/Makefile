# Install
BIN = demo

# Flags
CFLAGS += -std=c89 -Wall -Wextra -pedantic -O2

SRC = main.c
OBJ = $(SRC:.c=.o)

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	LIBS = -lSDL2 -framework OpenGLES -lm
else
	LIBS = -lSDL2 -lGLESv2 -lm
endif

$(BIN):
	$(CC) $(SRC) $(CFLAGS) -o $(BIN) $(LIBS)

web:
	emcc $(SRC) -Os -s USE_SDL=2 -o index.html

rpi:
	$(CC) $(SRC) $(CFLAGS) -o $(BIN) `PKG_CONFIG_PATH=/opt/vc/lib/pkgconfig/ pkg-config --cflags --libs bcm_host brcmglesv2` `/usr/local/bin/sdl2-config --libs --cflags`

clean:
	$(RM) $(BIN) $(OBJ)
