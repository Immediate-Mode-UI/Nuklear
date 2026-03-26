WAYLAND=`pkg-config wayland-client --cflags --libs`
WAYLAND_SCANNER=wayland-scanner
WAYLAND_PROTOCOLS_DIR=/usr/share/wayland-protocols

CFLAGS+=-std=c99 -Wall -Wextra -pedantic -Wno-unused-function -O3 -fvisibility=hidden

.PHONY: clean

demo: main.c xdg-shell.c xdg-shell.h
	$(CC) $(CFLAGS) -o demo *.c $(WAYLAND) -lrt -lm

xdg-shell.c:
	$(WAYLAND_SCANNER) code $(WAYLAND_PROTOCOLS_DIR)/stable/xdg-shell/xdg-shell.xml xdg-shell.c

xdg-shell.h:
	$(WAYLAND_SCANNER) client-header $(WAYLAND_PROTOCOLS_DIR)/stable/xdg-shell/xdg-shell.xml xdg-shell.h

clean:
	$(RM) demo xdg-shell.c xdg-shell.h
