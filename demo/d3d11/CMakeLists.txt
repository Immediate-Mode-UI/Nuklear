cmake_minimum_required(VERSION 4.0)
set(CMAKE_C_STANDARD 17)
project(NuklearDemo LANGUAGES C)

file(GLOB SOURCES CONFIGURE_DEPENDS
   RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
   "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
   "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)

add_executable(D3D11Demo ${SOURCES})

target_link_libraries(D3D11Demo PRIVATE dxgi.lib D3D11.lib dxguid.lib d3dcompiler.lib)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT D3D11Demo)