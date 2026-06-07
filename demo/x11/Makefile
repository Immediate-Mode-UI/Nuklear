# Install
BIN = demo

# Flags
CFLAGS += -g -std=c89 -Wall -Wextra -pedantic -Wno-unused-function -D_POSIX_C_SOURCE=200809L

SRC = main.c
OBJ = $(SRC:.c=.o)

X11_CFLAGS += ${shell pkg-config --cflags x11}
X11_LDFLAGS += ${shell pkg-config --libs x11} -lm

XFT_CFLAGS += ${shell pkg-config --cflags xft x11} -DNK_XLIB_USE_XFT
XFT_LDFLAGS += ${shell pkg-config --libs xft x11} -lm

all: $(BIN) $(BIN)-xft

$(BIN):
	@mkdir -p bin
	rm -f bin/$(BIN) $(OBJS)
	$(CC) $(SRC) $(CFLAGS) $(X11_CFLAGS) -o bin/$(BIN) ${LDFLAGS} ${X11_LDFLAGS}

$(BIN)-xft:
	@mkdir -p bin
	rm -f bin/$(BIN)-xft $(OBJS)
	$(CC) $(SRC) $(CFLAGS) $(XFT_CFLAGS) -o bin/$(BIN)-xft ${LDFLAGS} ${LDFLAGS} ${XFT_LDFLAGS}
