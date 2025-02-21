cmake_minimum_required(VERSION 3.16)
project(nuklear_sdl3
    DESCRIPTION "nuklear_sdl3: Nuklear Immediate Mode GUI for SDL3"
    HOMEPAGE_URL "https://github.com/Immediate-Mode-UI/Nuklear/"
    LANGUAGES C
)

# nuklear
add_library(nuklear INTERFACE)
target_include_directories(nuklear INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../..)

# nuklear_sdl3
add_library(nuklear_sdl3 INTERFACE)
target_include_directories(nuklear_sdl3 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# Options
if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
    set(NUKLEAR_SDL3_IS_MAIN TRUE)
else()
    set(NUKLEAR_SDL3_IS_MAIN FALSE)
endif()
option(NUKLEAR_SDL3_EXAMPLES "Examples" ${NUKLEAR_SDL3_IS_MAIN})

if (NUKLEAR_SDL3_EXAMPLES)
    # CMAKE Modules
    set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

    find_package(SDL3 REQUIRED)

    # nuklear_sdl3_renderer
    add_executable(nuklear_sdl3_renderer
        nuklear_sdl3_renderer.c
    )
    target_link_libraries(nuklear_sdl3_renderer PUBLIC
        nuklear
        nuklear_sdl3
        SDL3::SDL3
        m # TODO: Remove the math dependency and use nk_floor()
    )

    # on Visual Studio, set our app as the default project
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT "nuklear_sdl3_renderer")

    # Enable Warnings
    if(MSVC)
        target_compile_options(nuklear_sdl3_renderer PRIVATE /W4 /WX)
    else()
        target_compile_options(nuklear_sdl3_renderer PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()

    # SDL3 does not support C89/C90
    set_property(TARGET nuklear_sdl3_renderer PROPERTY C_STANDARD 99)
endif()
