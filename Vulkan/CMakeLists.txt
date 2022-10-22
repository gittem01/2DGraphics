project(VkGraphics2D VERSION 1.0.0)

set(CMAKE_C_STANDARD 17)

# put this before add_executable else get linker error
link_directories(${CMAKE_SOURCE_DIR}/third_party/vulkan/lib)

include_directories(include)

add_executable(VkGraphics2D main.cpp src/Shader.cpp include/Shader.h)

file(GLOB sourceFiles src/*.cpp src/ShaderSources/*.cpp)
file(GLOB headerFiles include/ShaderHeaders/*.h)
file(GLOB headerFiles include/*.h)

add_library(vkMines ${headerFiles} ${sourceFiles})

target_link_libraries(VkGraphics2D vkMines glfw)

if (APPLE)
	target_link_libraries(VkGraphics2D vulkan)
elseif(UNIX)
	target_link_libraries(VkGraphics2D vulkanUnx)
elseif(WIN32)
	target_link_libraries(VkGraphics2D vulkanWin)
endif()