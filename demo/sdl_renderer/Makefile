# Install
BIN = demo

# Flags
CFLAGS += -std=c89 -Wall -Wextra -pedantic -O2 -DSDL_DISABLE_IMMINTRIN_H
CFLAGS += `sdl2-config --cflags`

SRC = main.c
OBJ = $(SRC:.c=.o)

ifeq ($(OS),Windows_NT)
#TODO
#BIN := $(BIN).exe
#LIBS = -lmingw32 -lSDL2main -lSDL2 -lopengl32 -lm -lGLU32
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Darwin)
#TODO		LIBS = -lSDL2 -framework OpenGL -lm
	else
		LIBS += -lm -ldl `sdl2-config --libs`
	endif
endif

$(BIN):
	@mkdir -p bin
	rm -f bin/$(BIN) $(OBJS)
	$(CC) $(SRC) $(CFLAGS) -o bin/$(BIN) $(LIBS)
