#define _GNU_SOURCE // for O_TMPFILE
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_IMPLEMENTATION
#define NK_RAWFB_IMPLEMENTATION
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_INCLUDE_SOFTWARE_FONT

#include <wayland-client.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>

#include "../../../nuklear.h"

#include "xdg-shell.h"
#include "../nuklear_rawfb.h"

struct nk_wayland {
    /*wayland vars*/
    struct wl_display *display;
    struct wl_compositor *compositor;
    struct xdg_wm_base *xdg_wm_base;
    struct wl_shm *wl_shm;
    struct wl_seat* seat;
    struct wl_callback *frame_callback;
    struct wl_surface *surface;
    struct xdg_surface *xdg_surface;
    struct xdg_toplevel *xdg_toplevel;
    struct wl_buffer *front_buffer;

    int32_t *data;
    int mouse_pointer_x;
    int mouse_pointer_y;
    uint8_t tex_scratch[512 * 512];

    struct rawfb_context *rawfb;
};

#define WIDTH 800
#define HEIGHT 600

#define DTIME           20

/* ===============================================================
 *
 *                          EXAMPLE
 *
 * ===============================================================*/
/* This are some code examples to provide a small overview of what can be
 * done with this library. To try out an example uncomment the defines */
/*#define INCLUDE_ALL */
/*#define INCLUDE_STYLE */
/*#define INCLUDE_CALCULATOR */
/*#define INCLUDE_CANVAS */
#define INCLUDE_OVERVIEW
/*#define INCLUDE_CONFIGURATOR */
/*#define INCLUDE_NODE_EDITOR */

#ifdef INCLUDE_ALL
  #define INCLUDE_STYLE
  #define INCLUDE_CALCULATOR
  #define INCLUDE_CANVAS
  #define INCLUDE_OVERVIEW
  #define INCLUDE_CONFIGURATOR
  #define INCLUDE_NODE_EDITOR
#endif

#ifdef INCLUDE_STYLE
  #include "../../common/style.c"
#endif
#ifdef INCLUDE_CALCULATOR
  #include "../../common/calculator.c"
#endif
#ifdef INCLUDE_CANVAS
  #include "../../common/canvas.c"
#endif
#ifdef INCLUDE_OVERVIEW
  #include "../../common/overview.c"
#endif
#ifdef INCLUDE_CONFIGURATOR
  #include "../../common/style_configurator.c"
#endif
#ifdef INCLUDE_NODE_EDITOR
  #include "../../common/node_editor.c"
#endif


//WAYLAND OUTPUT INTERFACE
static void nk_wayland_output_cb_geometry(void *data, struct wl_output *wl_output, int x, int y, int w, int h, int subpixel, const char *make, const char *model, int transform)
{
    NK_UNUSED(data);
    NK_UNUSED(wl_output);
    NK_UNUSED(subpixel);
    NK_UNUSED(make);
    NK_UNUSED(model);
    NK_UNUSED(transform);

    printf("wl_output geometry x=%d, y=%d, w=%d, h=%d make=%s, model=%s \n", x,y,w,h, make, model);
}

static void nk_wayland_output_cb_mode(void *data, struct wl_output *wl_output, unsigned int flags, int w, int h, int refresh)
{
    NK_UNUSED(data);
    NK_UNUSED(wl_output);
    NK_UNUSED(flags);
    NK_UNUSED(w);
    NK_UNUSED(h);
    NK_UNUSED(refresh);
}

static void nk_wayland_output_cb_done(void *data, struct wl_output *output)
{
    NK_UNUSED(data);
    NK_UNUSED(output);
}

static void nk_wayland_output_cb_scale(void *data, struct wl_output *output, int scale)
{
    NK_UNUSED(data);
    NK_UNUSED(output);
    NK_UNUSED(scale);
}

static const struct wl_output_listener nk_wayland_output_listener =
{
   &nk_wayland_output_cb_geometry,
   &nk_wayland_output_cb_mode,
   &nk_wayland_output_cb_done,
   &nk_wayland_output_cb_scale
};
//-------------------------------------------------------------------- endof WAYLAND OUTPUT INTERFACE

//WAYLAND POINTER INTERFACE (mouse/touchpad)
static void nk_wayland_pointer_enter (void *data, struct wl_pointer *pointer, uint32_t serial, struct wl_surface *surface, wl_fixed_t surface_x, wl_fixed_t surface_y)
{
    NK_UNUSED(data);
    NK_UNUSED(pointer);
    NK_UNUSED(serial);
    NK_UNUSED(surface);
    NK_UNUSED(surface_x);
    NK_UNUSED(surface_y);
}

static void nk_wayland_pointer_leave (void *data, struct wl_pointer *pointer, uint32_t serial, struct wl_surface *surface)
{
    NK_UNUSED(data);
    NK_UNUSED(pointer);
    NK_UNUSED(serial);
    NK_UNUSED(surface);
}

static void nk_wayland_pointer_motion (void *data, struct wl_pointer *pointer, uint32_t time, wl_fixed_t x, wl_fixed_t y)
{
    struct nk_wayland* win = (struct nk_wayland*)data;

    NK_UNUSED(pointer);
    NK_UNUSED(time);

    win->mouse_pointer_x = wl_fixed_to_int(x);
    win->mouse_pointer_y = wl_fixed_to_int(y);

    nk_input_motion(&(win->rawfb->ctx), win->mouse_pointer_x, win->mouse_pointer_y);
}

static void nk_wayland_pointer_button (void *data, struct wl_pointer *pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state)
{
    struct nk_wayland* win = (struct nk_wayland*)data;

    NK_UNUSED(pointer);
    NK_UNUSED(serial);
    NK_UNUSED(time);

    if (button == 272){ //left mouse button
        if (state == WL_POINTER_BUTTON_STATE_PRESSED) {
           // printf("nk_input_button x=%d, y=%d press: 1 \n", win->mouse_pointer_x, win->mouse_pointer_y);
            nk_input_button(&(win->rawfb->ctx), NK_BUTTON_LEFT, win->mouse_pointer_x, win->mouse_pointer_y, 1);

        } else if (state == WL_POINTER_BUTTON_STATE_RELEASED) {
            nk_input_button(&(win->rawfb->ctx), NK_BUTTON_LEFT, win->mouse_pointer_x, win->mouse_pointer_y, 0);
        }
    }
}

static void nk_wayland_pointer_axis (void *data, struct wl_pointer *pointer, uint32_t time, uint32_t axis, wl_fixed_t value)
{
    NK_UNUSED(data);
    NK_UNUSED(pointer);
    NK_UNUSED(time);
    NK_UNUSED(axis);
    NK_UNUSED(value);
}

static struct wl_pointer_listener nk_wayland_pointer_listener =
{
    &nk_wayland_pointer_enter,
    &nk_wayland_pointer_leave,
    &nk_wayland_pointer_motion,
    &nk_wayland_pointer_button,
    &nk_wayland_pointer_axis,
    NULL,
    NULL,
    NULL,
    NULL
};
//-------------------------------------------------------------------- endof WAYLAND POINTER INTERFACE

//WAYLAND KEYBOARD INTERFACE
static void nk_wayland_keyboard_keymap (void *data, struct wl_keyboard *keyboard, uint32_t format, int32_t fd, uint32_t size)
{
    NK_UNUSED(data);
    NK_UNUSED(keyboard);
    NK_UNUSED(format);
    NK_UNUSED(fd);
    NK_UNUSED(size);
}

static void nk_wayland_keyboard_enter (void *data, struct wl_keyboard *keyboard, uint32_t serial, struct wl_surface *surface, struct wl_array *keys)
{
    NK_UNUSED(data);
    NK_UNUSED(keyboard);
    NK_UNUSED(serial);
    NK_UNUSED(surface);
    NK_UNUSED(keys);
}

static void nk_wayland_keyboard_leave (void *data, struct wl_keyboard *keyboard, uint32_t serial, struct wl_surface *surface)
{
    NK_UNUSED(data);
    NK_UNUSED(keyboard);
    NK_UNUSED(serial);
    NK_UNUSED(surface);
}

static void nk_wayland_keyboard_key (void *data, struct wl_keyboard *keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state)
{
    NK_UNUSED(data);
    NK_UNUSED(keyboard);
    NK_UNUSED(serial);
    NK_UNUSED(time);
    NK_UNUSED(state);
    printf("key: %d \n", key);
}

static void nk_wayland_keyboard_modifiers (void *data, struct wl_keyboard *keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group)
{
    NK_UNUSED(data);
    NK_UNUSED(keyboard);
    NK_UNUSED(serial);
    NK_UNUSED(mods_depressed);
    NK_UNUSED(mods_latched);
    NK_UNUSED(mods_locked);
    NK_UNUSED(group);
}

static struct wl_keyboard_listener nk_wayland_keyboard_listener =
{
    &nk_wayland_keyboard_keymap,
    &nk_wayland_keyboard_enter,
    &nk_wayland_keyboard_leave,
    &nk_wayland_keyboard_key,
    &nk_wayland_keyboard_modifiers,
    NULL
};
//-------------------------------------------------------------------- endof WAYLAND KEYBOARD INTERFACE

//WAYLAND SEAT INTERFACE
static void seat_capabilities (void *data, struct wl_seat *seat, uint32_t capabilities)
{
     struct nk_wayland* win = (struct nk_wayland*)data;

	if (capabilities & WL_SEAT_CAPABILITY_POINTER) {
		struct wl_pointer *pointer = wl_seat_get_pointer (seat);
		wl_pointer_add_listener (pointer, &nk_wayland_pointer_listener, win);
	}
	if (capabilities & WL_SEAT_CAPABILITY_KEYBOARD) {
		struct wl_keyboard *keyboard = wl_seat_get_keyboard (seat);
		wl_keyboard_add_listener (keyboard, &nk_wayland_keyboard_listener, win);
	}
}

static struct wl_seat_listener seat_listener =
{
    &seat_capabilities,
    NULL
};
//-------------------------------------------------------------------- endof WAYLAND SEAT INTERFACE

// WAYLAND SHELL INTERFACE
static void nk_wayland_xdg_wm_base_ping (void *data, struct xdg_wm_base *xdg_wm_base, uint32_t serial)
{
    NK_UNUSED(data);
    xdg_wm_base_pong (xdg_wm_base, serial);
}

static struct xdg_wm_base_listener nk_wayland_xdg_wm_base_listener =
{
    &nk_wayland_xdg_wm_base_ping
};

static void nk_wayland_xdg_surface_configure (void *data, struct xdg_surface *xdg_surface, uint32_t serial)
{
    NK_UNUSED(data);
    xdg_surface_ack_configure(xdg_surface, serial);
}

static struct xdg_surface_listener nk_wayland_xdg_surface_listener =
{
    &nk_wayland_xdg_surface_configure,
};

static void nk_wayland_xdg_toplevel_configure (void *data, struct xdg_toplevel *xdg_toplevel, int32_t width, int32_t height, struct wl_array *states)
{
    NK_UNUSED(data);
    NK_UNUSED(xdg_toplevel);
    NK_UNUSED(width);
    NK_UNUSED(height);
    NK_UNUSED(states);
}

static void nk_wayland_xdg_toplevel_close (void *data, struct xdg_toplevel *xdg_toplevel)
{
    NK_UNUSED(data);
    NK_UNUSED(xdg_toplevel);
}

static struct xdg_toplevel_listener nk_wayland_xdg_toplevel_listener =
{
    &nk_wayland_xdg_toplevel_configure,
    &nk_wayland_xdg_toplevel_close
};
//--------------------------------------------------------------------- endof WAYLAND SHELL INTERFACE


// WAYLAND REGISTRY INTERFACE
static void nk_wayland_registry_add_object (void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version)
{
    struct nk_wayland* win = (struct nk_wayland*)data;

    NK_UNUSED(version);

    //printf("looking for %s interface \n", interface);
	if (!strcmp(interface,"wl_compositor")) {
		win->compositor = wl_registry_bind (registry, name, &wl_compositor_interface, 1);

	} else if (!strcmp(interface,"xdg_wm_base")) {
		win->xdg_wm_base = wl_registry_bind (registry, name, &xdg_wm_base_interface, 1);
		xdg_wm_base_add_listener (win->xdg_wm_base, &nk_wayland_xdg_wm_base_listener, win);
	} else if (!strcmp(interface,"wl_shm")) {
		win->wl_shm = wl_registry_bind (registry, name, &wl_shm_interface, 1);

	} else if (!strcmp(interface,"wl_seat")) {
		win->seat = wl_registry_bind (registry, name, &wl_seat_interface, 1);
		wl_seat_add_listener (win->seat, &seat_listener, win);

	} else if (!strcmp(interface, "wl_output")) {
        struct wl_output *wl_output = wl_registry_bind(registry, name, &wl_output_interface, 1);
        wl_output_add_listener(wl_output, &nk_wayland_output_listener, NULL);
    }
}

static void nk_wayland_registry_remove_object (void *data, struct wl_registry *registry, uint32_t name)
{
    NK_UNUSED(data);
    NK_UNUSED(registry);
    NK_UNUSED(name);
}

static struct wl_registry_listener nk_wayland_registry_listener =
{
    &nk_wayland_registry_add_object,
    &nk_wayland_registry_remove_object
};
//------------------------------------------------------------------------------------------------ endof WAYLAND REGISTRY INTERFACE

static void nk_wayland_deinit(struct nk_wayland *win)
{
	xdg_toplevel_destroy (win->xdg_toplevel);
	xdg_surface_destroy (win->xdg_surface);
	xdg_wm_base_destroy (win->xdg_wm_base);
	wl_surface_destroy (win->surface);
}

static long timestamp(void)
{
    struct timeval tv;
    if (gettimeofday(&tv, NULL) < 0) return 0;
    return (long)((long)tv.tv_sec * 1000 + (long)tv.tv_usec/1000);
}

static void sleep_for(long t)
{
    struct timespec req;
    const time_t sec = (int)(t/1000);
    const long ms = t - (sec * 1000);
    req.tv_sec = sec;
    req.tv_nsec = ms * 1000000L;
    while(-1 == nanosleep(&req, &req));
}

static void nk_wayland_surf_clear(struct nk_wayland* win)
{
    int x, y;
    int pix_idx;

    for (y = 0; y < HEIGHT; y++){
        for (x = 0; x < WIDTH; x++){
            pix_idx = y * WIDTH + x;
            win->data[pix_idx] = 0xFF000000;
        }
    }
}

//This causes the screen to refresh
static const struct wl_callback_listener frame_listener;

static void redraw(void *data, struct wl_callback *callback, uint32_t time)
{
  //  printf("redrawing.. 1\n");
    struct nk_wayland* win = (struct nk_wayland*)data;

    NK_UNUSED(callback);
    NK_UNUSED(time);

    wl_callback_destroy(win->frame_callback);
    wl_surface_damage(win->surface, 0, 0, WIDTH, HEIGHT);



    win->frame_callback = wl_surface_frame(win->surface);
    wl_surface_attach(win->surface, win->front_buffer, 0, 0);
    wl_callback_add_listener(win->frame_callback, &frame_listener, win);
    wl_surface_commit(win->surface);

}


static const struct wl_callback_listener frame_listener = {
    redraw
};

int main ()
{
    long dt;
    long started;
    struct nk_wayland nk_wayland_ctx;
    struct wl_registry *registry;
    int running = 1;
    struct rawfb_pl pl;

    #ifdef INCLUDE_CONFIGURATOR
    static struct nk_color color_table[NK_COLOR_COUNT];
    memcpy(color_table, nk_default_color_style, sizeof(color_table));
    #endif

    //1. Initialize display
	nk_wayland_ctx.display = wl_display_connect (NULL);
    if (nk_wayland_ctx.display == NULL) {
        printf("no wayland display found. do you have wayland composer running? \n");
        return -1;
    }

	registry = wl_display_get_registry (nk_wayland_ctx.display);
	wl_registry_add_listener (registry, &nk_wayland_registry_listener, &nk_wayland_ctx);
	wl_display_roundtrip (nk_wayland_ctx.display);


    //2. Create Window
	nk_wayland_ctx.surface = wl_compositor_create_surface (nk_wayland_ctx.compositor);

	nk_wayland_ctx.xdg_surface = xdg_wm_base_get_xdg_surface(nk_wayland_ctx.xdg_wm_base, nk_wayland_ctx.surface);
	xdg_surface_add_listener (nk_wayland_ctx.xdg_surface, &nk_wayland_xdg_surface_listener, &nk_wayland_ctx);

	nk_wayland_ctx.xdg_toplevel = xdg_surface_get_toplevel(nk_wayland_ctx.xdg_surface);
	xdg_toplevel_add_listener (nk_wayland_ctx.xdg_toplevel, &nk_wayland_xdg_toplevel_listener, &nk_wayland_ctx);

    nk_wayland_ctx.frame_callback = wl_surface_frame(nk_wayland_ctx.surface);
    wl_callback_add_listener(nk_wayland_ctx.frame_callback, &frame_listener, &nk_wayland_ctx);

    wl_surface_commit (nk_wayland_ctx.surface);

	size_t size = WIDTH * HEIGHT * 4;
	char *xdg_runtime_dir = getenv ("XDG_RUNTIME_DIR");
	int fd = open (xdg_runtime_dir, O_TMPFILE|O_RDWR|O_EXCL, 0600);
	ftruncate (fd, size);
	nk_wayland_ctx.data = mmap (NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	struct wl_shm_pool *pool = wl_shm_create_pool (nk_wayland_ctx.wl_shm, fd, size);
	nk_wayland_ctx.front_buffer = wl_shm_pool_create_buffer (pool, 0, WIDTH, HEIGHT, WIDTH*4, WL_SHM_FORMAT_XRGB8888);
	wl_shm_pool_destroy (pool);
	close (fd);

	wl_display_roundtrip (nk_wayland_ctx.display);


    //3. Clear window and start rendering loop
	nk_wayland_surf_clear(&nk_wayland_ctx);
    wl_surface_attach (nk_wayland_ctx.surface, nk_wayland_ctx.front_buffer, 0, 0);
    wl_surface_commit (nk_wayland_ctx.surface);

    pl.bytesPerPixel = 4;
    pl.ashift = 24;
    pl.rshift = 16;
    pl.gshift = 8;
    pl.bshift = 0;
    pl.aloss = 0;
    pl.rloss = 0;
    pl.gloss = 0;
    pl.bloss = 0;

    nk_rawfb_init(nk_wayland_ctx.data, nk_wayland_ctx.tex_scratch, WIDTH, HEIGHT, WIDTH*4, pl);


    //4. rendering UI
    while (running) {
        started = timestamp();

        // GUI
        if (nk_begin(&(nk_wayland_ctx.rawfb->ctx), "Demo", nk_rect(50, 50, 200, 200),
            NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|
            NK_WINDOW_CLOSABLE|NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE)) {
            enum {EASY, HARD};
            static int op = EASY;
            static int property = 20;

            nk_layout_row_static(&(nk_wayland_ctx.rawfb->ctx), 30, 80, 1);
            if (nk_button_label(&(nk_wayland_ctx.rawfb->ctx), "button")){
                printf("button pressed\n");
            }
            nk_layout_row_dynamic(&(nk_wayland_ctx.rawfb->ctx), 30, 2);
            if (nk_option_label(&(nk_wayland_ctx.rawfb->ctx), "easy", op == EASY)) op = EASY;
            if (nk_option_label(&(nk_wayland_ctx.rawfb->ctx), "hard", op == HARD)) op = HARD;
            nk_layout_row_dynamic(&(nk_wayland_ctx.rawfb->ctx), 25, 1);
            nk_property_int(&(nk_wayland_ctx.rawfb->ctx), "Compression:", 0, &property, 100, 10, 1);
        }
        nk_end(&(nk_wayland_ctx.rawfb->ctx));

        if (nk_window_is_closed(&(nk_wayland_ctx.rawfb->ctx), "Demo")) break;

        /* -------------- EXAMPLES ---------------- */
        #ifdef INCLUDE_CALCULATOR
          calculator(&(nk_wayland_ctx.rawfb->ctx));
        #endif
        #ifdef INCLUDE_CANVAS
          canvas(&(nk_wayland_ctx.rawfb->ctx));
        #endif
        #ifdef INCLUDE_OVERVIEW
          overview(&(nk_wayland_ctx.rawfb->ctx));
        #endif
        #ifdef INCLUDE_CONFIGURATOR
          style_configurator(&(nk_wayland_ctx.rawfb->ctx), color_table);
        #endif
        #ifdef INCLUDE_NODE_EDITOR
          node_editor(&(nk_wayland_ctx.rawfb->ctx));
        #endif
        /* ----------------------------------------- */

        // Draw framebuffer
        nk_rawfb_render(nk_wayland_ctx.rawfb, nk_rgb(30,30,30), 1);


        //handle wayland stuff (send display to FB & get inputs)
        nk_input_begin(&(nk_wayland_ctx.rawfb->ctx));
        wl_display_dispatch(nk_wayland_ctx.display);
        nk_input_end(&(nk_wayland_ctx.rawfb->ctx));

        // Timing
        dt = timestamp() - started;
        if (dt < DTIME)
            sleep_for(DTIME - dt);
    }

	nk_wayland_deinit (&nk_wayland_ctx);
	wl_display_disconnect (nk_wayland_ctx.display);
	return 0;
}
