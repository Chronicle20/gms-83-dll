cmake_minimum_required(VERSION 3.26)
project(enable-minimize)

add_library(enable-minimize SHARED
        dllmain.cpp
        ${CMAKE_SOURCE_DIR}/common/hooker.cpp
        ${CMAKE_SOURCE_DIR}/common/Logger.cpp
)

# Set additional include directories if needed
target_include_directories(enable-minimize PRIVATE ${CMAKE_SOURCE_DIR}/common)
target_link_libraries(enable-minimize PRIVATE ${CMAKE_SOURCE_DIR}/common/detours.lib)
