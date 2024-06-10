cmake_minimum_required(VERSION 3.10)

project(hello_sdl3)
set(CMAKE_CXX_STANDARD 20)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/third_party/SDL3" EXCLUDE_FROM_ALL)
include_directories(third_party/SDL3/include)

add_subdirectory(third_party/glad EXCLUDE_FROM_ALL)
add_subdirectory(utils)

link_libraries(SDL3::SDL3 utils)

add_executable(hello_window hello_window.cpp)
add_executable(hello_triangle hello_triangle.cpp)
add_executable(hello_triangle_gl hello_triangle_gl.cpp)
add_executable(hello_texture hello_texture.cpp)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_executable(interop_with_d3d11 interop_with_d3d11.cpp)
endif()

target_include_directories(hello_triangle_gl PRIVATE third_party/glad/include)
target_link_libraries(hello_triangle_gl PRIVATE glad)

# This is safe to set on all platforms. Otherwise your SDL app will have a
# terminal window pop up with it on Windows. Must include SDL3/SDL_main.h to use
# this.
# set_property(TARGET hello_triangle PROPERTY WIN32_EXECUTABLE TRUE)
# set_property(TARGET hello_triangle_gl PROPERTY WIN32_EXECUTABLE TRUE)

# Copy the DLLs to the output directory
add_custom_command(TARGET hello_window POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:SDL3::SDL3>
    $<TARGET_FILE_DIR:hello_window>
)
