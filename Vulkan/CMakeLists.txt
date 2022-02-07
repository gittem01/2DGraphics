cmake_minimum_required(VERSION 3.0)
project(VkGraphics2D VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 14)

# put this before add_executable else get linker error
link_directories(${CMAKE_SOURCE_DIR}/third_party/vulkan/lib)

include_directories(include)

add_executable(VkGraphics2D main.c)

file(GLOB sourceFiles src/*.c)
file(GLOB headerFiles include/*.h)

add_library(vkMines ${headerFiles} ${sourceFiles})

# target_link_libraries(VkGraphics2D vkMines)
target_link_libraries(VkGraphics2D glfw)

if (APPLE)
	target_link_libraries(VkGraphics2D vulkanMac.dylib)
elseif(UNIX)
	target_link_libraries(VkGraphics2D vulkanUnx)
elseif(WIN32)
	target_link_libraries(VkGraphics2D vulkanWin)
endif()