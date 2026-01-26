/* nuklear - public domain */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <limits.h>
#include <time.h>

#include <SDL3/SDL.h>

/* This demo uses "main callbacks" which are new in SDL3
 * Those provide highly portable entry point and event loop for the app
 * see: https://wiki.libsdl.org/SDL3/README-main-functions
 * */
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>

/* ===============================================================
 *
 *                          CONFIG
 *
 * ===============================================================*/

/* optional: sdl3_renderer does not need any of these defines
 * (but some examples might need them, so be careful) */
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_STANDARD_IO

/* note that sdl3_renderer comes with nk_sdl_style_set_debug_font()
 * so you may wish to use that instead of font baking */
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT

/* note that sdl3_renderer comes with nk_sdl_allocator()
 * and you probably want to use that allocator instead of the default ones */
/*#define NK_INCLUDE_DEFAULT_ALLOCATOR*/

/* mandatory: sdl3_renderer depends on those defines */
#define NK_INCLUDE_COMMAND_USERDATA
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT


/* We can re-use the types provided by SDL which are extremely portable,
 * so there is no need for Nuklear to detect those on its own */
/*#define NK_INCLUDE_FIXED_TYPES*/
#ifndef NK_INCLUDE_FIXED_TYPES
    #define NK_INT8              Sint8
    #define NK_UINT8             Uint8
    #define NK_INT16             Sint16
    #define NK_UINT16            Uint16
    #define NK_INT32             Sint32
    #define NK_UINT32            Uint32
    /* SDL guarantees 'uintptr_t' typedef */
    #define NK_SIZE_TYPE         uintptr_t
    #define NK_POINTER_TYPE      uintptr_t
#endif

/* FIXME: We could also use the `bool` symbol provided by SDL,
 * but this is currently broken due to internal Nuklear issue, see:
 * https://github.com/Immediate-Mode-UI/Nuklear/issues/849
 * */
#define NK_INCLUDE_STANDARD_BOOL
#ifndef NK_INCLUDE_STANDARD_BOOL
    #define NK_BOOL               bool
#endif

/* We can re-use various portable libc functions provided by SDL */
#define NK_ASSERT(condition)      SDL_assert(condition)
#define NK_STATIC_ASSERT(exp)     SDL_COMPILE_TIME_ASSERT(, exp)
#define NK_MEMSET(dst, c, len)    SDL_memset(dst, c, len)
#define NK_MEMCPY(dst, src, len)  SDL_memcpy(dst, src, len)
#define NK_VSNPRINTF(s, n, f, a)  SDL_vsnprintf(s, n, f, a)
#define NK_STRTOD(str, endptr)    SDL_strtod(str, endptr)

/* sadly, SDL3 does not provide "dtoa" (only integer version) */
/*#define NK_DTOA (str, d)*/

/* SDL can also provide us with math functions, but beware that Nuklear's own
 * implementation can be slightly faster at the cost of some precision */
#define NK_INV_SQRT(f)            (1.0f / SDL_sqrtf(f))
#define NK_SIN(f)                 SDL_sinf(f)
#define NK_COS(f)                 SDL_cosf(f)

/* HACK: Nuklear pulls two stb libraries in order to use font baking
 * those libraries pull in some libc headers internally, creating a linkage dependency,
 * so you’ll most likely want to use SDL symbols instead */
#define STBTT_ifloor(x)       ((int)SDL_floor(x))
#define STBTT_iceil(x)        ((int)SDL_ceil(x))
#define STBTT_sqrt(x)         SDL_sqrt(x)
#define STBTT_pow(x,y)        SDL_pow(x,y)
#define STBTT_fmod(x,y)       SDL_fmod(x,y)
#define STBTT_cos(x)          SDL_cosf(x)
#define STBTT_acos(x)         SDL_acos(x)
#define STBTT_fabs(x)         SDL_fabs(x)
#define STBTT_assert(x)       SDL_assert(x)
#define STBTT_strlen(x)       SDL_strlen(x)
#define STBTT_memcpy          SDL_memcpy
#define STBTT_memset          SDL_memset
#define stbtt_uint8           Uint8
#define stbtt_int8            Sint8
#define stbtt_uint16          Uint16
#define stbtt_int16           Sint16
#define stbtt_uint32          Uint32
#define stbtt_int32           Sint32
#define STBRP_SORT            SDL_qsort
#define STBRP_ASSERT          SDL_assert
/* There is no need to define STBTT_malloc/STBTT_free macros
 * Nuklear will define those to user-provided nk_allocator */


#define NK_IMPLEMENTATION
#include "../../nuklear.h"
#define NK_SDL3_RENDERER_IMPLEMENTATION
#include "nuklear_sdl3_renderer.h"

#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 800

/* ===============================================================
 *
 *                          EXAMPLE
 *
 * ===============================================================*/
/* These are some code examples to provide a small overview of what can be
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
    #include "../../demo/common/style.c"
#endif
#ifdef INCLUDE_CALCULATOR
    #include "../../demo/common/calculator.c"
#endif
#ifdef INCLUDE_CANVAS
    #include "../../demo/common/canvas.c"
#endif
#ifdef INCLUDE_OVERVIEW
    #include "../../demo/common/overview.c"
#endif
#ifdef INCLUDE_CONFIGURATOR
    #include "../../demo/common/style_configurator.c"
#endif
#ifdef INCLUDE_NODE_EDITOR
    #include "../../demo/common/node_editor.c"
#endif

/* ===============================================================
 *
 *                          DEMO
 *
 * ===============================================================*/

struct nk_sdl_app {
    SDL_Window* window;
    SDL_Renderer* renderer;
    struct nk_context * ctx;
    struct nk_colorf bg;
    enum nk_anti_aliasing AA;
};

static SDL_AppResult
nk_sdl_fail()
{
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Error: %s", SDL_GetError());
    return SDL_APP_FAILURE;
}

SDL_AppResult
SDL_AppInit(void** appstate, int argc, char* argv[])
{
    struct nk_sdl_app* app;
    struct nk_context* ctx;
    float font_scale;
    NK_UNUSED(argc);
    NK_UNUSED(argv);

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        return nk_sdl_fail();
    }

    app = SDL_malloc(sizeof(*app));
    if (app == NULL) {
        return nk_sdl_fail();
    }

    if (!SDL_CreateWindowAndRenderer("Nuklear: SDL3 Renderer", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE, &app->window, &app->renderer)) {
        SDL_free(app);
        return nk_sdl_fail();
    }
    *appstate = app;

    if (!SDL_SetRenderVSync(app->renderer, 1)) {
        SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "SDL_SetRenderVSync failed: %s", SDL_GetError());
    }

    app->bg.r = 0.10f;
    app->bg.g = 0.18f;
    app->bg.b = 0.24f;
    app->bg.a = 1.0f;


    font_scale = 1;
    {
        /* This scaling logic was kept simple for the demo purpose.
         * On some platforms, this might not be the exact scale
         * that you want to use. For more information, see:
         * https://wiki.libsdl.org/SDL3/README-highdpi */
        const float scale = SDL_GetWindowDisplayScale(app->window);
        SDL_SetRenderScale(app->renderer, scale, scale);
        font_scale = scale;
    }

    ctx = nk_sdl_init(app->window, app->renderer, nk_sdl_allocator());
    app->ctx = ctx;

#if 0
    {
        /* If you don't want to use advanced Nuklear font baking API
         * you can use simple ASCII debug font provided by SDL
         * just change the `#if 0` above to `#if 1` */
        nk_sdl_style_set_debug_font(ctx);

        /* Note that since debug font is extremely small (only 8x8 pixels),
         * scaling it does not make much sense. The font would appear blurry. */
        NK_UNUSED(font_scale);

        /* You may wish to change a few style options, here are few recommendations: */
        ctx->style.button.rounding = 0.0f;
        ctx->style.menu_button.rounding = 0.0f;
        ctx->style.property.rounding = 0.0f;
        ctx->style.property.border = 0.0f;
        ctx->style.option.border = -1.0f;
        ctx->style.checkbox.border = -1.0f;
        ctx->style.property.dec_button.border = -2.0f;
        ctx->style.property.inc_button.border = -2.0f;
        ctx->style.tab.tab_minimize_button.border = -2.0f;
        ctx->style.tab.tab_maximize_button.border = -2.0f;
        ctx->style.tab.node_minimize_button.border = -2.0f;
        ctx->style.tab.node_maximize_button.border = -2.0f;
        ctx->style.checkbox.spacing = 5.0f;

        /* It's better to disable anti-aliasing when using small fonts */
        app->AA = NK_ANTI_ALIASING_OFF;
    }
#else
    {
        struct nk_font_atlas *atlas;
        struct nk_font_config config = nk_font_config(0);
        struct nk_font *font;

        /* set up the font atlas and add desired font; note that font sizes are
         * multiplied by font_scale to produce better results at higher DPIs */
        atlas = nk_sdl_font_stash_begin(ctx);
        font = nk_font_atlas_add_default(atlas, 13 * font_scale, &config);
        /*font = nk_font_atlas_add_from_file(atlas, "../../../extra_font/DroidSans.ttf", 14 * font_scale, &config);*/
        /*font = nk_font_atlas_add_from_file(atlas, "../../../extra_font/Roboto-Regular.ttf", 16 * font_scale, &config);*/
        /*font = nk_font_atlas_add_from_file(atlas, "../../../extra_font/kenvector_future_thin.ttf", 13 * font_scale, &config);*/
        /*font = nk_font_atlas_add_from_file(atlas, "../../../extra_font/ProggyClean.ttf", 12 * font_scale, &config);*/
        /*font = nk_font_atlas_add_from_file(atlas, "../../../extra_font/ProggyTiny.ttf", 10 * font_scale, &config);*/
        /*font = nk_font_atlas_add_from_file(atlas, "../../../extra_font/Cousine-Regular.ttf", 13 * font_scale, &config);*/
        nk_sdl_font_stash_end(ctx);

        /* this hack makes the font appear to be scaled down to the desired
         * size and is only necessary when font_scale > 1 */
        font->handle.height /= font_scale;
        /*nk_style_load_all_cursors(ctx, atlas->cursors);*/
        nk_style_set_font(ctx, &font->handle);

        app->AA = NK_ANTI_ALIASING_ON;
    }
#endif

    return SDL_APP_CONTINUE;
}

SDL_AppResult
SDL_AppEvent(void *appstate, SDL_Event* event)
{
    struct nk_sdl_app* app = (struct nk_sdl_app*)appstate;

    switch (event->type) {
        case SDL_EVENT_QUIT:
            return SDL_APP_SUCCESS;
        case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED:
            /* You may wish to rescale the renderer and Nuklear during this event.
             * Without this the UI and Font could appear too small or too big.
             * This is not handled by the demo in order to keep it simple,
             * but you may wish to re-bake the Font whenever this happens. */
            SDL_Log("Unhandled scale event! Nuklear may appear blurry");
            return SDL_APP_CONTINUE;
    }

    /* Remember to always rescale the event coordinates,
     * if your renderer uses custom scale. */
    SDL_ConvertEventToRenderCoordinates(app->renderer, event);

    nk_sdl_handle_event(app->ctx, event);

    return SDL_APP_CONTINUE;
}

SDL_AppResult
SDL_AppIterate(void *appstate)
{
    struct nk_sdl_app* app = (struct nk_sdl_app*)appstate;
    struct nk_context* ctx = app->ctx;

#ifdef INCLUDE_CONFIGURATOR
    static struct nk_color color_table[NK_COLOR_COUNT];
    NK_MEMCPY(color_table, nk_default_color_style, sizeof(color_table));
#endif

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
        if (nk_button_label(ctx, "button")) {
            SDL_Log("button pressed");
        }
        nk_layout_row_dynamic(ctx, 30, 2);
        if (nk_option_label(ctx, "easy", op == EASY)) op = EASY;
        if (nk_option_label(ctx, "hard", op == HARD)) op = HARD;
        nk_layout_row_dynamic(ctx, 25, 1);
        nk_property_int(ctx, "Compression:", 0, &property, 1000, 1, 1);

        nk_layout_row_dynamic(ctx, 20, 1);
        nk_label(ctx, "background:", NK_TEXT_LEFT);
        nk_layout_row_dynamic(ctx, 25, 1);
        if (nk_combo_begin_color(ctx, nk_rgb_cf(app->bg), nk_vec2(nk_widget_width(ctx),400))) {
            nk_layout_row_dynamic(ctx, 120, 1);
            app->bg = nk_color_picker(ctx, app->bg, NK_RGBA);
            nk_layout_row_dynamic(ctx, 25, 1);
            app->bg.r = nk_propertyf(ctx, "#R:", 0, app->bg.r, 1.0f, 0.01f,0.005f);
            app->bg.g = nk_propertyf(ctx, "#G:", 0, app->bg.g, 1.0f, 0.01f,0.005f);
            app->bg.b = nk_propertyf(ctx, "#B:", 0, app->bg.b, 1.0f, 0.01f,0.005f);
            app->bg.a = nk_propertyf(ctx, "#A:", 0, app->bg.a, 1.0f, 0.01f,0.005f);
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
    #ifdef INCLUDE_OVERVIEW
        overview(ctx);
    #endif
    #ifdef INCLUDE_CONFIGURATOR
        style_configurator(ctx, color_table);
    #endif
    #ifdef INCLUDE_NODE_EDITOR
        node_editor(ctx);
    #endif
    /* ----------------------------------------- */

    SDL_SetRenderDrawColorFloat(app->renderer, app->bg.r, app->bg.g, app->bg.b, app->bg.a);
    SDL_RenderClear(app->renderer);

    nk_sdl_render(ctx, app->AA);
    nk_sdl_update_TextInput(ctx);

    /* show if TextInput is active for debug purpose. Feel free to remove this. */
    SDL_SetRenderDrawColor(app->renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderDebugTextFormat(app->renderer, 10, 10, "TextInputActive? %s",
                              SDL_TextInputActive(app->window) ? "Yes" : "No");

    SDL_RenderPresent(app->renderer);

    nk_input_begin(ctx);
    return SDL_APP_CONTINUE;
}

void
SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    struct nk_sdl_app* app = (struct nk_sdl_app*)appstate;
    NK_UNUSED(result);

    if (app) {
        nk_sdl_shutdown(app->ctx);
        SDL_DestroyRenderer(app->renderer);
        SDL_DestroyWindow(app->window);
        SDL_free(app);
    }
}

