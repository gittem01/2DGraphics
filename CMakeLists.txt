cmake_minimum_required(VERSION 3.0)
project(base VERSION 1.0.0)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(${CMAKE_SOURCE_DIR}/third_party/GLFW)

include_directories(${CMAKE_SOURCE_DIR}/third_party/GLM/include)
include_directories(${CMAKE_SOURCE_DIR}/third_party/GLFW/include)
include_directories(${CMAKE_SOURCE_DIR}/third_party/stb_image)
include_directories(${CMAKE_SOURCE_DIR}/third_party/vulkan/include)

add_subdirectory(Vulkan)