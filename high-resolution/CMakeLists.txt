cmake_minimum_required(VERSION 3.26)
project(high-resolution VERSION 1.0.0)

add_library(high-resolution SHARED
        dllmain.cpp
        ${CMAKE_SOURCE_DIR}/common/memedit.cpp
        ${CMAKE_SOURCE_DIR}/common/hooker.cpp
        ${CMAKE_SOURCE_DIR}/common/logger.cpp
)

# Set additional include directories if needed
target_include_directories(high-resolution PRIVATE ${CMAKE_SOURCE_DIR}/common)
target_link_libraries(high-resolution PRIVATE ${CMAKE_SOURCE_DIR}/common/detours.lib)
target_link_libraries(high-resolution PRIVATE build_config)

set_target_properties(high-resolution PROPERTIES OUTPUT_NAME "high-resolution-${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")