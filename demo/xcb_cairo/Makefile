# Target
BIN = demo
CFLAGS += -std=c89 -pedantic
LDFLAGS += -lm
SRC = ${wildcard *.c}
OBJ = $(SRC:.c=.o)

# Freetype
CFLAGS += ${shell pkg-config --cflags freetype2}
LDFLAGS += ${shell pkg-config --libs freetype2}

# XCB
CFLAGS += ${shell pkg-config --cflags xcb xcb-util xcb-keysyms xkbcommon xkbcommon-x11}
LDFLAGS += ${shell pkg-config --libs xcb xcb-util xcb-keysyms xkbcommon xkbcommon-x11}

# Cairo
CFLAGS += ${shell pkg-config --cflags cairo}
LDFLAGS += ${shell pkg-config --libs cairo}

# Rules
$(BIN):
	@mkdir -p bin
	rm -f bin/$(BIN) $(OBJS)
	$(CC) $(SRC) $(CFLAGS) -D_POSIX_C_SOURCE=200809L -o bin/$(BIN) ${LDFLAGS}
