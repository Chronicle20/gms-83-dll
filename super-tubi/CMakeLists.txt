cmake_minimum_required(VERSION 3.26)
project(super-tubi VERSION 1.0.0)

add_library(super-tubi SHARED
        dllmain.cpp
        ${CMAKE_SOURCE_DIR}/common/memedit.cpp
)

# Set additional include directories if needed
target_include_directories(super-tubi PRIVATE ${CMAKE_SOURCE_DIR}/common)
target_link_libraries(super-tubi PRIVATE ${CMAKE_SOURCE_DIR}/common/detours.lib)

set_target_properties(super-tubi PROPERTIES OUTPUT_NAME "super-tubi-${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
