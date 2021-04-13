/* nuklear - v1.05 - public domain */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <time.h>
#include <limits.h>

#include <SDL.h>
#include <SDL_mouse.h>
#include <SDL_keyboard.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_INCLUDE_SOFTWARE_FONT
#define NK_IMPLEMENTATION
#include "../../nuklear.h"
#define NK_SDLSURFACE_IMPLEMENTATION
#include "sdl2surface_rawfb.h"

/* ===============================================================
 *
 *                          DEVICE
 *
 * ===============================================================*/
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


struct nk_canvas {
    struct nk_command_buffer *painter;
    struct nk_vec2 item_spacing;
    struct nk_vec2 panel_padding;
    struct nk_style_item window_background;
};

static void
canvas_begin(struct nk_context *ctx, struct nk_canvas *canvas, nk_flags flags,
    int x, int y, int width, int height, struct nk_color background_color)
{
    /* save style properties which will be overwritten */
    canvas->panel_padding = ctx->style.window.padding;
    canvas->item_spacing = ctx->style.window.spacing;
    canvas->window_background = ctx->style.window.fixed_background;

    /* use the complete window space and set background */
    ctx->style.window.spacing = nk_vec2(0,0);
    ctx->style.window.padding = nk_vec2(0,0);
    ctx->style.window.fixed_background = nk_style_item_color(background_color);

    /* create/update window and set position + size */
    flags = flags & ~NK_WINDOW_DYNAMIC;
    nk_window_set_bounds(ctx, "Window", nk_rect(x, y, width, height));
    nk_begin(ctx, "Window", nk_rect(x, y, width, height), NK_WINDOW_NO_SCROLLBAR|flags);

    /* allocate the complete window space for drawing */
    {struct nk_rect total_space;
    total_space = nk_window_get_content_region(ctx);
    nk_layout_row_dynamic(ctx, total_space.h, 1);
    nk_widget(&total_space, ctx);
    canvas->painter = nk_window_get_canvas(ctx);}
}

static void
canvas_end(struct nk_context *ctx, struct nk_canvas *canvas)
{
    nk_end(ctx);
    ctx->style.window.spacing = canvas->panel_padding;
    ctx->style.window.padding = canvas->item_spacing;
    ctx->style.window.fixed_background = canvas->window_background;
}

/* ===============================================================
 *
 * HELPER UTILS
 *
 * ===============================================================*/
#define TIMER_INIT(x)   float ltime##x = SDL_GetTicks();  \
                        float ctime##x = SDL_GetTicks();

#define TIMER_GET(x, d) ctime##x = SDL_GetTicks();      \
                        d = ctime##x - ltime##x;        \
                        ltime##x = ctime##x;

/* Unlike TIMER_GET, this does not reset the timer */
#define TIMER_GET_ELAPSED(x, d) ctime##x = SDL_GetTicks();      \
                                d = ctime##x - ltime##x;

#define TIMER_RESET(x)  ctime##x = SDL_GetTicks();      \
                        ltime##x = ctime##x;

#define FPS_INIT(y)     float delaytime##y = 0;

#define FPS_LOCK_MAX(y, d, fps) if ((delaytime##y += (1000/(fps))-d) > 0)   \
                                    if (delaytime##y > 0)                   \
                                        SDL_Delay(delaytime##y);            \


/* updatecnt describes how often the fps should be updated   */
/* 1 -> after 1sec, 2 -> every half sec, 10 -> every 100msec */
#define FPS_COUNT_INIT(z, uc)   float cycles##z = 0;      \
                                unsigned int count##z = 0;       \
                                float updatecnt##z = uc;

/* You gotta call this in your main loop and it returns the current fps */
#define FPS_COUNT_TICK(z, d, fps)   cycles##z += d;                             \
                                    count##z++;                                 \
                                    if (cycles##z > 1000*(1/updatecnt##z)) {    \
                                        cycles##z -= 1000*(1/updatecnt##z);     \
                                        fps = count##z*updatecnt##z;            \
                                        count##z = 0;                           \
                                    }

/* This is used to simulate min "FPS", so instead of lag,   */
/* it runs the game logic "slower"                          */
#define DELTA_MIN_STEP(d, fps)  if (d > 1000/fps)   \
                                    d = 1000/fps;   \


int main(int argc, char *argv[])
{
    SDL_DisplayMode mode;
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        return 1;
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

    SDL_Window *window = SDL_CreateWindow("Demo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_RESIZABLE);
    if (!window)
    {
        SDL_Log("Failed to open window: %s", SDL_GetError());
        return 2;
    }

    struct sdlsurface_context *context = nk_sdlsurface_init(800, 600);
    struct nk_context *ctx = &(context->ctx);

    SDL_Surface* screen = SDL_GetWindowSurface(window);
    SDL_SetSurfaceBlendMode(screen, SDL_BLENDMODE_NONE);

    nk_sdl_font_stash_begin(context);
    context->atlas.default_font = nk_font_atlas_add_default(&context->atlas, 13.0f, 0);
    nk_sdl_font_stash_end(context);

    /* FPS Counter and Limiter */
    float delta = 0;
    unsigned int fps = 0;
    TIMER_INIT(dcnt);
    FPS_INIT(fpscontrol);
    FPS_COUNT_INIT(fpsdisp, 2.0f);

    for(;;)
    {
        /* Input */
        nk_input_begin(ctx);
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                goto quit;
            if ((event.type == SDL_WINDOWEVENT) && (event.window.windowID == SDL_GetWindowID(window)))
            {   /* If we got a window size changed event, we need to recreate the surface! */
                if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                {
                    nk_sdlsurface_update(context, event.window.data1, event.window.data2);
                    screen = SDL_GetWindowSurface(window);
                    SDL_SetSurfaceBlendMode(screen, SDL_BLENDMODE_NONE);
                }
            }
            nk_sdlsurface_handle_event(ctx, &event);
        }
        nk_input_end(ctx);

        /* GUI */
        {struct nk_canvas canvas;
        int w, h;
        SDL_GetWindowSize(window, &w, &h);
        canvas_begin(ctx, &canvas, 0, 0, 0, w, h, nk_rgb(250,250,250));
        {
            nk_fill_rect(canvas.painter, nk_rect(15,15,210,210), 5, nk_rgb(247, 230, 154));
            nk_fill_rect(canvas.painter, nk_rect(20,20,200,200), 5, nk_rgb(188, 174, 118));
            nk_draw_text(canvas.painter, nk_rect(30, 30, 150, 20), "Text to draw", 12, ctx->style.font, nk_rgb(188,174,118), nk_rgb(0,0,0));
            nk_fill_rect(canvas.painter, nk_rect(250,20,100,100), 5, nk_rgb(0,0,255));
            nk_fill_circle(canvas.painter, nk_rect(20,250,90,90), nk_rgb(255,0,0));
            nk_fill_triangle(canvas.painter, 250, 250, 350, 250, 300, 350, nk_rgb(0,255,0));
            nk_fill_arc(canvas.painter, 300, 180, 50, 0, 3.141592654f * 3.0f / 4.0f, nk_rgb(255,255,0));

            {float points[12];
            points[0] = 200; points[1] = 250;
            points[2] = 250; points[3] = 350;
            points[4] = 225; points[5] = 350;
            points[6] = 200; points[7] = 300;
            points[8] = 175; points[9] = 350;
            points[10] = 150; points[11] = 350;
            nk_fill_polygon(canvas.painter, points, 6, nk_rgb(0,0,0));}
            
            static float i = 0;
            static float j = 0;
            nk_stroke_arc(canvas.painter, 200, 425, 50, 1.0f+i, -3.141592654f * 3.0f / 4.0f, 15, nk_rgb(0,255,0));
            nk_stroke_arc(canvas.painter, 200, 425, 30, 1.0f+j, 2.0f, 8, nk_rgb(0,255,0));
            i += 0.002f * delta;
            j -= 0.003f * delta;

            nk_stroke_line(canvas.painter, 15, 10, 200, 10, 2.0f, nk_rgb(189,45,75));
            nk_stroke_rect(canvas.painter, nk_rect(370, 20, 100, 100), 10, 3, nk_rgb(0,0,255));
            nk_stroke_curve(canvas.painter, 380, 200, 405, 270, 455, 120, 480, 200, 2, nk_rgb(0,150,220));
            nk_stroke_triangle(canvas.painter, 370, 250, 470, 250, 420, 350, 6, nk_rgb(255,0,143));
            nk_stroke_triangle(canvas.painter, 500, 350, 600, 350, 550, 250, 6, nk_rgb(255,0,143));
            nk_stroke_triangle(canvas.painter, 450, 450, 550, 450, 300, 370, 6, nk_rgb(255,0,143));  
            nk_stroke_circle(canvas.painter, nk_rect(20, 370, 90, 130), 5, nk_rgb(0,255,120));
            nk_fill_rect_multi_color(canvas.painter, nk_rect(500, 15, 100, 200), nk_rgb(255,0,143), nk_rgb(0,255,120), nk_rgb(189,45,75), nk_rgb(255,255,0));
        }
        canvas_end(ctx, &canvas);}

        /* Draw */
        nk_sdlsurface_render(context, (const struct nk_color){0, 0, 0, 255}, 0);
        SDL_BlitSurface(context->fb, NULL, screen, NULL);
        SDL_UpdateWindowSurface(window);

        /* Get the delta */
        TIMER_GET(dcnt, delta);
        /* Minimal FPS */
        DELTA_MIN_STEP(delta, 30.0f);
        /* Limit FPS, comment this out if you want to go as fast as possible */
        FPS_LOCK_MAX(fpscontrol, delta, 60.0f);
        /* Get current FPS */
        FPS_COUNT_TICK(fpsdisp, delta, fps);
        SDL_Log("delta: %f\t--\tfps: %u\t", delta, fps);
    }


    quit:
    nk_sdlsurface_shutdown(context);
    screen = NULL;
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

