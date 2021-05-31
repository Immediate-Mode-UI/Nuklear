CFLAGS=`sdl2-config --cflags --libs`   -std=c11 -Wall  -O0 -g -fvisibility=hidden -Wno-unused `pkg-config SDL2_ttf --cflags --libs`

.PHONY: clean

demo: main.c sdl2surface_rawfb.h
	$(CC) -o demo *.c $(CFLAGS) -lrt -lm

clean:
	$(RM) demo
