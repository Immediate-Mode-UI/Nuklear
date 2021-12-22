# Install
BIN = demo

# Flags
CFLAGS += -std=c89 -Wall -Wextra -pedantic -Wno-unused-function -O2

SRC = main.c
OBJ = $(SRC:.c=.o)

LDFLAGS += -lX11 -lm

CFLAGS += ${shell pkg-config --cflags xft} -DNK_XLIB_USE_XFT
LDFLAGS += ${shell pkg-config --libs xft}

SRC = ${wildcard *.c}
OBJ = $(SRC:.c=.o)

$(BIN):
	$(CC) $(SRC) $(CFLAGS) -D_POSIX_C_SOURCE=200809L -o $(BIN) -lX11 ${LDFLAGS}

clean:
	$(RM) $(BIN) $(OBJ)

