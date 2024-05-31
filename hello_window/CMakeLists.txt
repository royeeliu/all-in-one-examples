cmake_minimum_required(VERSION 3.10)

if(WIN32)
    if(NOT DEFINED VCPKG_ROOT)
        set(VCPKG_ROOT $ENV{VCPKG_ROOT})
    endif()

    if(NOT DEFINED VCPKG_ROOT)
        message(FATAL_ERROR "VCPKG_ROOT not defined")
    endif()

    SET(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}\\scripts\\buildsystems\\vcpkg.cmake")
    message(STATUS "CMAKE_TOOLCHAIN_FILE: ${CMAKE_TOOLCHAIN_FILE}")
endif()

project(hello_window)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # hello_x11
    find_package(X11 REQUIRED)
    include_directories(${X11_INCLUDE_DIR})

    add_executable(hello_x11 hello_x11.cpp)

    target_link_libraries(hello_x11 ${X11_LIBRARIES})

    # hello_wayland
    #[[
find_package(PkgConfig REQUIRED)
pkg_check_modules(WAYLAND_CLIENT wayland - client REQUIRED)
]]
    add_executable(hello_wayland hello_wayland.cpp)
    target_link_libraries(hello_wayland ${WAYLAND_CLIENT_LIBRARIES})
    target_include_directories(hello_wayland PUBLIC ${WAYLAND_CLIENT_INCLUDE_DIRS})
    target_compile_options(hello_wayland PUBLIC ${WAYLAND_CLIENT_CFLAGS_OTHER})
endif()

# hello_sdl2
find_package(SDL2 CONFIG REQUIRED)
add_executable(hello_sdl2 hello_sdl2.cpp)
target_include_directories(hello_sdl2 PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(hello_sdl2 PRIVATE ${SDL2_LIBRARIES})
