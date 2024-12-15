include(FetchContent)
FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG eb3fc06
    #GIT_SHALLOW 1
)
FetchContent_MakeAvailable(SDL3)
