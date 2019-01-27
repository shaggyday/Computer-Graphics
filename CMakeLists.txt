cmake_minimum_required(VERSION 3.10)
project(day_7 C)

set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang")

find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)

find_package(OpenGL REQUIRED)

include_directories(. ${GLFW_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIR})

add_executable(day_7 130mainDepth.c 000pixel.o)

target_link_libraries(day_7 ${GLFW_LIBRARIES} ${OPENGL_LIBRARIES} m)