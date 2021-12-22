# Install
BIN = demo
CFLAGS = `sdl2-config --cflags --libs`   -std=c89 -Wall -Wextra -pedantic -Wno-unused-function -O0 -g -fvisibility=hidden `pkg-config SDL2_ttf --cflags --libs`

$(BIN):
	$(CC) -o $(BIN) *.c $(CFLAGS) -lrt -lm

clean:
	$(RM) $(BIN)
