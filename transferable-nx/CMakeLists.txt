cmake_minimum_required(VERSION 3.26)
project(transferable_nx)

add_library(transferable_nx SHARED
        dllmain.cpp
        ${CMAKE_SOURCE_DIR}/common/memedit.cpp
)

# Set additional include directories if needed
target_include_directories(transferable_nx PRIVATE ${CMAKE_SOURCE_DIR}/common)
target_link_libraries(transferable_nx PRIVATE ${CMAKE_SOURCE_DIR}/common/detours.lib)
