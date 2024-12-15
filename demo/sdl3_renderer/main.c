#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#define NK_SDL_RENDERER_IMPLEMENTATION
#include "nuklear_sdl3_renderer.h"

typedef struct AppContext {
    SDL_Window* window;
    SDL_Renderer* renderer;
} AppContext;

SDL_AppResult SDL_Fail(){
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Error %s", SDL_GetError());
    return SDL_APP_FAILURE;
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        return SDL_Fail();
    }

    AppContext* appContext = (AppContext*)SDL_malloc(sizeof(AppContext));
    if (appContext == NULL) {
        return SDL_Fail();
    }

    if (!SDL_CreateWindowAndRenderer("Nuklear: SDL3 Renderer", 1200, 800, SDL_WINDOW_RESIZABLE, &appContext->window, &appContext->renderer)) {
        SDL_free(appContext);
        return SDL_Fail();
    }

    *appstate = appContext;

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event* event) {
    AppContext* app = (AppContext*)appstate;

    switch (event->type) {
        case SDL_EVENT_QUIT:
            return SDL_APP_SUCCESS;
    }

    nk_sdl_handle_event(event);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    AppContext* app = (AppContext*)appstate;

    SDL_SetRenderDrawColor(app->renderer, 255, 0, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(app->renderer);
    SDL_RenderPresent(app->renderer);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
    AppContext* app = (AppContext*)appstate;

    if (app) {
        SDL_DestroyRenderer(app->renderer);
        SDL_DestroyWindow(app->window);
        SDL_free(app);
    }

    SDL_Quit();
}
