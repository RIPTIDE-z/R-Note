target_sources(${PROJECT_NAME}
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/markdown_preview_widget.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/markdown_editor_widget.cpp
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)