cmake_minimum_required(VERSION 3.26)
project(window-mode VERSION 1.0.0)

add_library(window-mode SHARED
        dllmain.cpp
        ${CMAKE_SOURCE_DIR}/common/memedit.cpp
)

# Set additional include directories if needed
target_include_directories(window-mode PRIVATE ${CMAKE_SOURCE_DIR}/common)
target_link_libraries(window-mode PRIVATE ${CMAKE_SOURCE_DIR}/common/detours.lib)
target_link_libraries(window-mode PRIVATE build_config)

set_target_properties(window-mode PROPERTIES OUTPUT_NAME "window-mode-${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
