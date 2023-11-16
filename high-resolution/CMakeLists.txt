cmake_minimum_required(VERSION 3.26)
project(high-resolution)

add_library(high-resolution SHARED
        dllmain.cpp
        Client.cpp
        ${CMAKE_SOURCE_DIR}/common/memedit.cpp
)

# Set additional include directories if needed
target_include_directories(high-resolution PRIVATE ${CMAKE_SOURCE_DIR}/common)
target_link_libraries(high-resolution PRIVATE ${CMAKE_SOURCE_DIR}/common/detours.lib)
