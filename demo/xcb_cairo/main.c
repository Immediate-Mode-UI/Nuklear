/* nuklear - v1.32.0 - public domain */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_IMPLEMENTATION
#include "../../nuklear.h"

#define NK_XCB_CAIRO_IMPLEMENTATION
#include "nuklear_xcb.h"

static void die(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputs("\n", stderr);
    exit(EXIT_FAILURE);
}

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
#define INCLUDE_OVERVIEW
/*#define INCLUDE_CONFIGURATOR */
/*#define INCLUDE_NODE_EDITOR */
/*#define INCLUDE_CANVAS */

#ifdef INCLUDE_ALL
  #define INCLUDE_STYLE
  #define INCLUDE_CALCULATOR
  #define INCLUDE_OVERVIEW
  #define INCLUDE_NODE_EDITOR
  #define INCLUDE_CONFIGURATOR
  #define INCLUDE_CANVAS
#endif

#ifdef INCLUDE_STYLE
  #include "../common/style.c"
#endif
#ifdef INCLUDE_CALCULATOR
  #include "../common/calculator.c"
#endif
#ifdef INCLUDE_OVERVIEW
  #include "../common/overview.c"
#endif
#ifdef INCLUDE_CONFIGURATOR
  #include "../../demo/common/style_configurator.c"
#endif
#ifdef INCLUDE_NODE_EDITOR
  #include "../common/node_editor.c"
#endif
#ifdef INCLUDE_CANVAS
  #include "../common/canvas.c"
#endif

/* ===============================================================
 *
 *                          DEMO
 *
 * ===============================================================*/
int
main(void)
{
    struct nk_xcb_context *xcb_ctx;
    struct nk_color background = nk_rgb(0, 0, 0);
    struct nk_cairo_context *cairo_ctx;
    struct nk_user_font *font;
    struct nk_context* ctx;
    int running = 1;
    int events;

    #ifdef INCLUDE_CONFIGURATOR
    static struct nk_color color_table[NK_COLOR_COUNT];
    memcpy(color_table, nk_default_color_style, sizeof(color_table));
    #endif

    xcb_ctx = nk_xcb_init("Nuklear XCB/Cairo", 20, 20, 600, 800);
    cairo_ctx = nk_cairo_init(&background, NULL, 0, nk_xcb_create_cairo_surface(xcb_ctx));
    /*cairo_ctx = nk_cairo_init(&background, "../../extra_font/DroidSans.ttf", 0, nk_xcb_create_surface(xcb_ctx));*/
    font = nk_cairo_default_font(cairo_ctx);
    ctx = malloc(sizeof(struct nk_context));
    nk_init_default(ctx, font);

    #ifdef INCLUDE_STYLE
    set_style(ctx, THEME_BLACK);
    /*nk_style_push_float(ctx, &ctx->style.window.rounding, 20.0f);*/
    /*set_style(ctx, THEME_WHITE);*/
    /*set_style(ctx, THEME_RED);*/
    /*set_style(ctx, THEME_BLUE);*/
    /*set_style(ctx, THEME_DARK);*/
    #endif

    while (running)
    {
        /* Events */
        events = nk_xcb_handle_event(xcb_ctx, ctx);
        if (events & NK_XCB_EVENT_STOP) {
            break;
        }
        if (events & NK_XCB_EVENT_PAINT) {
            nk_cairo_damage(cairo_ctx);
        }
        if (events & NK_XCB_EVENT_RESIZED) {
            nk_xcb_resize_cairo_surface(xcb_ctx, nk_cairo_surface(cairo_ctx));
        }

        /* GUI */
        if (nk_begin(ctx, "Demo", nk_rect(50, 50, 200, 200),
            NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
            NK_WINDOW_CLOSABLE|NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE))
        {
            enum {EASY, HARD};
            static int op = EASY;
            static int property = 20;

            nk_layout_row_static(ctx, 30, 80, 1);
            if (nk_button_label(ctx, "button"))
                fprintf(stdout, "button pressed\n");
            nk_layout_row_dynamic(ctx, 30, 2);
            if (nk_option_label(ctx, "easy", op == EASY)) op = EASY;
            if (nk_option_label(ctx, "hard", op == HARD)) op = HARD;
            nk_layout_row_dynamic(ctx, 25, 1);
            nk_property_int(ctx, "Compression:", 0, &property, 100, 10, 1);
        }
        nk_end(ctx);
        if (nk_window_is_hidden(ctx, "Demo")) {
            break;
        }

        /* -------------- EXAMPLES ---------------- */
        #ifdef INCLUDE_CALCULATOR
        calculator(ctx);
        #endif
        #ifdef INCLUDE_OVERVIEW
        overview(ctx);
        #endif
        #ifdef INCLUDE_CONFIGURATOR
          style_configurator(ctx, color_table);
        #endif
        #ifdef INCLUDE_NODE_EDITOR
        node_editor(ctx);
        #endif
        #ifdef INCLUDE_CANVAS
        canvas(ctx);
        #endif
        /* ----------------------------------------- */

        /* Render */
        nk_cairo_render(cairo_ctx, ctx);
        nk_xcb_render(xcb_ctx);
        nk_clear(ctx);
    }

    nk_free(ctx);
    free(ctx);
    nk_cairo_free(cairo_ctx);
    nk_xcb_free(xcb_ctx);

    return EXIT_SUCCESS;
}
