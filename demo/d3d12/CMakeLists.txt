cmake_minimum_required(VERSION 4.0)
set(CMAKE_C_STANDARD 17)
project(NuklearDemo LANGUAGES C)

file(GLOB SOURCES CONFIGURE_DEPENDS
   RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
   "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
   "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)

add_executable(D3D12Demo ${SOURCES})

target_link_libraries(D3D12Demo PRIVATE dxgi.lib D3D12.lib dxguid.lib d3dcompiler.lib)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT D3D12Demo)