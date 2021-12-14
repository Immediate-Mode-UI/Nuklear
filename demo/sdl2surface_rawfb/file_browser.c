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
#include <unistd.h>
#include <dirent.h>

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
/* ===============================================================
 *
 *                          GUI
 *
 * ===============================================================*/
struct icons {
    struct nk_image desktop;
    struct nk_image home;
    struct nk_image computer;
    struct nk_image directory;

    struct nk_image default_file;
    struct nk_image text_file;
    struct nk_image music_file;
    struct nk_image font_file;
    struct nk_image img_file;
    struct nk_image movie_file;
};

enum file_groups {
    FILE_GROUP_DEFAULT,
    FILE_GROUP_TEXT,
    FILE_GROUP_MUSIC,
    FILE_GROUP_FONT,
    FILE_GROUP_IMAGE,
    FILE_GROUP_MOVIE,
    FILE_GROUP_MAX
};

enum file_types {
    FILE_DEFAULT,
    FILE_TEXT,
    FILE_C_SOURCE,
    FILE_CPP_SOURCE,
    FILE_HEADER,
    FILE_CPP_HEADER,
    FILE_MP3,
    FILE_WAV,
    FILE_OGG,
    FILE_TTF,
    FILE_BMP,
    FILE_PNG,
    FILE_JPEG,
    FILE_PCX,
    FILE_TGA,
    FILE_GIF,
    FILE_MAX
};

struct file_group {
    enum file_groups group;
    const char *name;
    struct nk_image *icon;
};

struct file {
    enum file_types type;
    const char *suffix;
    enum file_groups group;
};

struct media {
    int font;
    int icon_sheet;
    struct icons icons;
    struct file_group group[FILE_GROUP_MAX];
    struct file files[FILE_MAX];
};

#define MAX_PATH_LEN 512
struct file_browser {
    /* path */
    char file[MAX_PATH_LEN];
    char home[MAX_PATH_LEN];
    char desktop[MAX_PATH_LEN];
    char directory[MAX_PATH_LEN];

    /* directory content */
    char **files;
    char **directories;
    size_t file_count;
    size_t dir_count;
    struct media *media;
};

#ifdef __unix__
#include <dirent.h>
#include <unistd.h>
#endif

#ifndef _WIN32
# include <pwd.h>
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

static char*
file_load(const char* path, size_t* siz)
{
    char *buf;
    FILE *fd = fopen(path, "rb");
    if (!fd) die("Failed to open file: %s\n", path);
    fseek(fd, 0, SEEK_END);
    *siz = (size_t)ftell(fd);
    fseek(fd, 0, SEEK_SET);
    buf = (char*)calloc(*siz, 1);
    fread(buf, *siz, 1, fd);
    fclose(fd);
    return buf;
}

static char*
str_duplicate(const char *src)
{
    char *ret;
    size_t len = strlen(src);
    if (!len) return 0;
    ret = (char*)malloc(len+1);
    if (!ret) return 0;
    memcpy(ret, src, len);
    ret[len] = '\0';
    return ret;
}

static void
dir_free_list(char **list, size_t size)
{
    size_t i;
    for (i = 0; i < size; ++i)
        free(list[i]);
    free(list);
}

static char**
dir_list(const char *dir, int return_subdirs, size_t *count)
{
    size_t n = 0;
    char buffer[MAX_PATH_LEN];
    char **results = NULL;
    const DIR *none = NULL;
    size_t capacity = 32;
    size_t size;
    DIR *z;

    assert(dir);
    assert(count);
    strncpy(buffer, dir, MAX_PATH_LEN);
    n = strlen(buffer);

    if (n > 0 && (buffer[n-1] != '/'))
        buffer[n++] = '/';

    size = 0;

    z = opendir(dir);
    if (z != none) {
        int nonempty = 1;
        struct dirent *data = readdir(z);
        nonempty = (data != NULL);
        if (!nonempty) return NULL;

        do {
            DIR *y;
            char *p;
            int is_subdir;
            if (data->d_name[0] == '.')
                continue;

            strncpy(buffer + n, data->d_name, MAX_PATH_LEN-n);
            y = opendir(buffer);
            is_subdir = (y != NULL);
            if (y != NULL) closedir(y);

            if ((return_subdirs && is_subdir) || (!is_subdir && !return_subdirs)){
                if (!size) {
                    results = (char**)calloc(sizeof(char*), capacity);
                } else if (size >= capacity) {
                    void *old = results;
                    capacity = capacity * 2;
                    results = (char**)realloc(results, capacity * sizeof(char*));
                    assert(results);
                    if (!results) free(old);
                }
                p = str_duplicate(data->d_name);
                results[size++] = p;
            }
        } while ((data = readdir(z)) != NULL);
    }

    if (z) closedir(z);
    *count = size;
    return results;
}

static struct file_group
FILE_GROUP(enum file_groups group, const char *name, struct nk_image *icon)
{
    struct file_group fg;
    fg.group = group;
    fg.name = name;
    fg.icon = icon;
    return fg;
}

static struct file
FILE_DEF(enum file_types type, const char *suffix, enum file_groups group)
{
    struct file fd;
    fd.type = type;
    fd.suffix = suffix;
    fd.group = group;
    return fd;
}

static struct nk_image*
media_icon_for_file(struct media *media, const char *file)
{
    int i = 0;
    const char *s = file;
    char suffix[4];
    int found = 0;
    memset(suffix, 0, sizeof(suffix));

    /* extract suffix .xxx from file */
    while (*s++ != '\0') {
        if (found && i < 3)
            suffix[i++] = *s;

        if (*s == '.') {
            if (found){
                found = 0;
                break;
            }
            found = 1;
        }
    }

    /* check for all file definition of all groups for fitting suffix*/
    for (i = 0; i < FILE_MAX && found; ++i) {
        struct file *d = &media->files[i];
        {
            const char *f = d->suffix;
            s = suffix;
            while (f && *f && *s && *s == *f) {
                s++; f++;
            }

            /* found correct file definition so */
            if (f && *s == '\0' && *f == '\0')
                return media->group[d->group].icon;
        }
    }
    return &media->icons.default_file;
}

static void
media_init(struct media *media)
{
    /* file groups */
    struct icons *icons = &media->icons;
    media->group[FILE_GROUP_DEFAULT] = FILE_GROUP(FILE_GROUP_DEFAULT,"default",&icons->default_file);
    media->group[FILE_GROUP_TEXT] = FILE_GROUP(FILE_GROUP_TEXT, "textual", &icons->text_file);
    media->group[FILE_GROUP_MUSIC] = FILE_GROUP(FILE_GROUP_MUSIC, "music", &icons->music_file);
    media->group[FILE_GROUP_FONT] = FILE_GROUP(FILE_GROUP_FONT, "font", &icons->font_file);
    media->group[FILE_GROUP_IMAGE] = FILE_GROUP(FILE_GROUP_IMAGE, "image", &icons->img_file);
    media->group[FILE_GROUP_MOVIE] = FILE_GROUP(FILE_GROUP_MOVIE, "movie", &icons->movie_file);

    /* files */
    media->files[FILE_DEFAULT] = FILE_DEF(FILE_DEFAULT, NULL, FILE_GROUP_DEFAULT);
    media->files[FILE_TEXT] = FILE_DEF(FILE_TEXT, "txt", FILE_GROUP_TEXT);
    media->files[FILE_C_SOURCE] = FILE_DEF(FILE_C_SOURCE, "c", FILE_GROUP_TEXT);
    media->files[FILE_CPP_SOURCE] = FILE_DEF(FILE_CPP_SOURCE, "cpp", FILE_GROUP_TEXT);
    media->files[FILE_HEADER] = FILE_DEF(FILE_HEADER, "h", FILE_GROUP_TEXT);
    media->files[FILE_CPP_HEADER] = FILE_DEF(FILE_HEADER, "hpp", FILE_GROUP_TEXT);
    media->files[FILE_MP3] = FILE_DEF(FILE_MP3, "mp3", FILE_GROUP_MUSIC);
    media->files[FILE_WAV] = FILE_DEF(FILE_WAV, "wav", FILE_GROUP_MUSIC);
    media->files[FILE_OGG] = FILE_DEF(FILE_OGG, "ogg", FILE_GROUP_MUSIC);
    media->files[FILE_TTF] = FILE_DEF(FILE_TTF, "ttf", FILE_GROUP_FONT);
    media->files[FILE_BMP] = FILE_DEF(FILE_BMP, "bmp", FILE_GROUP_IMAGE);
    media->files[FILE_PNG] = FILE_DEF(FILE_PNG, "png", FILE_GROUP_IMAGE);
    media->files[FILE_JPEG] = FILE_DEF(FILE_JPEG, "jpg", FILE_GROUP_IMAGE);
    media->files[FILE_PCX] = FILE_DEF(FILE_PCX, "pcx", FILE_GROUP_IMAGE);
    media->files[FILE_TGA] = FILE_DEF(FILE_TGA, "tga", FILE_GROUP_IMAGE);
    media->files[FILE_GIF] = FILE_DEF(FILE_GIF, "gif", FILE_GROUP_IMAGE);
}

static void
file_browser_reload_directory_content(struct file_browser *browser, const char *path)
{
    strncpy(browser->directory, path, MAX_PATH_LEN);
    dir_free_list(browser->files, browser->file_count);
    dir_free_list(browser->directories, browser->dir_count);
    browser->files = dir_list(path, 0, &browser->file_count);
    browser->directories = dir_list(path, 1, &browser->dir_count);
}

static void
file_browser_init(struct file_browser *browser, struct media *media)
{
    memset(browser, 0, sizeof(*browser));
    browser->media = media;
    {
        /* load files and sub-directory list */
        const char *home = getenv("HOME");
#ifdef _WIN32
        if (!home) home = getenv("USERPROFILE");
#else
        if (!home) home = getpwuid(getuid())->pw_dir;
        {
            size_t l;
            strncpy(browser->home, home, MAX_PATH_LEN);
            l = strlen(browser->home);
            strcpy(browser->home + l, "/");
            strcpy(browser->directory, browser->home);
        }
#endif
        {
            size_t l;
            strcpy(browser->desktop, browser->home);
            l = strlen(browser->desktop);
            strcpy(browser->desktop + l, "desktop/");
        }
        browser->files = dir_list(browser->directory, 0, &browser->file_count);
        browser->directories = dir_list(browser->directory, 1, &browser->dir_count);
    }
}

static void
file_browser_free(struct file_browser *browser)
{
    if (browser->files)
        dir_free_list(browser->files, browser->file_count);
    if (browser->directories)
        dir_free_list(browser->directories, browser->dir_count);
    browser->files = NULL;
    browser->directories = NULL;
    memset(browser, 0, sizeof(*browser));
}

static int
file_browser_run(struct file_browser *browser, struct nk_context *ctx)
{
    int ret = 0;
    struct media *media = browser->media;
    struct nk_rect total_space;

    if (nk_begin(ctx, "File Browser", nk_rect(0, 0, 800, 600),
        NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR))
    {
        static float ratio[] = {0.25f, NK_UNDEFINED};
        float spacing_x = ctx->style.window.spacing.x;

        /* output path directory selector in the menubar */
        ctx->style.window.spacing.x = 0;
        nk_menubar_begin(ctx);
        {
            nk_layout_row_dynamic(ctx, 25, 6);

            #ifdef __unix__
            if (nk_button_label(ctx, "/"))
                file_browser_reload_directory_content(browser, "/");
            #endif
            char *d = browser->directory;
            char *begin = d + 1;
            while (*d++) {
                if (*d == '/') {
                    *d = '\0';
                    if (nk_button_label(ctx, begin)) {
                        *d++ = '/';
                        *d = '\0';
                        file_browser_reload_directory_content(browser, browser->directory);
                        break;
                    }
                    *d = '/';
                    begin = d + 1;
                }
            }
        }
        nk_menubar_end(ctx);
        ctx->style.window.spacing.x = spacing_x;

        /* window layout */
        total_space = nk_window_get_content_region(ctx);
        nk_layout_row(ctx, NK_DYNAMIC, total_space.h, 2, ratio);
        nk_group_begin(ctx, "Special", NK_WINDOW_NO_SCROLLBAR);
        {
            struct nk_image home = media->icons.home;
            struct nk_image desktop = media->icons.desktop;
            struct nk_image computer = media->icons.computer;

            nk_layout_row_dynamic(ctx, 40, 1);
            if (nk_button_image_label(ctx, home, "home", NK_TEXT_CENTERED))
                file_browser_reload_directory_content(browser, browser->home);
            if (nk_button_image_label(ctx,desktop,"desktop",NK_TEXT_CENTERED))
                file_browser_reload_directory_content(browser, browser->desktop);
            if (nk_button_image_label(ctx,computer,"computer",NK_TEXT_CENTERED))
                file_browser_reload_directory_content(browser, "/");
            nk_group_end(ctx);
        }

        /* output directory content window */
        nk_group_begin(ctx, "Content", 0);
        {
            int index = -1;
            size_t i = 0, j = 0, k = 0;
            size_t rows = 0, cols = 0;
            size_t count = browser->dir_count + browser->file_count;

            cols = 4;
            rows = count / cols;
            for (i = 0; i <= rows; i += 1) {
                {size_t n = j + cols;
                nk_layout_row_dynamic(ctx, 135, (int)cols);
                for (; j < count && j < n; ++j) {
                    /* draw one row of icons */
                    if (j < browser->dir_count) {
                        /* draw and execute directory buttons */
                        if (nk_button_image(ctx,media->icons.directory))
                            index = (int)j;
                    } else {
                        /* draw and execute files buttons */
                        struct nk_image *icon;
                        size_t fileIndex = ((size_t)j - browser->dir_count);
                        icon = media_icon_for_file(media,browser->files[fileIndex]);
                        if (nk_button_image(ctx, *icon)) {
                            strncpy(browser->file, browser->directory, MAX_PATH_LEN);
                            n = strlen(browser->file);
                            strncpy(browser->file + n, browser->files[fileIndex], MAX_PATH_LEN - n);
                            ret = 1;
                        }
                    }
                }}
                {size_t n = k + cols;
                nk_layout_row_dynamic(ctx, 20, (int)cols);
                for (; k < count && k < n; k++) {
                    /* draw one row of labels */
                    if (k < browser->dir_count) {
                        nk_label(ctx, browser->directories[k], NK_TEXT_CENTERED);
                    } else {
                        size_t t = k-browser->dir_count;
                        nk_label(ctx,browser->files[t],NK_TEXT_CENTERED);
                    }
                }}
            }

            if (index != -1) {
                size_t n = strlen(browser->directory);
                strncpy(browser->directory + n, browser->directories[index], MAX_PATH_LEN - n);
                n = strlen(browser->directory);
                if (n < MAX_PATH_LEN - 1) {
                    browser->directory[n] = '/';
                    browser->directory[n+1] = '\0';
                }
                file_browser_reload_directory_content(browser, browser->directory);
            }
            nk_group_end(ctx);
        }
    }
    nk_end(ctx);
    return ret;
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

// Unlike TIMER_GET, this does not reset the timer
#define TIMER_GET_ELAPSED(x, d) ctime##x = SDL_GetTicks();      \
                                d = ctime##x - ltime##x;

#define TIMER_RESET(x)  ctime##x = SDL_GetTicks();      \
                        ltime##x = ctime##x;

#define FPS_INIT(y)     float delaytime##y = 0;

#define FPS_LOCK_MAX(y, d, fps) if ((delaytime##y += (1000/(fps))-d) > 0)   \
                                    if (delaytime##y > 0)                   \
                                        SDL_Delay(delaytime##y);            \


// updatecnt describes how often the fps should be updated
// 1 -> after 1sec, 2 -> every half sec, 10 -> every 100msec
#define FPS_COUNT_INIT(z, uc)   float cycles##z = 0;      \
                                unsigned int count##z = 0;       \
                                float updatecnt##z = uc;

// You gotta call this in your main loop and it returns the current fps
#define FPS_COUNT_TICK(z, d, fps)   cycles##z += d;                             \
                                    count##z++;                                 \
                                    if (cycles##z > 1000*(1/updatecnt##z)) {    \
                                        cycles##z -= 1000*(1/updatecnt##z);     \
                                        fps = count##z*updatecnt##z;            \
                                        count##z = 0;                           \
                                    }

// This is used to simulate min "FPS", so instead of lag,
// it runs the game logic "slower"
#define DELTA_MIN_STEP(d, fps)  if (d > 1000/fps)   \
                                    d = 1000/fps;   \

/* ===============================================================
 *
 *                          DEVICE
 *
 * ===============================================================*/
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
    struct file_browser browser;
    struct media media;

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        return 1;
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

    SDL_Window *window = SDL_CreateWindow("Demo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, 0);
    if (!window)
    {
        SDL_Log("Failed to open window: %s", SDL_GetError());
        return 2;
    }

    SDL_Surface* screen = SDL_GetWindowSurface(window);
    SDL_SetSurfaceBlendMode(screen, SDL_BLENDMODE_NONE);

    struct sdlsurface_context *context = nk_sdlsurface_init(800, 600);
    struct nk_context *ctx = &(context->ctx);

    nk_sdl_font_stash_begin(context);
    context->atlas.default_font = nk_font_atlas_add_default(&context->atlas, 13.0f, 0);
    nk_sdl_font_stash_end(context);

    /* icons */
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG))
    {
        SDL_Log("Failed to initialise SDL_Image: %s", IMG_GetError());
        return 3;
    }

    media.icons.home = icon_load("../../example/icon/home.png");
    media.icons.directory = icon_load("../../example/icon/directory.png");
    media.icons.computer = icon_load("../../example/icon/computer.png");
    media.icons.desktop = icon_load("../../example/icon/desktop.png");
    media.icons.default_file = icon_load("../../example/icon/default.png");
    media.icons.text_file = icon_load("../../example/icon/text.png");
    media.icons.music_file = icon_load("../../example/icon/music.png");
    media.icons.font_file =  icon_load("../../example/icon/font.png");
    media.icons.img_file = icon_load("../../example/icon/img.png");
    media.icons.movie_file = icon_load("../../example/icon/movie.png");
    media_init(&media);

    /* FPS Counter and Limiter */
    float delta = 0;
    unsigned int fps = 0;
    TIMER_INIT(dcnt);
    FPS_INIT(fpscontrol);
    FPS_COUNT_INIT(fpsdisp, 2.0f);

    file_browser_init(&browser, &media);
    for (;;)
    {
        /* Input */
        nk_input_begin(ctx);
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                goto quit;

            nk_sdlsurface_handle_event(ctx, &event);
        }
        nk_input_end(ctx);

        /* GUI */
        file_browser_run(&browser, ctx);

        /* Draw */
        nk_sdlsurface_render(context, (const struct nk_color){0, 0, 0, 0}, 0);
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

    SDL_FreeSurface((SDL_Surface *)media.icons.home.handle.ptr);
    SDL_FreeSurface((SDL_Surface *)media.icons.directory.handle.ptr);
    SDL_FreeSurface((SDL_Surface *)media.icons.computer.handle.ptr);
    SDL_FreeSurface((SDL_Surface *)media.icons.desktop.handle.ptr);
    SDL_FreeSurface((SDL_Surface *)media.icons.default_file.handle.ptr);
    SDL_FreeSurface((SDL_Surface *)media.icons.text_file.handle.ptr);
    SDL_FreeSurface((SDL_Surface *)media.icons.music_file.handle.ptr);
    SDL_FreeSurface((SDL_Surface *)media.icons.font_file.handle.ptr);
    SDL_FreeSurface((SDL_Surface *)media.icons.img_file.handle.ptr);
    SDL_FreeSurface((SDL_Surface *)media.icons.movie_file.handle.ptr);
    
    quit:
    file_browser_free(&browser);
    nk_sdlsurface_shutdown(context);
    screen = NULL;
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}


