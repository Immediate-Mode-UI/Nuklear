set(_libs "file_browser" "extended" "canvas" "skinning")

find_package(glfw3 CONFIG REQUIRED)
find_package(GLEW REQUIRED)

include(FindOpenGL)
find_package(OpenGL REQUIRED)

include("${CMAKE_SOURCE_DIR}/cmake/modules/CMakeFindM.cmake")

set(deps "glfw" "GLEW::GLEW" "OpenGL::GL" "${M_LIBRARIES}")
if (APPLE)
    #SET(GUI_TYPE MACOSX_BUNDLE)
    #INCLUDE_DIRECTORIES ( /Developer/Headers/FlatCarbon )
    find_library(COCOA_LIBRARY Cocoa)
    find_library(COREVIDEO_LIBRARY CoreVideo)
    find_library(IOKIT_LIBRARY IOKit)
    mark_as_advanced(COCOA_LIBRARY COREVIDEO_LIBRARY IOKIT_LIBRARY)
    list(APPEND deps "${COCOA_LIBRARY}" "${COREVIDEO_LIBRARY}" "${IOKIT_LIBRARY}")
    # -L/usr/local/lib
    # -I/usr/local/include
endif (APPLE)

foreach (_lib ${_libs})
    set(src "${_lib}.c")
    source_group("${_lib} Files" FILES "${src}")
    add_executable("${_lib}" "${src}")
    target_link_libraries("${_lib}" PRIVATE "${deps}")
    target_include_directories("${_lib}" PRIVATE "${M_INCLUDE_DIRS}")
endforeach (_lib ${_libs})
