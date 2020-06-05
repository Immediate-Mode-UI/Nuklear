BIN = main
CFLAGS += -std=c99 -Wall -pedantic -O0
SRC = main.c
LIBS = -L. -lraylib -lopengl32 `pkg-config --libs glfw3`

$(BIN): Makefile main.c nuklear_raylib.h
	rm -f $(BIN)
	$(CC) $(SRC) $(CFLAGS) -o $(BIN) $(LIBS)

clean:
	rm -f $(BIN)