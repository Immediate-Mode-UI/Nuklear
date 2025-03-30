/* nuklear - public domain */

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_SDL3_RENDERER_IMPLEMENTATION
#define NK_INCLUDE_STANDARD_BOOL
#define NK_INCLUDE_COMMAND_USERDATA
#define NK_MFREE(userdata, ptr) SDL_free(ptr)
#define NK_MALLOC(userdata, old, size) SDL_malloc(size)
#include "nuklear.h"
#include "nuklear_sdl3_renderer.h"

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
#define INCLUDE_OVERVIEW
/*#define INCLUDE_CONFIGURATOR */
/*#define INCLUDE_NODE_EDITOR */

/*#define INCLUDE_ALL*/

#ifdef INCLUDE_ALL
    #define INCLUDE_STYLE
    #define INCLUDE_CALCULATOR
    #define INCLUDE_CANVAS
    #define INCLUDE_OVERVIEW
    /* #define INCLUDE_CONFIGURATOR */
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

typedef struct AppContext {
    SDL_Window* window;
    SDL_Renderer* renderer;
    struct nk_context * ctx;
    struct nk_colorf bg;
} AppContext;

SDL_AppResult SDL_Fail(){
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Error %s", SDL_GetError());
    return SDL_APP_FAILURE;
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
    NK_UNUSED(argc);
    NK_UNUSED(argv);
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        return SDL_Fail();
    }

    AppContext* appContext = (AppContext*)SDL_malloc(sizeof(AppContext));
    if (appContext == NULL) {
        return SDL_Fail();
    }

    if (!SDL_CreateWindowAndRenderer("Nuklear: SDL3 Renderer", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE, &appContext->window, &appContext->renderer)) {
        SDL_free(appContext);
        return SDL_Fail();
    }

    SDL_StartTextInput(appContext->window);

    appContext->bg.r = 0.10f;
    appContext->bg.g = 0.18f;
    appContext->bg.b = 0.24f;
    appContext->bg.a = 1.0f;

    *appstate = appContext;

    float font_scale = 1;

    {
        int render_w, render_h;
        int window_w, window_h;
        float scale_x, scale_y;
        SDL_GetCurrentRenderOutputSize(appContext->renderer, &render_w, &render_h);
        SDL_GetWindowSize(appContext->window, &window_w, &window_h);
        scale_x = (float)(render_w) / (float)(window_w);
        scale_y = (float)(render_h) / (float)(window_h);
        SDL_SetRenderScale(appContext->renderer, scale_x, scale_y);
        font_scale = scale_y;
    }

    struct nk_context* ctx = nk_sdl_init(appContext->window, appContext->renderer);
    appContext->ctx = ctx;

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
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event* event) {
    AppContext* app = (AppContext*)appstate;

    switch (event->type) {
        case SDL_EVENT_QUIT:
            return SDL_APP_SUCCESS;
    }

    nk_sdl_handle_event(app->ctx, event);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    AppContext* app = (AppContext*)appstate;
    struct nk_context* ctx = app->ctx;
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

    SDL_SetRenderDrawColor(app->renderer,
        (Uint8)(app->bg.r * 255.0f),
        (Uint8)(app->bg.g * 255.0f),
        (Uint8)(app->bg.b * 255.0f),
        SDL_ALPHA_OPAQUE);
    SDL_RenderClear(app->renderer);

    nk_sdl_render(ctx, NK_ANTI_ALIASING_ON);

    SDL_RenderPresent(app->renderer);

    nk_input_begin(ctx);
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
    AppContext* app = (AppContext*)appstate;
    NK_UNUSED(result);

    if (app) {
        nk_sdl_shutdown(app->ctx);
        SDL_DestroyRenderer(app->renderer);
        SDL_DestroyWindow(app->window);
        SDL_free(app);
    }

    SDL_Quit();
}
