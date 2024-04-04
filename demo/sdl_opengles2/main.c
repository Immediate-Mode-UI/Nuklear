/* nuklear - 1.40.8 - public domain */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <limits.h>
#include <time.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_SDL_GLES2_IMPLEMENTATION
#include "../../nuklear.h"
#include "nuklear_sdl_gles2.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../../demo/common/filebrowser/stb_image.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define MAX_VERTEX_MEMORY 512 * 1024
#define MAX_ELEMENT_MEMORY 128 * 1024

#define UNUSED(a) (void)a
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) < (b) ? (b) : (a))
#define LEN(a) (sizeof(a)/sizeof(a)[0])

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

/* ===============================================================
 *
 *                          DEMO
 *
 * ===============================================================*/


/* Platform */
SDL_Window *win;
int running = nk_true;
#ifdef INCLUDE_FILE_BROWSER
struct file_browser browser;
struct media media;
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
    int x,y,n;
    GLuint tex;
    unsigned char *data = stbi_load(filename, &x, &y, &n, 0);
    if (!data) die("[SDL]: failed to load image: %s", filename);

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
    return nk_image_id((int)tex);
}

static void
MainLoop(void* loopArg){
    struct nk_context *ctx = (struct nk_context *)loopArg;

    /* Input */
    SDL_Event evt;
    nk_input_begin(ctx);
    while (SDL_PollEvent(&evt)) {
        if (evt.type == SDL_QUIT) running = nk_false;
        nk_sdl_handle_event(&evt);
    }
    nk_sdl_handle_grab(); /* optional grabbing behavior */
    nk_input_end(ctx);


    /* GUI */
    if (nk_begin(ctx, "Demo", nk_rect(50, 50, 200, 200),
        NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
        NK_WINDOW_CLOSABLE|NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE))
    {
        nk_menubar_begin(ctx);
        nk_layout_row_begin(ctx, NK_STATIC, 25, 2);
        nk_layout_row_push(ctx, 45);
        if (nk_menu_begin_label(ctx, "FILE", NK_TEXT_LEFT, nk_vec2(120, 200))) {
            nk_layout_row_dynamic(ctx, 30, 1);
            nk_menu_item_label(ctx, "OPEN", NK_TEXT_LEFT);
            nk_menu_item_label(ctx, "CLOSE", NK_TEXT_LEFT);
            nk_menu_end(ctx);
        }
        nk_layout_row_push(ctx, 45);
        if (nk_menu_begin_label(ctx, "EDIT", NK_TEXT_LEFT, nk_vec2(120, 200))) {
            nk_layout_row_dynamic(ctx, 30, 1);
            nk_menu_item_label(ctx, "COPY", NK_TEXT_LEFT);
            nk_menu_item_label(ctx, "CUT", NK_TEXT_LEFT);
            nk_menu_item_label(ctx, "PASTE", NK_TEXT_LEFT);
            nk_menu_end(ctx);
        }
        nk_layout_row_end(ctx);
        nk_menubar_end(ctx);

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

    /* Draw */
    {float bg[4];
    int win_width, win_height;
    nk_color_fv(bg, nk_rgb(28,48,62));
    SDL_GetWindowSize(win, &win_width, &win_height);
    glViewport(0, 0, win_width, win_height);
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(bg[0], bg[1], bg[2], bg[3]);
    /* IMPORTANT: `nk_sdl_render` modifies some global OpenGL state
     * with blending, scissor, face culling, depth test and viewport and
     * defaults everything back into a default state.
     * Make sure to either a.) save and restore or b.) reset your own state after
     * rendering the UI. */
    nk_sdl_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY);
    SDL_GL_SwapWindow(win);}
}

int main(int argc, char* argv[])
{
    /* GUI */
    struct nk_context *ctx;
    SDL_GLContext glContext;

    NK_UNUSED(argc);
    NK_UNUSED(argv);

    /* SDL setup */
    SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0");
    /*SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER|SDL_INIT_EVENTS); // - do NOT init SDL on GL ES 2 */
    SDL_GL_SetAttribute (SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute (SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    win = SDL_CreateWindow("Demo",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL|SDL_WINDOW_SHOWN|SDL_WINDOW_ALLOW_HIGHDPI);
    glContext = SDL_GL_CreateContext(win);

    /* OpenGL setup */
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    ctx = nk_sdl_init(win);
    /* Load Fonts: if none of these are loaded a default font will be used  */
    /* Load Cursor: if you uncomment cursor loading please hide the cursor */
    {struct nk_font_atlas *atlas;
    nk_sdl_font_stash_begin(&atlas);
    /*struct nk_font *droid = nk_font_atlas_add_from_file(atlas, "../../../extra_font/DroidSans.ttf", 14, 0);*/
    /*struct nk_font *roboto = nk_font_atlas_add_from_file(atlas, "../../../extra_font/Roboto-Regular.ttf", 16, 0);*/
    /*struct nk_font *future = nk_font_atlas_add_from_file(atlas, "../../../extra_font/kenvector_future_thin.ttf", 13, 0);*/
    /*struct nk_font *clean = nk_font_atlas_add_from_file(atlas, "../../../extra_font/ProggyClean.ttf", 12, 0);*/
    /*struct nk_font *tiny = nk_font_atlas_add_from_file(atlas, "../../../extra_font/ProggyTiny.ttf", 10, 0);*/
    /*struct nk_font *cousine = nk_font_atlas_add_from_file(atlas, "../../../extra_font/Cousine-Regular.ttf", 13, 0);*/
    nk_sdl_font_stash_end();
    /*nk_style_load_all_cursors(ctx, atlas->cursors);*/
    /*nk_style_set_font(ctx, &roboto->handle)*/;}

    /* style.c */
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

    #ifdef INCLUDE_FILE_BROWSER
    /* icons */
    glEnable(GL_TEXTURE_2D);
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

#if defined(__EMSCRIPTEN__)
    #include <emscripten.h>
    emscripten_set_main_loop_arg(MainLoop, (void*)ctx, 0, nk_true);
#else
    while (running) MainLoop((void*)ctx);
#endif

    #ifdef INCLUDE_FILE_BROWSER
    glDeleteTextures(1,(const GLuint*)&media.icons.home.handle.id);
    glDeleteTextures(1,(const GLuint*)&media.icons.directory.handle.id);
    glDeleteTextures(1,(const GLuint*)&media.icons.computer.handle.id);
    glDeleteTextures(1,(const GLuint*)&media.icons.desktop.handle.id);
    glDeleteTextures(1,(const GLuint*)&media.icons.default_file.handle.id);
    glDeleteTextures(1,(const GLuint*)&media.icons.text_file.handle.id);
    glDeleteTextures(1,(const GLuint*)&media.icons.music_file.handle.id);
    glDeleteTextures(1,(const GLuint*)&media.icons.font_file.handle.id);
    glDeleteTextures(1,(const GLuint*)&media.icons.img_file.handle.id);
    glDeleteTextures(1,(const GLuint*)&media.icons.movie_file.handle.id);

    file_browser_free(&browser);
    #endif

    nk_sdl_shutdown();
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}

