cmake_minimum_required(VERSION 3.26)
project(no-patcher VERSION 1.0.0)

add_library(no-patcher SHARED
        dllmain.cpp
        ${CMAKE_SOURCE_DIR}/common/memedit.cpp
)

# Set additional include directories if needed
target_include_directories(no-patcher PRIVATE ${CMAKE_SOURCE_DIR}/common)
target_link_libraries(no-patcher PRIVATE ${CMAKE_SOURCE_DIR}/common/detours.lib)
target_link_libraries(no-patcher PRIVATE build_config)

set_target_properties(no-patcher PROPERTIES OUTPUT_NAME "no-patcher-${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
