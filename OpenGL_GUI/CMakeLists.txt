cmake_minimum_required(VERSION 3.0)
project(GUI_Test VERSION 1.0.0)

include_directories(include)
include_directories(${CMAKE_SOURCE_DIR}/third_party/json)

add_executable(GUI_Test main.cpp)

file(GLOB sourceFiles src/*.cpp)
file(GLOB headerFiles include/*.h)
add_library(mines2 ${sourceFiles} ${headerFiles})

target_link_libraries(GUI_Test OpenGL::GL)
target_link_libraries(GUI_Test mines2)
target_link_libraries(GUI_Test glfw)
target_link_libraries(GUI_Test GLAD)
target_link_libraries(GUI_Test freetype)
target_link_libraries(GUI_Test JSON)