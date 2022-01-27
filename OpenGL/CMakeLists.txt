cmake_minimum_required(VERSION 3.0)
project(2DGraphics VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 14)

add_subdirectory(${CMAKE_SOURCE_DIR}/third_party/GLFW)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/third_party/GLM/include)
include_directories(${CMAKE_SOURCE_DIR}/third_party/GLFW/include)
include_directories(${CMAKE_SOURCE_DIR}/third_party/GLAD/include)

add_executable(2DGraphics main.cpp)

file(GLOB sourceFiles ${CMAKE_SOURCE_DIR}/src/*.cpp)
file(GLOB headerFiles ${CMAKE_SOURCE_DIR}/include/*.h)
add_library(mines ${sourceFiles} ${headerFiles})

add_library(GLAD ${CMAKE_SOURCE_DIR}/third_party/GLAD/src/glad.c)

find_package(OpenGL REQUIRED)
target_link_libraries(2DGraphics OpenGL::GL)

target_link_libraries(2DGraphics mines)
target_link_libraries(2DGraphics glfw)
target_link_libraries(2DGraphics GLAD)
