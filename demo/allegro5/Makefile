# Install
BIN = demo

PARENT_DIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST)))/..)
include $(PARENT_DIR)/Makefile.inc

SRC = main.c
OBJ = $(SRC:.c=.o)

# TODO: Handle Windows build
#ifeq ($(OS),Windows_NT)
#BIN := $(BIN).exe
#LIBS = -lglfw3 -lopengl32 -lm -lGLU32 -lGLEW32
#else
LIBS = -lallegro -lallegro_main -lallegro_image -lallegro_font \
	-lallegro_ttf -lallegro_primitives -lm
#endif

$(BIN):
	@mkdir -p bin
	rm -f bin/$(BIN) $(OBJS)
	$(CC) $(SRC) $(CFLAGS) -o bin/$(BIN) $(LIBS)
