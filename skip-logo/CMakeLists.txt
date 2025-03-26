cmake_minimum_required(VERSION 3.26)
project(skip-logo VERSION 1.0.0)

add_library(skip-logo SHARED
        dllmain.cpp
        ${CMAKE_SOURCE_DIR}/common/memedit.cpp
        ${CMAKE_SOURCE_DIR}/common/hooker.cpp
        ${CMAKE_SOURCE_DIR}/common/logger.cpp
        ${CMAKE_SOURCE_DIR}/common/CWvsApp.cpp
        ${CMAKE_SOURCE_DIR}/common/CInputSystem.cpp
        ${CMAKE_SOURCE_DIR}/common/CStage.cpp
        ${CMAKE_SOURCE_DIR}/common/CLogo.cpp
)

# Set additional include directories if needed
target_include_directories(skip-logo PRIVATE ${CMAKE_SOURCE_DIR}/common)
target_link_libraries(skip-logo PRIVATE ${CMAKE_SOURCE_DIR}/common/detours.lib)
target_link_libraries(skip-logo PRIVATE build_config)

set_target_properties(skip-logo PROPERTIES OUTPUT_NAME "skip-logo-${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
