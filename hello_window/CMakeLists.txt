cmake_minimum_required(VERSION 3.10)
project(hello_window)

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

# hello_sdl
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})
add_executable(hello_sdl hello_sdl.cpp)
target_link_libraries(hello_sdl ${SDL2_LIBRARIES})
