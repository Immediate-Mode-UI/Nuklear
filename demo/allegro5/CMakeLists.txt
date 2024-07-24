###################
# KeyboardHandler #
###################
#get_filename_component(LIBRARY_NAME "${CMAKE_CURRENT_SOURCE_DIR}" NAME)
#set(LIBRARY_NAME "${PROJECT_NAME}_demo_${LIBRARY_NAME}")
#
#set(Header_Files "KeyboardHandleriOS.h")
#source_group("Header Files" FILES "${Header_Files}")
#
#set(Source_Files "KeyboardHandleriOS.m")
#source_group("Source Files" FILES "${Source_Files}")
#
#add_library("${LIBRARY_NAME}" "${Header_Files}" "${Source_Files}")
#include(GNUInstallDirs)
#target_include_directories(
#        "${LIBRARY_NAME}"
#        PUBLIC
#        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
#        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
#)
#
#target_link_libraries(
#        "${LIBRARY_NAME}"
#        PRIVATE
#        "${PROJECT_NAME}_compiler_flags"
#)
#
#set_target_properties("${LIBRARY_NAME}" PROPERTIES LINKER_LANGUAGE C)

#################
# allegro5 demo #
#################

get_filename_component(EXEC_NAME "${CMAKE_CURRENT_SOURCE_DIR}" NAME)
set(EXEC_NAME "${PROJECT_NAME}_demo_${EXEC_NAME}")

set(Header_Files "nuklear_allegro5.h")
source_group("Header Files" FILES "${Header_Files}")

set(Source_Files "main.c")
source_group("Source Files" FILES "${Source_Files}")

add_executable("${EXEC_NAME}" "${Header_Files}" "${Source_Files}")
include(GNUInstallDirs)
target_include_directories(
        "${EXEC_NAME}"
        PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

target_link_libraries(
        "${EXEC_NAME}"
        PRIVATE
        "${PROJECT_NAME}_compiler_flags"
)

find_package(unofficial-allegro5 CONFIG REQUIRED)
target_link_libraries(
        "${EXEC_NAME}"
        PRIVATE
        "unofficial-allegro5::allegro"
        "unofficial-allegro5::allegro_ttf"
        "unofficial-allegro5::allegro_font"
        "unofficial-allegro5::allegro_main"
)
