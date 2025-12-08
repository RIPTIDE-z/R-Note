cmake_minimum_required(VERSION 3.21)

project(R-Note LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 安装输出目录（放到 bin文件夹）
set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/bin"
    CACHE PATH "Install path prefix" FORCE)

find_package(Qt6 REQUIRED COMPONENTS Gui Core Widgets Network)

# 如果希望 QWindowKit 以“静态库”的形式直接被链接进exe：
set(QWINDOWKIT_BUILD_STATIC ON CACHE BOOL "Build QWindowKit as static libraries" FORCE)
# 不需要安装 QWindowKit 自己的头文件/库
set(QWINDOWKIT_INSTALL OFF CACHE BOOL "" FORCE)

# Widgets 模块
set(QWINDOWKIT_BUILD_WIDGETS ON CACHE BOOL "" FORCE)
# 关掉 Quick 模块
set(QWINDOWKIT_BUILD_QUICK OFF CACHE BOOL "" FORCE)

# 不编 Examples
set(QWINDOWKIT_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
# 不生成文档
set(QWINDOWKIT_BUILD_DOCUMENTATIONS OFF CACHE BOOL "" FORCE)

# 添加QWindowkit依赖
add_subdirectory(external/QWindowKit)

# 添加md4c依赖，编译为静态库
add_subdirectory(external/md4c)

# src, 内含add_executable
add_subdirectory(src)
