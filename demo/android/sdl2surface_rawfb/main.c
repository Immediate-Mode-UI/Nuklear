#include <SDL.h>
#include <SDL_mouse.h>
#include <SDL_keyboard.h>

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
#define NK_SDLSURFACE_IMPLEMENTATION
#include "sdl2surface_rawfb.h"

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
  #include "../style.c"
#endif
#ifdef INCLUDE_CALCULATOR
  #include "../calculator.c"
#endif
#ifdef INCLUDE_CANVAS
  #include "../canvas.c"
#endif
#ifdef INCLUDE_OVERVIEW
  #include "../overview.c"
#endif
#ifdef INCLUDE_NODE_EDITOR
  #include "../node_editor.c"
#endif

SDL_Surface *surface = NULL;
struct sdlsurface_context *context = NULL;

static int
translate_sdl_key(struct SDL_Keysym const *k)
{
    /*keyboard handling left as an exercise for the reader */
    NK_UNUSED(k);

    return NK_KEY_NONE;
}

static int
sdl_button_to_nk(int button)
{
    switch(button)
    {
        default:
        /* ft */
        case SDL_BUTTON_LEFT:
            return NK_BUTTON_LEFT;
            break;
        case SDL_BUTTON_MIDDLE:
            return NK_BUTTON_MIDDLE;
            break;
        case SDL_BUTTON_RIGHT:
            return NK_BUTTON_RIGHT;
            break;

    }
}

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

static void
create_surface(int width, int height){
    surface = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_ARGB8888);
    context = nk_sdlsurface_init(surface, 13.0f);
}

int main(int argc, char **argv)
{
    struct nk_color clear = {28,48,62,255};
    struct nk_vec2 vec;
    struct nk_rect bounds = {40,40,400,400};

    SDL_DisplayMode display;
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *tex;
    SDL_Rect gui_rect;

    NK_UNUSED(argc);
    NK_UNUSED(argv);

    int flags = SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER;

    SDL_Init(flags);
    printf("sdl init called...\n");

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

    SDL_GetDesktopDisplayMode(0, &display);

    printf("desktop display mode %d %d\n", display.w, display.h);

    gui_rect.x = 0;
    gui_rect.y = 0;
    gui_rect.w = display.w;
    gui_rect.h = display.h;

    flags = SDL_WINDOW_OPENGL|SDL_WINDOW_ALLOW_HIGHDPI|SDL_WINDOW_FULLSCREEN|SDL_WINDOW_RESIZABLE;

    window = SDL_CreateWindow("Nuklear", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, display.w,display.h, flags);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    if (!window)
    {
        printf("can't open window!\n");
        exit(1);
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    create_surface(display.w, display.h);

    SDL_Event event;

    while(1)
    {
        nk_input_begin(&(context->ctx));
        while (SDL_PollEvent(&event))
        {
            switch(event.type)
            {
                case SDL_QUIT:
                    goto exit;
                break;
                case SDL_KEYDOWN:
                    nk_input_key(&(context->ctx), translate_sdl_key(&event.key.keysym), 1);
                break;
                case SDL_KEYUP:
                    nk_input_key(&(context->ctx), translate_sdl_key(&event.key.keysym), 0);
                break;
                case SDL_MOUSEMOTION:
                    nk_input_motion(&(context->ctx), event.motion.x, event.motion.y);
                break;
                case SDL_MOUSEBUTTONDOWN:
                    nk_input_button(&(context->ctx), sdl_button_to_nk(event.button.button), event.button.x, event.button.y,1);
                break;
                case SDL_MOUSEBUTTONUP:
                    nk_input_button(&(context->ctx), sdl_button_to_nk(event.button.button), event.button.x, event.button.y,0);
                break;
                case SDL_MOUSEWHEEL:
                    vec.x = event.wheel.x;
                    vec.y = event.wheel.y;
                    nk_input_scroll(&(context->ctx), vec );
                break;
                case SDL_WINDOWEVENT:
                    switch (event.window.event) {
                        case SDL_WINDOWEVENT_SIZE_CHANGED:
                            display.w = gui_rect.w = event.window.data1;
                            display.h = gui_rect.h = event.window.data2;
                            create_surface(event.window.data1, event.window.data2);
                            break;
                        case SDL_WINDOWEVENT_EXPOSED:
                            break;
                    }
                    break;
            }
        }
        nk_input_end(&(context->ctx));

        /* GUI */
        if (nk_begin(&(context->ctx), "Demo", nk_rect(50, 50, 200, 200),
                     NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
                     NK_WINDOW_CLOSABLE|NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE))
        {
            nk_menubar_begin(&(context->ctx));
            nk_layout_row_begin(&(context->ctx), NK_STATIC, 25, 2);
            nk_layout_row_push(&(context->ctx), 45);
            if (nk_menu_begin_label(&(context->ctx), "FILE", NK_TEXT_LEFT, nk_vec2(120, 200))) {
                nk_layout_row_dynamic(&(context->ctx), 30, 1);
                nk_menu_item_label(&(context->ctx), "OPEN", NK_TEXT_LEFT);
                nk_menu_item_label(&(context->ctx), "CLOSE", NK_TEXT_LEFT);
                nk_menu_end(&(context->ctx));
            }
            nk_layout_row_push(&(context->ctx), 45);
            if (nk_menu_begin_label(&(context->ctx), "EDIT", NK_TEXT_LEFT, nk_vec2(120, 200))) {
                nk_layout_row_dynamic(&(context->ctx), 30, 1);
                nk_menu_item_label(&(context->ctx), "COPY", NK_TEXT_LEFT);
                nk_menu_item_label(&(context->ctx), "CUT", NK_TEXT_LEFT);
                nk_menu_item_label(&(context->ctx), "PASTE", NK_TEXT_LEFT);
                nk_menu_end(&(context->ctx));
            }
            nk_layout_row_end(&(context->ctx));
            nk_menubar_end(&(context->ctx));

            {
                enum {EASY, HARD};
                static int op = EASY;
                static int property = 20;
                nk_layout_row_static(&(context->ctx), 30, 80, 1);
                if (nk_button_label(&(context->ctx), "button"))
                    fprintf(stdout, "button pressed\n");
                nk_layout_row_dynamic(&(context->ctx), 30, 2);
                if (nk_option_label(&(context->ctx), "easy", op == EASY)) op = EASY;
                if (nk_option_label(&(context->ctx), "hard", op == HARD)) op = HARD;
                nk_layout_row_dynamic(&(context->ctx), 25, 1);
                nk_property_int(&(context->ctx), "Compression:", 0, &property, 100, 10, 1);
            }
        }
        nk_end(&(context->ctx));

        grid_demo(&(context->ctx));

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

        nk_sdlsurface_render(context, clear, 1);

        tex = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_RenderCopy(renderer, tex, NULL, NULL);
        SDL_RenderPresent(renderer);
        SDL_DestroyTexture(tex);

    }
    exit:
    nk_sdlsurface_shutdown(context);
    SDL_FreeSurface(surface);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    exit(0);
}


