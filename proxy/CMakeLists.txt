cmake_minimum_required(VERSION 3.26)
project(proxy)

add_library(proxy SHARED
        ijl15.cpp
        dllmain.cpp
        ${CMAKE_SOURCE_DIR}/common/Common.cpp
        ${CMAKE_SOURCE_DIR}/common/logger.cpp
        ${CMAKE_SOURCE_DIR}/common/memedit.cpp
        ${CMAKE_SOURCE_DIR}/common/winhooks.cpp
        ${CMAKE_SOURCE_DIR}/common/winhook_types.cpp
        ${CMAKE_SOURCE_DIR}/common/hooker.cpp
        ${CMAKE_SOURCE_DIR}/common/FakeModule.cpp
)

# Set additional include directories if needed
target_include_directories(proxy PRIVATE ${CMAKE_SOURCE_DIR}/common)
target_link_libraries(proxy PRIVATE ${CMAKE_SOURCE_DIR}/common/detours.lib)

set_target_properties(proxy PROPERTIES OUTPUT_NAME "ijl15")