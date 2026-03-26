CFLAGS+=`sdl2-config --cflags --libs`   -std=c89 -Wall -Wextra -pedantic -Wno-unused-function -O0 -g -fvisibility=hidden `pkg-config SDL2_ttf --cflags --libs`

.PHONY: clean

demo: main.c ../nuklear_rawfb.h
	$(CC) -o demo *.c $(CFLAGS) -lrt -lm

clean:
	$(RM) demo
