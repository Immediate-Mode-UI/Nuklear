CFLAGS=`sdl2-config --cflags --libs` -std=c99 -Wall -O0 -g -fvisibility=hidden -Wno-unused `pkg-config SDL2_ttf --cflags --libs`

.PHONY: clean

demo: main.c sdl2surface_rawfb.h
	$(CC) -o demo main.c $(CFLAGS) -lrt -lm

filebrowser: file_browser.c sdl2surface_rawfb.h
	$(CC) -o filebr file_browser.c $(CFLAGS) -lSDL2_image -lrt -lm

canvas: canvas.c sdl2surface_rawfb.h
	$(CC) -o canvas canvas.c $(CFLAGS) -lSDL2_image -lrt -lm

extended: extended.c sdl2surface_rawfb.h
	$(CC) -o extended extended.c $(CFLAGS) -lSDL2_image -lrt -lm

skinning: skinning.c sdl2surface_rawfb.h
	$(CC) -o skinning skinning.c $(CFLAGS) -lSDL2_image -lrt -lm

clean:
	$(RM) demo filebr canvas extended skinning
