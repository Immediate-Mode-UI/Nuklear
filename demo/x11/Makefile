# Install
BIN = demo

# Flags
CFLAGS += -std=c89 -Wall -Wextra -pedantic -Wno-unused-function -O2

SRC = main.c
OBJ = $(SRC:.c=.o)

$(BIN): clean bin
	$(CC) $(SRC) $(CFLAGS) -D_POSIX_C_SOURCE=200809L -o $(BIN) -lX11 -lm

clean:
	$(RM) $(BIN) $(OBJ)
