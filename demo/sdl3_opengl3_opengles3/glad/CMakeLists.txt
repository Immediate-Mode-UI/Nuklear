cmake_minimum_required(VERSION 3.18)

project(glad)

add_library(glad STATIC)

target_sources(glad PRIVATE src/glad.c)
target_include_directories(glad PUBLIC include)