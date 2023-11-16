# Install
BIN = demo

# Flags
CFLAGS += -std=c89 -pedantic -O0
CFLAGS += `sdl2-config --cflags` -DUSE_SDL_IMAGE

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
		LIBS += -lSDL2_image `sdl2-config --libs` -lm
	endif
endif

$(BIN):
	@mkdir -p bin
	rm -f bin/$(BIN) $(OBJS)
	$(CC) $(SRC) $(CFLAGS) -o bin/$(BIN) $(LIBS)
