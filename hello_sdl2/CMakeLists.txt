cmake_minimum_required(VERSION 3.10)

if(WIN32)
    if(NOT DEFINED VCPKG_ROOT)
        set(VCPKG_ROOT $ENV{VCPKG_ROOT})
    endif()

    if(NOT DEFINED VCPKG_ROOT)
        message(FATAL_ERROR "VCPKG_ROOT not defined")
    endif()

    set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}\\scripts\\buildsystems\\vcpkg.cmake")
    message(STATUS "CMAKE_TOOLCHAIN_FILE: ${CMAKE_TOOLCHAIN_FILE}")
endif()

project(hello_sdl2)

find_package(SDL2 CONFIG REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})
link_libraries(${SDL2_LIBRARIES})

add_executable(hello_triangle hello_triangle.cpp)
add_executable(hello_triangle_gl hello_triangle_gl.cpp)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_subdirectory(third_party/glad)
    target_include_directories(hello_triangle_gl PRIVATE third_party/glad/include)
    target_link_libraries(hello_triangle_gl PRIVATE glad)
endif()