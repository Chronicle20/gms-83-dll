cmake_minimum_required(VERSION 3.26)
project(bypass)

add_library(bypass SHARED
        dllmain.cpp
        ${CMAKE_SOURCE_DIR}/common/memedit.cpp
        ${CMAKE_SOURCE_DIR}/common/hooker.cpp
        ${CMAKE_SOURCE_DIR}/common/Logger.cpp
)

# Set additional include directories if needed
target_include_directories(bypass PRIVATE ${CMAKE_SOURCE_DIR}/common)
target_link_libraries(bypass PRIVATE ${CMAKE_SOURCE_DIR}/common/detours.lib)
