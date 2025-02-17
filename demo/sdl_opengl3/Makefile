# Install
BIN = demo

# Flags
CFLAGS += -std=c89 -Wall -Wextra -pedantic -DSDL_DISABLE_IMMINTRIN_H

SRC = main.c
OBJ = $(SRC:.c=.o)

ifeq ($(OS),Windows_NT)
BIN := $(BIN).exe
LIBS = -lmingw32 -lSDL2main -lSDL2 -lm
else
	LIBS = -lSDL2 -lm
endif

$(BIN):
	@mkdir -p bin
	rm -f bin/$(BIN) $(OBJS)
	$(CC) $(SRC) $(CFLAGS) -o bin/$(BIN) $(LIBS)
