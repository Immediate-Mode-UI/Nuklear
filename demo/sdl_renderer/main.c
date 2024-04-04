/* nuklear - 1.32.0 - public domain */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <limits.h>
#include <time.h>

#include <SDL2/SDL.h>
#ifdef USE_SDL_IMAGE
#include <SDL2/SDL_image.h>
#endif

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_SDL_RENDERER_IMPLEMENTATION
#include "../../nuklear.h"
#include "nuklear_sdl_renderer.h"

#ifndef USE_SDL_IMAGE
#define STB_IMAGE_IMPLEMENTATION
#include "../../demo/common/filebrowser/stb_image.h"
#endif

#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 800

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
/*#define INCLUDE_FILE_BROWSER */
/*#define INCLUDE_OVERVIEW */
/*#define INCLUDE_NODE_EDITOR */

#ifdef INCLUDE_ALL
  #define INCLUDE_STYLE
  #define INCLUDE_CALCULATOR
  #define INCLUDE_CANVAS
  #define INCLUDE_FILE_BROWSER
  #define INCLUDE_OVERVIEW
  #define INCLUDE_NODE_EDITOR
#endif

#ifdef INCLUDE_STYLE
  #include "../../demo/common/style.c"
#endif
#ifdef INCLUDE_CALCULATOR
  #include "../../demo/common/calculator.c"
#endif
#ifdef INCLUDE_CANVAS
  #include "../../demo/common/canvas.c"
#endif
#ifdef INCLUDE_FILE_BROWSER
  #include "../../demo/common/file_browser.c"
#endif
#ifdef INCLUDE_OVERVIEW
  #include "../../demo/common/overview.c"
#endif
#ifdef INCLUDE_NODE_EDITOR
  #include "../../demo/common/node_editor.c"
#endif

static void
die(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputs("\n", stderr);
    exit(EXIT_FAILURE);
}

static struct nk_image
icon_load(const char *filename)
{
#ifdef USE_SDL_IMAGE
    SDL_Texture *tex = IMG_LoadTexture(sdl.renderer, filename);
    if (!tex) die("[SDL]: failed to load image: %s", filename);
#else
    int x,y,n;
    SDL_Texture *tex;
    unsigned char *data = stbi_load(filename, &x, &y, &n, 0);
    if (!data) die("[SDL]: failed to load image: %s", filename);

    tex = SDL_CreateTexture(sdl.renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, x, y);
    if (!tex) die("error creating texture");

    SDL_UpdateTexture(tex, NULL, data, x * 4);
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
    stbi_image_free(data);
#endif
    return nk_image_ptr(tex);
}

/* ===============================================================
 *
 *                          DEMO
 *
 * ===============================================================*/
int
main(int argc, char *argv[])
{
    /* Platform */
    SDL_Window *win;
    SDL_Renderer *renderer;
    int running = 1;
    int flags = 0;
    float font_scale = 1;

    /* GUI */
    struct nk_context *ctx;
    struct nk_colorf bg;
#ifdef INCLUDE_FILE_BROWSER
    struct file_browser browser;
    struct media media;
#endif

    /* SDL setup */
    SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0");
    SDL_Init(SDL_INIT_VIDEO);

    win = SDL_CreateWindow("Demo",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN|SDL_WINDOW_ALLOW_HIGHDPI);

    if (win == NULL) {
        SDL_Log("Error SDL_CreateWindow %s", SDL_GetError());
        exit(-1);
    }

    flags |= SDL_RENDERER_ACCELERATED;
    flags |= SDL_RENDERER_PRESENTVSYNC;

#if 0
    SDL_SetHint(SDL_HINT_RENDER_BATCHING, "1");
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "software");
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "software");
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengles2");
#endif

    renderer = SDL_CreateRenderer(win, -1, flags);

    if (renderer == NULL) {
        SDL_Log("Error SDL_CreateRenderer %s", SDL_GetError());
        exit(-1);
    }

    /* scale the renderer output for High-DPI displays */
    {
        int render_w, render_h;
        int window_w, window_h;
        float scale_x, scale_y;
        SDL_GetRendererOutputSize(renderer, &render_w, &render_h);
        SDL_GetWindowSize(win, &window_w, &window_h);
        scale_x = (float)(render_w) / (float)(window_w);
        scale_y = (float)(render_h) / (float)(window_h);
        SDL_RenderSetScale(renderer, scale_x, scale_y);
        font_scale = scale_y;
    }

    /* GUI */
    ctx = nk_sdl_init(win, renderer);
    /* Load Fonts: if none of these are loaded a default font will be used  */
    /* Load Cursor: if you uncomment cursor loading please hide the cursor */
    {
        struct nk_font_atlas *atlas;
        struct nk_font_config config = nk_font_config(0);
        struct nk_font *font;

        /* set up the font atlas and add desired font; note that font sizes are
         * multiplied by font_scale to produce better results at higher DPIs */
        nk_sdl_font_stash_begin(&atlas);
        font = nk_font_atlas_add_default(atlas, 13 * font_scale, &config);
        /*font = nk_font_atlas_add_from_file(atlas, "../../../extra_font/DroidSans.ttf", 14 * font_scale, &config);*/
        /*font = nk_font_atlas_add_from_file(atlas, "../../../extra_font/Roboto-Regular.ttf", 16 * font_scale, &config);*/
        /*font = nk_font_atlas_add_from_file(atlas, "../../../extra_font/kenvector_future_thin.ttf", 13 * font_scale, &config);*/
        /*font = nk_font_atlas_add_from_file(atlas, "../../../extra_font/ProggyClean.ttf", 12 * font_scale, &config);*/
        /*font = nk_font_atlas_add_from_file(atlas, "../../../extra_font/ProggyTiny.ttf", 10 * font_scale, &config);*/
        /*font = nk_font_atlas_add_from_file(atlas, "../../../extra_font/Cousine-Regular.ttf", 13 * font_scale, &config);*/
        nk_sdl_font_stash_end();

        /* this hack makes the font appear to be scaled down to the desired
         * size and is only necessary when font_scale > 1 */
        font->handle.height /= font_scale;
        /*nk_style_load_all_cursors(ctx, atlas->cursors);*/
        nk_style_set_font(ctx, &font->handle);
    }

    #ifdef INCLUDE_STYLE
    /* ease regression testing during Nuklear release process; not needed for anything else */
    #ifdef STYLE_WHITE
    set_style(ctx, THEME_WHITE);
    #elif defined(STYLE_RED)
    set_style(ctx, THEME_RED);
    #elif defined(STYLE_BLUE)
    set_style(ctx, THEME_BLUE);
    #elif defined(STYLE_DARK)
    set_style(ctx, THEME_DARK);
    #endif
    #endif

    bg.r = 0.10f, bg.g = 0.18f, bg.b = 0.24f, bg.a = 1.0f;

    #ifdef INCLUDE_FILE_BROWSER
    /* icons */
    media.icons.home = icon_load("../../demo/common/filebrowser/icon/home.png");
    media.icons.directory = icon_load("../../demo/common/filebrowser/icon/directory.png");
    media.icons.computer = icon_load("../../demo/common/filebrowser/icon/computer.png");
    media.icons.desktop = icon_load("../../demo/common/filebrowser/icon/desktop.png");
    media.icons.default_file = icon_load("../../demo/common/filebrowser/icon/default.png");
    media.icons.text_file = icon_load("../../demo/common/filebrowser/icon/text.png");
    media.icons.music_file = icon_load("../../demo/common/filebrowser/icon/music.png");
    media.icons.font_file =  icon_load("../../demo/common/filebrowser/icon/font.png");
    media.icons.img_file = icon_load("../../demo/common/filebrowser/icon/img.png");
    media.icons.movie_file = icon_load("../../demo/common/filebrowser/icon/movie.png");
    media_init(&media);

    file_browser_init(&browser, &media);
    #endif

    while (running)
    {
        /* Input */
        SDL_Event evt;
        nk_input_begin(ctx);
        while (SDL_PollEvent(&evt)) {
            if (evt.type == SDL_QUIT) goto cleanup;
            nk_sdl_handle_event(&evt);
        }
        nk_sdl_handle_grab(); /* optional grabbing behavior */
        nk_input_end(ctx);

        /* GUI */
        if (nk_begin(ctx, "Demo", nk_rect(50, 50, 230, 250),
            NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
            NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE))
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

            nk_layout_row_dynamic(ctx, 20, 1);
            nk_label(ctx, "background:", NK_TEXT_LEFT);
            nk_layout_row_dynamic(ctx, 25, 1);
            if (nk_combo_begin_color(ctx, nk_rgb_cf(bg), nk_vec2(nk_widget_width(ctx),400))) {
                nk_layout_row_dynamic(ctx, 120, 1);
                bg = nk_color_picker(ctx, bg, NK_RGBA);
                nk_layout_row_dynamic(ctx, 25, 1);
                bg.r = nk_propertyf(ctx, "#R:", 0, bg.r, 1.0f, 0.01f,0.005f);
                bg.g = nk_propertyf(ctx, "#G:", 0, bg.g, 1.0f, 0.01f,0.005f);
                bg.b = nk_propertyf(ctx, "#B:", 0, bg.b, 1.0f, 0.01f,0.005f);
                bg.a = nk_propertyf(ctx, "#A:", 0, bg.a, 1.0f, 0.01f,0.005f);
                nk_combo_end(ctx);
            }
        }
        nk_end(ctx);

        /* -------------- EXAMPLES ---------------- */
        #ifdef INCLUDE_CALCULATOR
          calculator(ctx);
        #endif
        #ifdef INCLUDE_CANVAS
        canvas(ctx);
        #endif
        #ifdef INCLUDE_FILE_BROWSER
          file_browser_run(&browser, ctx);
        #endif
        #ifdef INCLUDE_OVERVIEW
          overview(ctx);
        #endif
        #ifdef INCLUDE_NODE_EDITOR
          node_editor(ctx);
        #endif
        /* ----------------------------------------- */

        SDL_SetRenderDrawColor(renderer, bg.r * 255, bg.g * 255, bg.b * 255, bg.a * 255);
        SDL_RenderClear(renderer);

        nk_sdl_render(NK_ANTI_ALIASING_ON);

        SDL_RenderPresent(renderer);
    }

cleanup:
    #ifdef INCLUDE_FILE_BROWSER
    SDL_DestroyTexture(media.icons.home.handle.ptr);
    SDL_DestroyTexture(media.icons.directory.handle.ptr);
    SDL_DestroyTexture(media.icons.computer.handle.ptr);
    SDL_DestroyTexture(media.icons.desktop.handle.ptr);
    SDL_DestroyTexture(media.icons.default_file.handle.ptr);
    SDL_DestroyTexture(media.icons.text_file.handle.ptr);
    SDL_DestroyTexture(media.icons.music_file.handle.ptr);
    SDL_DestroyTexture(media.icons.font_file.handle.ptr);
    SDL_DestroyTexture(media.icons.img_file.handle.ptr);
    SDL_DestroyTexture(media.icons.movie_file.handle.ptr);

    file_browser_free(&browser);
    #endif

    nk_sdl_shutdown();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
