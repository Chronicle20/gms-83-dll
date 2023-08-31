cmake_minimum_required(VERSION 3.26)
project(no-beginner-party-block)

add_library(no-beginner-party-block SHARED
        dllmain.cpp
        ${CMAKE_SOURCE_DIR}/common/memedit.cpp
)

# Set additional include directories if needed
target_include_directories(no-beginner-party-block PRIVATE ${CMAKE_SOURCE_DIR}/common)
target_link_libraries(no-beginner-party-block PRIVATE ${CMAKE_SOURCE_DIR}/common/detours.lib)
