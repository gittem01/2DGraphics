cmake_minimum_required(VERSION 3.0)
project(2DGraphics VERSION 1.0.0)

include_directories(include)

add_executable(2DGraphics main.cpp)

file(GLOB sourceFiles src/*.cpp)
file(GLOB headerFiles include/*.h)
add_library(mines ${sourceFiles} ${headerFiles})

target_link_libraries(2DGraphics OpenGL::GL)
target_link_libraries(2DGraphics mines)
target_link_libraries(2DGraphics glfw)
target_link_libraries(2DGraphics GLAD)
