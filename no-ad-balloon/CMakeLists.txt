cmake_minimum_required(VERSION 3.26)
project(no-ad-balloon)

add_library(no-ad-balloon SHARED
        dllmain.cpp
        ${CMAKE_SOURCE_DIR}/common/memedit.cpp
)

# Set additional include directories if needed
target_include_directories(no-ad-balloon PRIVATE ${CMAKE_SOURCE_DIR}/common)
target_link_libraries(no-ad-balloon PRIVATE ${CMAKE_SOURCE_DIR}/common/detours.lib)
