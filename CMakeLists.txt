cmake_minimum_required(VERSION 3.15)
project(nuklear
    DESCRIPTION "Nuklear: Immediate mode GUI"
    HOMEPAGE_URL "https://github.com/Immediate-Mode-UI/Nuklear"
    VERSION 4.13.2
    LANGUAGES C
)

add_library(nuklear INTERFACE)
target_include_directories(nuklear INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

install(FILES nuklear.h DESTINATION include)
install(TARGETS nuklear EXPORT nuklearTargets)
install(EXPORT nuklearTargets
    FILE nuklearTargets.cmake
    NAMESPACE nuklear::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/nuklear
)
