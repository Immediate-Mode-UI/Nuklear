#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <limits.h>
#include <time.h>
#include <stdbool.h>
#include <glad/include/glad.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_SDL3_GL3_IMPLEMENTATION
#include <nuklear.h>

#include "nuklear_sdl3_ogl3.h"

#define MAX_VERTEX_MEMORY  512 * 1024
#define MAX_ELEMENT_MEMORY 128 * 1024

enum { EASY = 0, NORMAL = 1, HARD = 2 };

SDL_Window* win;
SDL_GLContext glContext;
struct nk_context* ctx;
int win_width = 800, win_height = 600;
bool is_running = true;
struct nk_colorf bg;
int difficulty = 0;

#ifdef INCLUDE_CONFIGURATOR
static struct nk_color color_table[NK_COLOR_COUNT];
#endif

void main_loop(void) {
    SDL_Event evt;
    nk_input_begin(ctx);
    while (SDL_PollEvent(&evt)) {
        if (evt.type == SDL_EVENT_QUIT) {
            is_running = false;
#ifdef __EMSCRIPTEN__
            emscripten_cancel_main_loop();
#endif
        }
        nk_sdl_handle_event(&evt);
    }
    nk_sdl_handle_grab();
    nk_input_end(ctx);

    SDL_GetWindowSize(win, &win_width, &win_height);

    static bool show_settings = false;

    if (nk_begin(ctx, "Main Menu",
                 nk_rect(0, 0, (float) win_width, (float) win_height),
                 NK_WINDOW_BORDER)) {
        if (!show_settings) {
            nk_layout_row_dynamic(ctx, 60, 1);
            nk_label(ctx, "My Awesome Game", NK_TEXT_CENTERED);

            nk_layout_row_dynamic(ctx, 50, 1);
            if (nk_button_label(ctx, "Start Game")) {
                printf("Start Game clicked\n");
            }
            if (nk_button_label(ctx, "Settings")) {
                show_settings = true;
            }
            if (nk_button_label(ctx, "Exit")) {
                printf("Exit clicked\n");
            }
        } else {
            nk_layout_row_dynamic(ctx, 40, 1);
            if (nk_button_label(ctx, "Back to Menu")) {
                show_settings = false;
            }

            nk_layout_row_dynamic(ctx, 30, 1);
            nk_label(ctx, "Settings", NK_TEXT_CENTERED);

            static char name[64];
            static int name_len = 0;
            nk_label(ctx, "Player Name:", NK_TEXT_LEFT);
            nk_edit_string(ctx, NK_EDIT_DEFAULT, name, &name_len,
                           sizeof(name), nk_filter_default);

            nk_label(ctx, "Difficulty:", NK_TEXT_LEFT);
            if (nk_option_label(ctx, "Easy", difficulty == EASY))
                difficulty = EASY;
            if (nk_option_label(ctx, "Normal", difficulty == NORMAL))
                difficulty = NORMAL;
            if (nk_option_label(ctx, "Hard", difficulty == HARD))
                difficulty = HARD;

            static float volume = 0.5f;
            nk_label(ctx, "Master Volume:", NK_TEXT_LEFT);
            nk_slider_float(ctx, 0.0f, &volume, 1.0f, 0.01f);

            const char* resolutions[] = {"800x600", "1280x720", "1920x1080"};
            static int res_index = 0;
            nk_label(ctx, "Resolution:", NK_TEXT_LEFT);
            res_index = nk_combo(ctx, resolutions,
                                 NK_LEN(resolutions),
                                 res_index, 25,
                                 nk_vec2(200, 200));

            if (res_index == 0) {
                win_width = 800; win_height = 600;
            } else if (res_index == 1) {
                win_width = 1280; win_height = 720;
            } else if (res_index == 2) {
                win_width = 1920; win_height = 1080;
            }
            SDL_SetWindowSize(win, win_width, win_height);

            static nk_bool fullscreen = nk_false;
            nk_checkbox_label(ctx, "Fullscreen", &fullscreen);

            nk_label(ctx, "Background Color:", NK_TEXT_LEFT);
            bg = nk_color_picker(ctx, bg, NK_RGB);

            if (nk_button_label(ctx, "Apply and Save Settings")) {
                printf("Settings applied: Name=%s, Difficulty=%d, Volume=%.2f, "
                       "Resolution=%s, Fullscreen=%s\n",
                       name, difficulty, volume,
                       resolutions[res_index],
                       fullscreen ? "Yes" : "No");
                show_settings = false;
            }
        }
    }
    nk_end(ctx);

    glViewport(0, 0, win_width, win_height);
    glClearColor(bg.r, bg.g, bg.b, bg.a);
    glClear(GL_COLOR_BUFFER_BIT);
    nk_sdl_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY);
    SDL_GL_SwapWindow(win);
}

int main(int argc, char* argv[]) {
#ifdef INCLUDE_CONFIGURATOR
    memcpy(color_table, nk_default_color_style, sizeof(color_table));
#endif

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        fprintf(stderr, "Failed to init SDL3: %s\n", SDL_GetError());
        return -1;
    }

#if defined(SDL_PLATFORM_IOS) || defined(SDL_PLATFORM_ANDROID) || defined(SDL_PLATFORM_EMSCRIPTEN)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#elif defined(SDL_PLATFORM_WINDOWS) || defined(SDL_PLATFORM_LINUX) || defined(SDL_PLATFORM_MACOS)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
#endif

    win = SDL_CreateWindow("Nuklear + SDL3 Demo",
                           win_width, win_height,
                           SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    SDL_StartTextInput(win);

    glContext = SDL_GL_CreateContext(win);
    SDL_GetWindowSize(win, &win_width, &win_height);

    int major, minor;
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major);
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor);
    printf("OpenGL context version: %d.%d\n", major, minor);

    if (!gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress)) {
        fprintf(stderr, "Failed to setup GLAD\n");
        exit(1);
    }

    glViewport(0, 0, win_width, win_height);

    ctx = nk_sdl_init(win);

    struct nk_font_atlas* atlas;
    struct nk_font* font;

    nk_sdl_font_stash_begin(&atlas);
    font = nk_font_atlas_add_default(atlas, 14, 0);
    nk_sdl_font_stash_end();
    if (font) ctx->style.font = &font->handle;

    bg.r = 0.10f; bg.g = 0.18f; bg.b = 0.24f; bg.a = 1.0f;

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(main_loop, 0, 1);
#else
    while (is_running) {
        main_loop();
    }
#endif

    SDL_StopTextInput(win);
    nk_sdl_shutdown();
    SDL_GL_DestroyContext(glContext);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}