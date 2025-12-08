# 直接编译为静态库链接到程序
add_library(md4c STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/md4c.c
    ${CMAKE_CURRENT_SOURCE_DIR}/md4c-html.c
    ${CMAKE_CURRENT_SOURCE_DIR}/entity.c
)

target_include_directories(md4c
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)