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
#include <SDL_image.h>
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

/* macros */
#define UNUSED(a) (void)a
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) < (b) ? (b) : (a))
#define LEN(a) (sizeof(a)/sizeof(a)[0])

struct media {
    struct nk_font *font_14;
    struct nk_font *font_18;
    struct nk_font *font_20;
    struct nk_font *font_22;

    struct nk_image unchecked;
    struct nk_image checked;
    struct nk_image rocket;
    struct nk_image cloud;
    struct nk_image pen;
    struct nk_image play;
    struct nk_image pause;
    struct nk_image stop;
    struct nk_image prev;
    struct nk_image next;
    struct nk_image tools;
    struct nk_image dir;
    struct nk_image copy;
    struct nk_image convert;
    struct nk_image del;
    struct nk_image edit;
    struct nk_image images[9];
    struct nk_image menu[6];
};

/* ===============================================================
 *
 *                          CUSTOM WIDGET
 *
 * ===============================================================*/
static int
ui_piemenu(struct nk_context *ctx, struct nk_vec2 pos, float radius,
            struct nk_image *icons, int item_count)
{
    int ret = -1;
    struct nk_rect total_space;
    struct nk_rect bounds;
    int active_item = 0;

    /* pie menu popup */
    struct nk_color border = ctx->style.window.border_color;
    struct nk_style_item background = ctx->style.window.fixed_background;
    ctx->style.window.fixed_background = nk_style_item_hide();
    ctx->style.window.border_color = nk_rgba(0,0,0,0);

    total_space  = nk_window_get_content_region(ctx);
    ctx->style.window.spacing = nk_vec2(0,0);
    ctx->style.window.padding = nk_vec2(0,0);

    if (nk_popup_begin(ctx, NK_POPUP_STATIC, "piemenu", NK_WINDOW_NO_SCROLLBAR,
        nk_rect(pos.x - total_space.x - radius, pos.y - radius - total_space.y,
        2*radius,2*radius)))
    {
        int i = 0;
        struct nk_command_buffer* out = nk_window_get_canvas(ctx);
        const struct nk_input *in = &ctx->input;

        total_space = nk_window_get_content_region(ctx);
        ctx->style.window.spacing = nk_vec2(4,4);
        ctx->style.window.padding = nk_vec2(8,8);
        nk_layout_row_dynamic(ctx, total_space.h, 1);
        nk_widget(&bounds, ctx);

        /* outer circle */
        nk_fill_circle(out, bounds, nk_rgb(50,50,50));
        {
            /* circle buttons */
            float step = (2 * 3.141592654f) / (float)(MAX(1,item_count));
            float a_min = 0; float a_max = step;

            struct nk_vec2 center = nk_vec2(bounds.x + bounds.w / 2.0f, bounds.y + bounds.h / 2.0f);
            struct nk_vec2 drag = nk_vec2(in->mouse.pos.x - center.x, in->mouse.pos.y - center.y);
            float angle = (float)atan2(drag.y, drag.x);
            if (angle < -0.0f) angle += 2.0f * 3.141592654f;
            active_item = (int)(angle/step);

            for (i = 0; i < item_count; ++i) {
                struct nk_rect content;
                float rx, ry, dx, dy, a;
                nk_fill_arc(out, center.x, center.y, (bounds.w/2.0f),
                    a_min, a_max, (active_item == i) ? nk_rgb(45,100,255): nk_rgb(60,60,60));

                /* separator line */
                rx = bounds.w/2.0f; ry = 0;
                dx = rx * (float)cos(a_min) - ry * (float)sin(a_min);
                dy = rx * (float)sin(a_min) + ry * (float)cos(a_min);
                nk_stroke_line(out, center.x, center.y,
                    center.x + dx, center.y + dy, 1.0f, nk_rgb(50,50,50));

                /* button content */
                a = a_min + (a_max - a_min)/2.0f;
                rx = bounds.w/2.5f; ry = 0;
                content.w = 30; content.h = 30;
                content.x = center.x + ((rx * (float)cos(a) - ry * (float)sin(a)) - content.w/2.0f);
                content.y = center.y + (rx * (float)sin(a) + ry * (float)cos(a) - content.h/2.0f);
                nk_draw_image(out, content, &icons[i], nk_rgb(255,255,255));
                a_min = a_max; a_max += step;
            }
        }
        {
            /* inner circle */
            struct nk_rect inner;
            inner.x = bounds.x + bounds.w/2 - bounds.w/4;
            inner.y = bounds.y + bounds.h/2 - bounds.h/4;
            inner.w = bounds.w/2; inner.h = bounds.h/2;
            nk_fill_circle(out, inner, nk_rgb(45,45,45));

            /* active icon content */
            bounds.w = inner.w / 2.0f;
            bounds.h = inner.h / 2.0f;
            bounds.x = inner.x + inner.w/2 - bounds.w/2;
            bounds.y = inner.y + inner.h/2 - bounds.h/2;
            nk_draw_image(out, bounds, &icons[active_item], nk_rgb(255,255,255));
        }
        nk_layout_space_end(ctx);
        if (!nk_input_is_mouse_down(&ctx->input, NK_BUTTON_RIGHT)) {
            nk_popup_close(ctx);
            ret = active_item;
        }
    } else ret = -2;
    ctx->style.window.spacing = nk_vec2(4,4);
    ctx->style.window.padding = nk_vec2(8,8);
    nk_popup_end(ctx);

    ctx->style.window.fixed_background = background;
    ctx->style.window.border_color = border;
    return ret;
}

/* ===============================================================
 *
 *                          GRID
 *
 * ===============================================================*/
static void
grid_demo(struct nk_context *ctx, struct media *media)
{
    static char text[3][64];
    static int text_len[3];
    static const char *items[] = {"Item 0","item 1","item 2"};
    static int selected_item = 0;
    static int check = 1;

    int i;
    nk_style_set_font(ctx, &media->font_20->handle);
    if (nk_begin(ctx, "Grid Demo", nk_rect(600, 350, 275, 250),
        NK_WINDOW_TITLE|NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|
        NK_WINDOW_NO_SCROLLBAR))
    {
        nk_style_set_font(ctx, &media->font_18->handle);
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
    nk_style_set_font(ctx, &media->font_14->handle);
}

/* ===============================================================
 *
 *                          BUTTON DEMO
 *
 * ===============================================================*/
static void
ui_header(struct nk_context *ctx, struct media *media, const char *title)
{
    nk_style_set_font(ctx, &media->font_18->handle);
    nk_layout_row_dynamic(ctx, 20, 1);
    nk_label(ctx, title, NK_TEXT_LEFT);
}

static void
ui_widget(struct nk_context *ctx, struct media *media, float height)
{
    static const float ratio[] = {0.15f, 0.85f};
    nk_style_set_font(ctx, &media->font_22->handle);
    nk_layout_row(ctx, NK_DYNAMIC, height, 2, ratio);
    nk_spacing(ctx, 1);
}

static void
ui_widget_centered(struct nk_context *ctx, struct media *media, float height)
{
    static const float ratio[] = {0.15f, 0.50f, 0.35f};
    nk_style_set_font(ctx, &media->font_22->handle);
    nk_layout_row(ctx, NK_DYNAMIC, height, 3, ratio);
    nk_spacing(ctx, 1);
}

static void
button_demo(struct nk_context *ctx, struct media *media)
{
    static int option = 1;
    static int toggle0 = 1;
    static int toggle1 = 0;
    static int toggle2 = 1;

    nk_style_set_font(ctx, &media->font_20->handle);
    nk_begin(ctx, "Button Demo", nk_rect(50,50,255,610),
        NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_TITLE);

    /*------------------------------------------------
     *                  MENU
     *------------------------------------------------*/
    nk_menubar_begin(ctx);
    {
        /* toolbar */
        nk_layout_row_static(ctx, 40, 40, 4);
        if (nk_menu_begin_image(ctx, "Music", media->play, nk_vec2(110,120)))
        {
            /* settings */
            nk_layout_row_dynamic(ctx, 25, 1);
            nk_menu_item_image_label(ctx, media->play, "Play", NK_TEXT_RIGHT);
            nk_menu_item_image_label(ctx, media->stop, "Stop", NK_TEXT_RIGHT);
            nk_menu_item_image_label(ctx, media->pause, "Pause", NK_TEXT_RIGHT);
            nk_menu_item_image_label(ctx, media->next, "Next", NK_TEXT_RIGHT);
            nk_menu_item_image_label(ctx, media->prev, "Prev", NK_TEXT_RIGHT);
            nk_menu_end(ctx);
        }
        nk_button_image(ctx, media->tools);
        nk_button_image(ctx, media->cloud);
        nk_button_image(ctx, media->pen);
    }
    nk_menubar_end(ctx);

    /*------------------------------------------------
     *                  BUTTON
     *------------------------------------------------*/
    ui_header(ctx, media, "Push buttons");
    ui_widget(ctx, media, 35);
    if (nk_button_label(ctx, "Push me"))
        fprintf(stdout, "pushed!\n");
    ui_widget(ctx, media, 35);
    if (nk_button_image_label(ctx, media->rocket, "Styled", NK_TEXT_CENTERED))
        fprintf(stdout, "rocket!\n");

    /*------------------------------------------------
     *                  REPEATER
     *------------------------------------------------*/
    ui_header(ctx, media, "Repeater");
    ui_widget(ctx, media, 35);
    if (nk_button_label(ctx, "Press me"))
        fprintf(stdout, "pressed!\n");

    /*------------------------------------------------
     *                  TOGGLE
     *------------------------------------------------*/
    ui_header(ctx, media, "Toggle buttons");
    ui_widget(ctx, media, 35);
    if (nk_button_image_label(ctx, (toggle0) ? media->checked: media->unchecked, "Toggle", NK_TEXT_LEFT))
        toggle0 = !toggle0;

    ui_widget(ctx, media, 35);
    if (nk_button_image_label(ctx, (toggle1) ? media->checked: media->unchecked, "Toggle", NK_TEXT_LEFT))
        toggle1 = !toggle1;

    ui_widget(ctx, media, 35);
    if (nk_button_image_label(ctx, (toggle2) ? media->checked: media->unchecked, "Toggle", NK_TEXT_LEFT))
        toggle2 = !toggle2;

    /*------------------------------------------------
     *                  RADIO
     *------------------------------------------------*/
    ui_header(ctx, media, "Radio buttons");
    ui_widget(ctx, media, 35);
    if (nk_button_symbol_label(ctx, (option == 0)?NK_SYMBOL_CIRCLE_OUTLINE:NK_SYMBOL_CIRCLE_SOLID, "Select", NK_TEXT_LEFT))
        option = 0;
    ui_widget(ctx, media, 35);
    if (nk_button_symbol_label(ctx, (option == 1)?NK_SYMBOL_CIRCLE_OUTLINE:NK_SYMBOL_CIRCLE_SOLID, "Select", NK_TEXT_LEFT))
        option = 1;
    ui_widget(ctx, media, 35);
    if (nk_button_symbol_label(ctx, (option == 2)?NK_SYMBOL_CIRCLE_OUTLINE:NK_SYMBOL_CIRCLE_SOLID, "Select", NK_TEXT_LEFT))
        option = 2;

    /*------------------------------------------------
     *                  CONTEXTUAL
     *------------------------------------------------*/
    nk_style_set_font(ctx, &media->font_18->handle);
    if (nk_contextual_begin(ctx, NK_WINDOW_NO_SCROLLBAR, nk_vec2(150, 300), nk_window_get_bounds(ctx))) {
        nk_layout_row_dynamic(ctx, 30, 1);
        if (nk_contextual_item_image_label(ctx, media->copy, "Clone", NK_TEXT_RIGHT))
            fprintf(stdout, "pressed clone!\n");
        if (nk_contextual_item_image_label(ctx, media->del, "Delete", NK_TEXT_RIGHT))
            fprintf(stdout, "pressed delete!\n");
        if (nk_contextual_item_image_label(ctx, media->convert, "Convert", NK_TEXT_RIGHT))
            fprintf(stdout, "pressed convert!\n");
        if (nk_contextual_item_image_label(ctx, media->edit, "Edit", NK_TEXT_RIGHT))
            fprintf(stdout, "pressed edit!\n");
        nk_contextual_end(ctx);
    }
    nk_style_set_font(ctx, &media->font_14->handle);
    nk_end(ctx);
}

/* ===============================================================
 *
 *                          BASIC DEMO
 *
 * ===============================================================*/
static void
basic_demo(struct nk_context *ctx, struct media *media)
{
    static int image_active;
    static int check0 = 1;
    static int check1 = 0;
    static size_t prog = 80;
    static int selected_item = 0;
    static int selected_image = 3;
    static int selected_icon = 0;
    static const char *items[] = {"Item 0","item 1","item 2"};
    static int piemenu_active = 0;
    static struct nk_vec2 piemenu_pos;

    int i = 0;
    nk_style_set_font(ctx, &media->font_20->handle);
    nk_begin(ctx, "Basic Demo", nk_rect(320, 50, 275, 610),
        NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_TITLE);

    /*------------------------------------------------
     *                  POPUP BUTTON
     *------------------------------------------------*/
    ui_header(ctx, media, "Popup & Scrollbar & Images");
    ui_widget(ctx, media, 35);
    if (nk_button_image_label(ctx, media->dir, "Images", NK_TEXT_CENTERED))
        image_active = !image_active;

    /*------------------------------------------------
     *                  SELECTED IMAGE
     *------------------------------------------------*/
    ui_header(ctx, media, "Selected Image");
    ui_widget_centered(ctx, media, 100);
    nk_image(ctx, media->images[selected_image]);

    /*------------------------------------------------
     *                  IMAGE POPUP
     *------------------------------------------------*/
    if (image_active) {
        if (nk_popup_begin(ctx, NK_POPUP_STATIC, "Image Popup", 0, nk_rect(265, 0, 320, 220))) {
            nk_layout_row_static(ctx, 82, 82, 3);
            for (i = 0; i < 9; ++i) {
                if (nk_button_image(ctx, media->images[i])) {
                    selected_image = i;
                    image_active = 0;
                    nk_popup_close(ctx);
                }
            }
            nk_popup_end(ctx);
        }
    }
    /*------------------------------------------------
     *                  COMBOBOX
     *------------------------------------------------*/
    ui_header(ctx, media, "Combo box");
    ui_widget(ctx, media, 40);
    if (nk_combo_begin_label(ctx, items[selected_item], nk_vec2(nk_widget_width(ctx), 200))) {
        nk_layout_row_dynamic(ctx, 35, 1);
        for (i = 0; i < 3; ++i)
            if (nk_combo_item_label(ctx, items[i], NK_TEXT_LEFT))
                selected_item = i;
        nk_combo_end(ctx);
    }

    ui_widget(ctx, media, 40);
    if (nk_combo_begin_image_label(ctx, items[selected_icon], media->images[selected_icon], nk_vec2(nk_widget_width(ctx), 200))) {
        nk_layout_row_dynamic(ctx, 35, 1);
        for (i = 0; i < 3; ++i)
            if (nk_combo_item_image_label(ctx, media->images[i], items[i], NK_TEXT_RIGHT))
                selected_icon = i;
        nk_combo_end(ctx);
    }

    /*------------------------------------------------
     *                  CHECKBOX
     *------------------------------------------------*/
    ui_header(ctx, media, "Checkbox");
    ui_widget(ctx, media, 30);
    nk_checkbox_label(ctx, "Flag 1", &check0);
    ui_widget(ctx, media, 30);
    nk_checkbox_label(ctx, "Flag 2", &check1);

    /*------------------------------------------------
     *                  PROGRESSBAR
     *------------------------------------------------*/
    ui_header(ctx, media, "Progressbar");
    ui_widget(ctx, media, 35);
    nk_progress(ctx, &prog, 100, nk_true);

    /*------------------------------------------------
     *                  PIEMENU
     *------------------------------------------------*/
    if (nk_input_is_mouse_click_down_in_rect(&ctx->input, NK_BUTTON_RIGHT,
        nk_window_get_bounds(ctx),nk_true)){
        piemenu_pos = ctx->input.mouse.pos;
        piemenu_active = 1;
    }

    if (piemenu_active) {
        int ret = ui_piemenu(ctx, piemenu_pos, 140, &media->menu[0], 6);
        if (ret == -2) piemenu_active = 0;
        if (ret != -1) {
            fprintf(stdout, "piemenu selected: %d\n", ret);
            piemenu_active = 0;
        }
    }
    nk_style_set_font(ctx, &media->font_14->handle);
    nk_end(ctx);
}

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


static struct nk_image
icon_load(const char *filename)
{
    SDL_Surface* optimizedSurface = NULL;

    SDL_Surface* loadedSurface = IMG_Load(filename);
    if (!loadedSurface)
    {
        die("Unable to load image %s! SDL_image Error: %s\n", filename, IMG_GetError());
    }
    else
    {
        optimizedSurface = nk_sdlsurface_convert_surface(loadedSurface);
        if(!optimizedSurface)
        {
            die("Unable to optimize image %s! SDL Error: %s\n", filename, SDL_GetError());
        }
        SDL_FreeSurface(loadedSurface);
    }

    struct nk_image nk_sdl_image = nk_image_ptr(optimizedSurface);
    nk_sdl_image.w = optimizedSurface->w;
    nk_sdl_image.h = optimizedSurface->h;
    nk_sdl_image.region[0] = 0;
    nk_sdl_image.region[1] = 0;
    nk_sdl_image.region[2] = optimizedSurface->w;
    nk_sdl_image.region[3] = optimizedSurface->h;
    return nk_sdl_image;
}

int main(int argc, char *argv[])
{
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

    SDL_Surface* screen = SDL_GetWindowSurface(window);
    SDL_SetSurfaceBlendMode(screen, SDL_BLENDMODE_NONE);

    struct sdlsurface_context *context = nk_sdlsurface_init(800, 600);
    struct nk_context *ctx = &(context->ctx);

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG))
    {
        SDL_Log("Failed to initialise SDL_Image: %s", IMG_GetError());
        return 3;
    }

    /* icons */
    struct media media = {0};

    int w, h;
    struct nk_font_config cfg = nk_font_config(0);
    cfg.oversample_h = 3; cfg.oversample_v = 2;
    /* Loading one font with different heights is only required if you want higher
     * quality text otherwise you can just set the font height directly
     * e.g.: ctx->style.font.height = 20. */
    nk_sdl_font_stash_begin(context);
    media.font_14 = nk_font_atlas_add_from_file(&(context->atlas), "../../extra_font/Roboto-Regular.ttf", 14.0f, &cfg);
    media.font_18 = nk_font_atlas_add_from_file(&(context->atlas), "../../extra_font/Roboto-Regular.ttf", 18.0f, &cfg);
    media.font_20 = nk_font_atlas_add_from_file(&(context->atlas), "../../extra_font/Roboto-Regular.ttf", 20.0f, &cfg);
    media.font_22 = nk_font_atlas_add_from_file(&(context->atlas), "../../extra_font/Roboto-Regular.ttf", 22.0f, &cfg);
    nk_sdl_font_stash_end(context);

    media.unchecked = icon_load("../../example/icon/unchecked.png");
    media.checked = icon_load("../../example/icon/checked.png");
    media.rocket = icon_load("../../example/icon/rocket.png");
    media.cloud = icon_load("../../example/icon/cloud.png");
    media.pen = icon_load("../../example/icon/pen.png");
    media.play = icon_load("../../example/icon/play.png");
    media.pause = icon_load("../../example/icon/pause.png");
    media.stop = icon_load("../../example/icon/stop.png");
    media.next =  icon_load("../../example/icon/next.png");
    media.prev =  icon_load("../../example/icon/prev.png");
    media.tools = icon_load("../../example/icon/tools.png");
    media.dir = icon_load("../../example/icon/directory.png");
    media.copy = icon_load("../../example/icon/copy.png");
    media.convert = icon_load("../../example/icon/export.png");
    media.del = icon_load("../../example/icon/delete.png");
    media.edit = icon_load("../../example/icon/edit.png");
    media.menu[0] = icon_load("../../example/icon/home.png");
    media.menu[1] = icon_load("../../example/icon/phone.png");
    media.menu[2] = icon_load("../../example/icon/plane.png");
    media.menu[3] = icon_load("../../example/icon/wifi.png");
    media.menu[4] = icon_load("../../example/icon/settings.png");
    media.menu[5] = icon_load("../../example/icon/volume.png");

    {int i;
    for (i = 0; i < 9; ++i) {
        char buffer[256];
        sprintf(buffer, "../../example/images/image%d.png", (i+1));
        media.images[i] = icon_load(buffer);
    }}

    /* FPS Counter and Limiter */
    float delta = 0;
    unsigned int fps = 0;
    TIMER_INIT(dcnt);
    FPS_INIT(fpscontrol);
    FPS_COUNT_INIT(fpsdisp, 2.0f);

    for (;;)
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
        basic_demo(ctx, &media);
        button_demo(ctx, &media);
        grid_demo(ctx, &media);

        /* Draw */
        nk_sdlsurface_render(context, (const struct nk_color){77, 77, 77, 255}, 1);
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
    SDL_FreeSurface((SDL_Surface *)media.unchecked.handle.ptr);
    SDL_FreeSurface((SDL_Surface *)media.checked.handle.ptr);
    SDL_FreeSurface((SDL_Surface *)media.rocket.handle.ptr);
    SDL_FreeSurface((SDL_Surface *)media.cloud.handle.ptr);
    SDL_FreeSurface((SDL_Surface *)media.pen.handle.ptr);
    SDL_FreeSurface((SDL_Surface *)media.play.handle.ptr);
    SDL_FreeSurface((SDL_Surface *)media.pause.handle.ptr);
    SDL_FreeSurface((SDL_Surface *)media.stop.handle.ptr);
    SDL_FreeSurface((SDL_Surface *)media.next.handle.ptr);
    SDL_FreeSurface((SDL_Surface *)media.prev.handle.ptr);
    SDL_FreeSurface((SDL_Surface *)media.tools.handle.ptr);
    SDL_FreeSurface((SDL_Surface *)media.dir.handle.ptr);
    SDL_FreeSurface((SDL_Surface *)media.del.handle.ptr);

    nk_sdlsurface_shutdown(context);
    screen = NULL;
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

