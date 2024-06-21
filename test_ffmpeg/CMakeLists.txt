cmake_minimum_required(VERSION 3.10)

if(WIN32)
    if(NOT DEFINED VCPKG_ROOT)
        set(VCPKG_ROOT $ENV{VCPKG_ROOT})
    endif()

    if(NOT DEFINED VCPKG_ROOT)
        message(FATAL_ERROR "VCPKG_ROOT not defined")
    endif()

    set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
    set(VCPKG_INSTALLED_DIR "${VCPKG_ROOT}/installed/x64-windows")
    set(VCPKG_BINARY_DIR "${VCPKG_INSTALLED_DIR}/debug/bin")
    list(APPEND CMAKE_MODULE_PATH "${VCPKG_ROOT}/packages/ffmpeg_x64-windows/share/ffmpeg")
endif()

project(test_ffmpeg)
set(CMAKE_CXX_STANDARD 20)

set(THIRD_PARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../third_party")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# SDL3
add_subdirectory("${THIRD_PARTY_DIR}/SDL3" "${CMAKE_CURRENT_BINARY_DIR}/SDL3-build" EXCLUDE_FROM_ALL)

include_directories("${THIRD_PARTY_DIR}/SDL3/include")
link_libraries(SDL3::SDL3 SDL3::SDL3_test)

# Vulkan
find_package(Vulkan REQUIRED)
link_libraries(Vulkan::Vulkan)

# FFmpeg
find_package(FFmpeg REQUIRED)
include_directories(${FFMPEG_INCLUDE_DIRS})
link_libraries(${FFMPEG_LIBRARIES})
message(STATUS "FFMPEG_LIBRARIES: ${FFMPEG_LIBRARIES}")

# target_include_directories(testffmpeg PRIVATE ${FFMPEG_INCLUDE_DIRS})
set(TESTFFMPEG_SOURCES
    testffmpeg.cpp
    testffmpeg_vulkan.cpp
)
add_executable(testffmpeg ${TESTFFMPEG_SOURCES})

if (WIN32)
# Copy the DLLs to the output directory
add_custom_command(TARGET testffmpeg POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${VCPKG_BINARY_DIR}/avcodec-60.dll
    ${VCPKG_BINARY_DIR}/avformat-60.dll
    ${VCPKG_BINARY_DIR}/avutil-58.dll
    ${VCPKG_BINARY_DIR}/swresample-4.dll
    ${VCPKG_BINARY_DIR}/swscale-7.dll
    $<TARGET_FILE:SDL3::SDL3>
    $<TARGET_FILE_DIR:testffmpeg>
)
endif()
