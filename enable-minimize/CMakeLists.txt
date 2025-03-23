cmake_minimum_required(VERSION 3.26)
project(enable-minimize VERSION 1.0.0)

add_library(enable-minimize SHARED
        dllmain.cpp
        ${CMAKE_SOURCE_DIR}/common/hooker.cpp
        ${CMAKE_SOURCE_DIR}/common/logger.cpp
)

# Set additional include directories if needed
target_include_directories(enable-minimize PRIVATE ${CMAKE_SOURCE_DIR}/common)
target_link_libraries(enable-minimize PRIVATE ${CMAKE_SOURCE_DIR}/common/detours.lib)
target_link_libraries(enable-minimize PRIVATE build_config)

set_target_properties(enable-minimize PROPERTIES OUTPUT_NAME "enable-minimize-${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")