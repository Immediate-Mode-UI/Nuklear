cmake_minimum_required(VERSION 3.16)
project(nuklear_sdl3
    DESCRIPTION "nuklear_sdl3: Nuklear Immediate Mode GUI for SDL3"
    HOMEPAGE_URL "https://github.com/Immediate-Mode-UI/Nuklear/"
    LANGUAGES C
)


#  glad
add_subdirectory(glad)
target_include_directories(glad PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/glad/include)
# nuklear
add_library(nuklear INTERFACE)
target_include_directories(nuklear INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../..)

# nuklear_sdl3
add_library(nuklear_sdl3 INTERFACE)
target_include_directories(nuklear_sdl3 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(SDL3 REQUIRED)

add_executable(nuklear_sdl3_ogl3
    nuklear_sdl3_ogl3.c
)

target_link_libraries(nuklear_sdl3_ogl3 PUBLIC
    nuklear
    nuklear_sdl3
    SDL3::SDL3
    glad
)


set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT "nuklear_sdl3_ogl3")


set_property(TARGET nuklear_sdl3_ogl3 PROPERTY C_STANDARD 99)