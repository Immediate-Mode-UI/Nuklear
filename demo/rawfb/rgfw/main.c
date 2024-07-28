#define RGFW_BUFFER
#define RGFW_IMPLEMENTATION
#include "RGFW.h"


#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a,b) ((a) < (b) ? (b) : (a))
#endif


#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_IMPLEMENTATION
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_INCLUDE_SOFTWARE_FONT
#include "nuklear.h"
#define NK_RAWFB_IMPLEMENTATION
#include "nuklear_rawfb.h"

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
/*#define INCLUDE_OVERVIEW */
/*#define INCLUDE_NODE_EDITOR */

#ifdef INCLUDE_ALL
  #define INCLUDE_STYLE
  #define INCLUDE_CALCULATOR
  #define INCLUDE_CANVAS
  #define INCLUDE_OVERVIEW
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
#ifdef INCLUDE_NODE_EDITOR
  #include "../../common/node_editor.c"
#endif

static int translate_rgfw_key(u32 key)
{
    const static u32 map[] = {
        NK_KEY_NONE,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        NK_KEY_BACKSPACE,
        NK_KEY_TAB,
        0, 
        NK_KEY_SHIFT,
        NK_KEY_CTRL,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        NK_KEY_UP,
        NK_KEY_DOWN,
        NK_KEY_LEFT,
        NK_KEY_RIGHT, 
        
        NK_KEY_DEL,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

    return map[key];
}

#if 0
static void
grid_demo(struct nk_context *ctx)
{
    static char text[3][64];
    static int text_len[3];
    static const char *items[] = {"Item 0","item 1","item 2"};
    static int selected_item = 0;
    static int check = 1;

    int i;
    if (nk_begin(ctx, "Grid Demo", nk_rect(600, 350, 275, 250),
        NK_WINDOW_TITLE|NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|
        NK_WINDOW_NO_SCROLLBAR))
    {
        nk_layout_row_dynamic(ctx, 30, 2);
        nk_label(ctx, "Floating point:", NK_TEXT_RIGHT);
        nk_edit_string(ctx, NK_EDIT_FIELD, text[0], &text_len[0], 64, nk_filter_float);
        nk_label(ctx, "Hexadecimal:", NK_TEXT_RIGHT);
        nk_edit_string(ctx, NK_EDIT_FIELD, text[1], &text_len[1], 64, nk_filter_hex);
        nk_label(ctx, "Binary:", NK_TEXT_RIGHT);
        nk_edit_string(ctx, NK_EDIT_FIELD, text[2], &text_len[2], 64, nk_filter_binary);
        nk_label(ctx, "Checkbox:", NK_TEXT_RIGHT);
        nk_checkbox_label(ctx, "Check me", &check);
        nk_label(ctx, "Combobox:", NK_TEXT_RIGHT);
        if (nk_combo_begin_label(ctx, items[selected_item], nk_vec2(nk_widget_width(ctx), 200))) {
            nk_layout_row_dynamic(ctx, 25, 1);
            for (i = 0; i < 3; ++i)
                if (nk_combo_item_label(ctx, items[i], NK_TEXT_LEFT))
                    selected_item = i;
            nk_combo_end(ctx);
        }
    }
    nk_end(ctx);
}
#endif


int main(int argc, char **argv)
{
    struct nk_color clear = {0,100,0,255};
    struct nk_vec2 vec;
    struct nk_rect bounds = {40,40,0,0};
    struct rawfb_context *context;
    struct rawfb_pl pl;
    unsigned char tex_scratch[512 * 512];

    NK_UNUSED(argc);
    NK_UNUSED(argv);

    RGFW_window* window = RGFW_createWindow("Puzzle", RGFW_RECT(0, 0, 800, 700), RGFW_CENTER);
    if (!window)
    {
        printf("can't open window!\n");
        exit(1);
    }

    pl.bytesPerPixel = 4;
    pl.rshift = 16;
    pl.gshift = 8;
    pl.bshift = 0;
    pl.ashift = 24;
    pl.rloss = 0;
    pl.gloss = 0;
    pl.bloss = 0;
    pl.aloss = 0;

    RGFW_area a = RGFW_getScreenSize();
    context = nk_rawfb_init(window->buffer, tex_scratch, window->r.w, window->r.h, a.w * 4, pl);

    RGFW_window_showMouse(window, 0);
    while(RGFW_window_shouldClose(window) == RGFW_FALSE)
    {
        nk_input_begin(&(context->ctx));
        while (RGFW_window_checkEvent(window))
        {
            switch(window->event.type)
            {
                case RGFW_quit:
                    break;
                case RGFW_keyPressed:
                    nk_input_key(&(context->ctx), translate_rgfw_key(window->event.keyCode), 1);
                    break;
                case RGFW_keyReleased:
                    nk_input_key(&(context->ctx), translate_rgfw_key(window->event.keyCode), 0);
                    break;
                case RGFW_mousePosChanged:
                    nk_input_motion(&(context->ctx), window->event.point.x, window->event.point.y);
                    break;
                case RGFW_mouseButtonPressed:
                    if (window->event.button >= RGFW_mouseScrollUp) {
                        vec.x = 0;
                        vec.y = window->event.scroll;
                        nk_input_scroll(&(context->ctx), vec );
                        break;
                    }
                    nk_input_button(&(context->ctx), window->event.button - 1, window->event.point.x, window->event.point.y, 1);
                    break;
                case RGFW_mouseButtonReleased:
                    nk_input_button(&(context->ctx), window->event.button - 1, window->event.point.x, window->event.point.y, 0);
                    break;
                case RGFW_windowResized:
                    context->fb.w = window->r.w;
                    context->fb.h = window->r.h;
                    break;
            }
        }
        nk_input_end(&(context->ctx));

        bounds.w = 400;
        bounds.h = 400;
        if (nk_begin(&(context->ctx), "Test", bounds, NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE | NK_WINDOW_TITLE))
        {
            enum {EASY, HARD};
            static int op = EASY;
            static int property = 20;
            nk_layout_row_static(&(context->ctx), 30, 80, 1);
            if (nk_button_label(&(context->ctx), "button")){
                printf("button pressed\n");
            }
            nk_layout_row_dynamic(&(context->ctx), 40, 2);
            if (nk_option_label(&(context->ctx), "easy", op == EASY)) op = EASY;
            if (nk_option_label(&(context->ctx), "hard", op == HARD)) op = HARD;
            nk_layout_row_dynamic(&(context->ctx), 45, 1);
            nk_property_int(&(context->ctx), "Compression:", 0, &property, 100, 10, 1);
        }
        nk_end(&(context->ctx));

        /* grid_demo(&(context->ctx)); */

        /* -------------- EXAMPLES ---------------- */
        #ifdef INCLUDE_CALCULATOR
          calculator(&(context->ctx));
        #endif
        #ifdef INCLUDE_CANVAS
          canvas(&(context->ctx));
        #endif
        #ifdef INCLUDE_OVERVIEW
          overview(&(context->ctx));
        #endif
        #ifdef INCLUDE_NODE_EDITOR
          node_editor(&(context->ctx));
        #endif
        /* ----------------------------------------- */

        nk_rawfb_render(context, clear, 1);

        RGFW_window_swapBuffers(window);
    }

    nk_rawfb_shutdown(context);

    RGFW_window_close(window);
}