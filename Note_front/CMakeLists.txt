cmake_minimum_required(VERSION 3.16)

project(R-Note LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Gui Core Widgets Network)

# 添加主程序
add_subdirectory(NoteApp)

# 添加子模块
add_subdirectory(src/Auth)
add_subdirectory(src/Editor)
add_subdirectory(src/Http)
add_subdirectory(src/MainWindow)
add_subdirectory(src/Tools)