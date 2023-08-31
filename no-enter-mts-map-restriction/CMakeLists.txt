cmake_minimum_required(VERSION 3.26)
project(no-enter-mts-map-restriction)

add_library(no-enter-mts-map-restriction SHARED
        dllmain.cpp
        ${CMAKE_SOURCE_DIR}/common/memedit.cpp
)

# Set additional include directories if needed
target_include_directories(no-enter-mts-map-restriction PRIVATE ${CMAKE_SOURCE_DIR}/common)
target_link_libraries(no-enter-mts-map-restriction PRIVATE ${CMAKE_SOURCE_DIR}/common/detours.lib)
