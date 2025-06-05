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
#include "../../../nuklear.h"
#define NK_RAWFB_IMPLEMENTATION
#include "../nuklear_rawfb.h"

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

static int translate_sdl_key(struct SDL_Keysym const *k)
{
    /*keyboard handling left as an exercise for the reader */
    NK_UNUSED(k);

    return NK_KEY_NONE;
}


static int sdl_button_to_nk(int button)
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

    SDL_DisplayMode dm;
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *tex;
    SDL_Surface *surface;

    #ifdef INCLUDE_CONFIGURATOR
    static struct nk_color color_table[NK_COLOR_COUNT];
    memcpy(color_table, nk_default_color_style, sizeof(color_table));
    #endif

    NK_UNUSED(argc);
    NK_UNUSED(argv);

    SDL_Init(SDL_INIT_VIDEO);
    printf("sdl init called...\n");

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

    SDL_GetDesktopDisplayMode(0, &dm);

    printf("desktop display mode %d %d\n", dm.w, dm.h);


    window = SDL_CreateWindow("Puzzle", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, dm.w-200,dm.h-200, SDL_WINDOW_OPENGL);
    if (!window)
    {
        printf("can't open window!\n");
        exit(1);
    }


    renderer = SDL_CreateRenderer(window, -1, 0);

    surface = SDL_CreateRGBSurfaceWithFormat(0, dm.w-200, dm.h-200, 32, SDL_PIXELFORMAT_ARGB8888);

    pl.bytesPerPixel = surface->format->BytesPerPixel;
    pl.rshift = surface->format->Rshift;
    pl.gshift = surface->format->Gshift;
    pl.bshift = surface->format->Bshift;
    pl.ashift = surface->format->Ashift;
    pl.rloss = surface->format->Rloss;
    pl.gloss = surface->format->Gloss;
    pl.bloss = surface->format->Bloss;
    pl.aloss = surface->format->Aloss;

    context = nk_rawfb_init(surface->pixels, tex_scratch, surface->w, surface->h, surface->pitch, pl);


    while(1)
    {
        SDL_Event event;
        nk_input_begin(&(context->ctx));
        while (SDL_PollEvent(&event))
        {
            switch(event.type)
            {
                case SDL_QUIT:
                    exit(0);
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
                    vec.x = event.wheel.preciseX;
                    vec.y = event.wheel.preciseY;
                    nk_input_scroll(&(context->ctx), vec );

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
        #ifdef INCLUDE_CONFIGURATOR
          style_configurator(&(context->ctx), color_table);
        #endif
        #ifdef INCLUDE_NODE_EDITOR
          node_editor(&(context->ctx));
        #endif
        /* ----------------------------------------- */

        nk_rawfb_render(context, clear, 1);




        tex = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_RenderCopy(renderer, tex, NULL, NULL);
        SDL_RenderPresent(renderer);
        SDL_DestroyTexture(tex);

    }

    nk_rawfb_shutdown(context);

    SDL_FreeSurface(surface);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}


