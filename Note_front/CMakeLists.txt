cmake_minimum_required(VERSION 3.16)

project(R-Note LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Gui Core Widgets Network)

file(GLOB_RECURSE AUTH_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/auth/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/auth/*.h"
)

file(GLOB_RECURSE EDITOR_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/editor/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/editor/*.h"
)

file(GLOB_RECURSE HTTP_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/http/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/http/*.h"
)

set(SOURCES
    ${AUTH_SRCS}
    ${EDITOR_SRCS}
    ${HTTP_SRCS}
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/resource/resource.qrc"
)

qt_add_executable(${PROJECT_NAME}
    WIN32  
    ${SOURCES}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Network
        Qt6::Widgets
)
