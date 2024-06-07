cmake_minimum_required(VERSION 3.10)
project(hello_sdl3)

add_subdirectory(third_party/glad EXCLUDE_FROM_ALL)
add_subdirectory(third_party/SDL3 EXCLUDE_FROM_ALL)
add_subdirectory(utils)

include_directories(${SDL2_INCLUDE_DIRS})
link_libraries(SDL3::SDL3 utils)

add_executable(hello_sdl3 hello_sdl3.cpp)
add_executable(hello_triangle hello_triangle.cpp)
add_executable(hello_triangle_gl hello_triangle_gl.cpp)
add_executable(hello_texture hello_texture.cpp)

target_include_directories(hello_triangle_gl PRIVATE third_party/glad/include)
target_link_libraries(hello_triangle_gl PRIVATE glad)

# This is safe to set on all platforms. Otherwise your SDL app will have a
# terminal window pop up with it on Windows. Must include SDL3/SDL_main.h to use
# this.
# set_property(TARGET hello_triangle PROPERTY WIN32_EXECUTABLE TRUE)
# set_property(TARGET hello_triangle_gl PROPERTY WIN32_EXECUTABLE TRUE)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  set(SDL3_DLL SDL3.dll)
else()
  set(SDL3_DLL libSDL3.so)
endif()

# Copy the DLLs to the output directory
add_custom_command(
  TARGET hello_triangle
  POST_BUILD
  COMMAND
    ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_BINARY_DIR}/third_party/SDL3/${SDL3_DLL}"
    $<TARGET_FILE_DIR:hello_triangle>)
