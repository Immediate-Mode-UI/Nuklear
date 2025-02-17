# Install
BIN = demo

# Flags
CFLAGS += -std=c89 -Wall -Wextra -pedantic -Wno-unused-function

SRC = main.c
OBJ = $(SRC:.c=.o)

# TODO: Handle Windows build
#ifeq ($(OS),Windows_NT)
#BIN := $(BIN).exe
#LIBS = -lglfw3 -lm
#else
LIBS = -lallegro -lallegro_main -lallegro_image -lallegro_font \
	-lallegro_ttf -lallegro_primitives -lm
#endif

$(BIN):
	@mkdir -p bin
	rm -f bin/$(BIN) $(OBJS)
	$(CC) $(SRC) $(CFLAGS) -o bin/$(BIN) $(LIBS)
