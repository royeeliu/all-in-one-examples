# 生成链接库
add_library(glad ./src/glad.c)
target_include_directories(glad PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
