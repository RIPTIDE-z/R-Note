set(APP_SOURCES
    main.cpp
    ${CMAKE_SOURCE_DIR}/resource/resource.qrc
)

qt_add_executable(${PROJECT_NAME}
    WIN32
    ${APP_SOURCES}
)


target_link_libraries(R-Note
    PRIVATE 
      Qt6::Core 
      Qt6::Gui 
      Qt6::Widgets 
      Qt6::Network
      QWindowKit::Widgets
)