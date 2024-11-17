# Install
BIN = demo

# Flags
CFLAGS += -std=c89 -Wall -Wextra -pedantic -fsanitize=address -O2
CFLAGS += -DSDL_DISABLE_IMMINTRIN_H

SRC = main.c
OBJ = $(SRC:.c=.o)

ifeq ($(OS),Windows_NT)
BIN := $(BIN).exe
LIBS = -lsdl2 -lvulkan -lm
else
	UNAME_S := $(shell uname -s)
	SDL2 := $(shell pkg-config --libs sdl2)
	LIBS = $(SDL2) -lvulkan -lm
endif


$(BIN): shaders/demo.vert.spv shaders/demo.frag.spv
	@mkdir -p bin
	rm -f bin/$(BIN) $(OBJS)
	$(CC) $(SRC) $(CFLAGS) -o bin/$(BIN) $(LIBS)

shaders/demo.vert.spv: shaders/demo.vert
	glslc --target-env=vulkan shaders/demo.vert -o shaders/demo.vert.spv

shaders/demo.frag.spv: shaders/demo.frag
	glslc --target-env=vulkan shaders/demo.frag -o shaders/demo.frag.spv
