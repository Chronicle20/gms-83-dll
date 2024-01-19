cmake_minimum_required(VERSION 3.26)
project(redirect VERSION 1.0.0)

add_library(redirect SHARED
        dllmain.cpp
        ${CMAKE_SOURCE_DIR}/common/Common.cpp
        ${CMAKE_SOURCE_DIR}/common/logger.cpp
        ${CMAKE_SOURCE_DIR}/common/winhooks.cpp
        ${CMAKE_SOURCE_DIR}/common/winhook_types.cpp
        ${CMAKE_SOURCE_DIR}/common/hooker.cpp
        ${CMAKE_SOURCE_DIR}/common/FakeModule.cpp
)

# Set additional include directories if needed
target_include_directories(redirect PRIVATE ${CMAKE_SOURCE_DIR}/common)
target_link_libraries(redirect PRIVATE ${CMAKE_SOURCE_DIR}/common/detours.lib)

set_target_properties(redirect PROPERTIES OUTPUT_NAME "redirect-${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

# Copy INI file to build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/redirect.ini DESTINATION ${CMAKE_BINARY_DIR}/redirect)