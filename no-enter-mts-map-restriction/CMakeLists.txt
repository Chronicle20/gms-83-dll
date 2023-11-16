cmake_minimum_required(VERSION 3.26)
project(no-enter-mts-map-restriction VERSION 1.0.0)

add_library(no-enter-mts-map-restriction SHARED
        dllmain.cpp
        ${CMAKE_SOURCE_DIR}/common/memedit.cpp
)

# Set additional include directories if needed
target_include_directories(no-enter-mts-map-restriction PRIVATE ${CMAKE_SOURCE_DIR}/common)
target_link_libraries(no-enter-mts-map-restriction PRIVATE ${CMAKE_SOURCE_DIR}/common/detours.lib)

set_target_properties(no-enter-mts-map-restriction PROPERTIES OUTPUT_NAME "no-enter-mts-map-restriction-${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
