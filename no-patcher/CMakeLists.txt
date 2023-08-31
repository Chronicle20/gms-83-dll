cmake_minimum_required(VERSION 3.26)
project(no-patcher)

add_library(no-patcher SHARED
        dllmain.cpp
        ${CMAKE_SOURCE_DIR}/common/memedit.cpp
)

# Set additional include directories if needed
target_include_directories(no-patcher PRIVATE ${CMAKE_SOURCE_DIR}/common)
target_link_libraries(no-patcher PRIVATE ${CMAKE_SOURCE_DIR}/common/detours.lib)
