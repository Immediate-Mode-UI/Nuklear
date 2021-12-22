# Install
BIN = demo

# Compiler
CC = clang
DCC = gcc

# Flags
CFLAGS += -std=c89 -Wall -Wextra -pedantic -O2

SRC = main.c
OBJ = $(SRC:.c=.o)

# Modes
.PHONY: gcc
gcc: CC = gcc
gcc: $(BIN)

.PHONY: clang
clang: CC = clang
clang: $(BIN)

$(BIN):
	$(CC) $(SRC) $(CFLAGS) -o $(BIN) -lX11 -lm -lGL -lm -lGLU

clean:
	$(RM) $(BIN) $(OBJ)
