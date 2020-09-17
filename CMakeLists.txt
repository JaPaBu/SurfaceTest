cmake_minimum_required(VERSION 3.16)
project(SurfaceTest)

set(CMAKE_CXX_STANDARD 20)

find_package(glfw3 3.3.2 REQUIRED)

add_executable(SurfaceTest main.cpp glad.c)

target_include_directories(SurfaceTest PRIVATE ext)

target_link_libraries(SurfaceTest glfw)