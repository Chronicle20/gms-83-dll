cmake_minimum_required(VERSION 3.26)
project(redirect)

add_library(redirect SHARED
        dllmain.cpp
        ${CMAKE_SOURCE_DIR}/common/Common.cpp
        ${CMAKE_SOURCE_DIR}/common/Logger.cpp
        ${CMAKE_SOURCE_DIR}/common/winhooks.cpp
        ${CMAKE_SOURCE_DIR}/common/winhook_types.cpp
        ${CMAKE_SOURCE_DIR}/common/hooker.cpp
        ${CMAKE_SOURCE_DIR}/common/FakeModule.cpp
)

# Set additional include directories if needed
target_include_directories(redirect PRIVATE ${CMAKE_SOURCE_DIR}/common)
target_link_libraries(redirect PRIVATE ${CMAKE_SOURCE_DIR}/common/detours.lib)
