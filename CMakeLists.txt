cmake_minimum_required(VERSION 3.0)
project(base VERSION 1.0.0)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(${CMAKE_SOURCE_DIR}/third_party/GLFW)
add_subdirectory(${CMAKE_SOURCE_DIR}/third_party/freetype)
add_subdirectory(${CMAKE_SOURCE_DIR}/third_party/cglm)

include_directories(${CMAKE_SOURCE_DIR}/third_party/GLM/include)
include_directories(${CMAKE_SOURCE_DIR}/third_party/GLFW/include)
include_directories(${CMAKE_SOURCE_DIR}/third_party/GLAD/include)
include_directories(${CMAKE_SOURCE_DIR}/third_party/cglm/include)
include_directories(${CMAKE_SOURCE_DIR}/third_party/freetype/include)
include_directories(${CMAKE_SOURCE_DIR}/third_party/stb_image)
include_directories(${CMAKE_SOURCE_DIR}/third_party/json)
include_directories(${CMAKE_SOURCE_DIR}/third_party/vulkan/include)
include_directories(${CMAKE_SOURCE_DIR}/third_party/perlin)

add_library(GLAD STATIC ${CMAKE_SOURCE_DIR}/third_party/GLAD/src/glad.c)
add_library(JSON STATIC ${CMAKE_SOURCE_DIR}/third_party/json/json11.cpp)

find_package(OpenGL REQUIRED)

add_subdirectory(OpenGL)
add_subdirectory(OpenGL_GUI)
add_subdirectory(Vulkan)