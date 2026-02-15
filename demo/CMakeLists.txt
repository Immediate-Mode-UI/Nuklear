set(demos
        "allegro5"
#        "common"
#        "d3d11"
#        "d3d12"
#        "d3d9"
#        "gdi"
#        "gdip"
#        "glfw_opengl2"
#        "glfw_opengl3"
#        "glfw_opengl4"
#        "sdl2surface_rawfb"
#        "sdl_opengl2"
#        "sdl_opengl3"
#        "sdl_opengles2"
#        "sdl_renderer"
#        "sfml_opengl2"
#        "sfml_opengl3"
#        "wayland_rawfb"
#        "x11"
#        "x11_opengl2"
#        "x11_opengl3"
#        "x11_rawfb"
#        "x11_xft"
        )

foreach (demo ${demos})
    add_subdirectory("${demo}")
endforeach (demo ${demos})
