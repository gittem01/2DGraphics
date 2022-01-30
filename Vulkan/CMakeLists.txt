cmake_minimum_required(VERSION 3.0)
project(2DGraphics VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 14)

add_subdirectory(${CMAKE_SOURCE_DIR}/third_party/GLFW)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/third_party/GLM/include)
include_directories(${CMAKE_SOURCE_DIR}/third_party/GLFW/include)
include_directories(${CMAKE_SOURCE_DIR}/third_party/vulkan/include)


# put this before add_executable else get linker error
link_directories(${CMAKE_SOURCE_DIR}/third_party/vulkan/lib)

add_executable(2DGraphics main.cpp)

file(GLOB sourceFiles ${CMAKE_SOURCE_DIR}/src/*.cpp)
file(GLOB headerFiles ${CMAKE_SOURCE_DIR}/include/*.h)
# add_library(mines ${sourceFiles} ${headerFiles})

# target_link_libraries(2DGraphics mines)
target_link_libraries(2DGraphics glfw)

if (APPLE)
	target_link_libraries(2DGraphics vulkanMac.dylib)
elseif(UNIX)
	target_link_libraries(2DGraphics vulkanUnx)
elseif(WIN32)
	target_link_libraries(2DGraphics vulkanWin)
endif()