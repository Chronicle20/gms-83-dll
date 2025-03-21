cmake_minimum_required(VERSION 3.26)
project(gms_83_dll)

set(CMAKE_CXX_STANDARD 14)

# Required build flags
set(BUILD_REGION "" CACHE STRING "Game region (e.g., GMS, JMS)")
set(BUILD_VERSION "" CACHE STRING "Game version (e.g., v83_1, v185_1)")

if (NOT BUILD_REGION OR NOT BUILD_VERSION)
    message(FATAL_ERROR "You must set both BUILD_REGION and BUILD_VERSION")
endif()

set(MEMORY_MAP_FILE "${CMAKE_SOURCE_DIR}/memory_maps/${BUILD_REGION}/${BUILD_VERSION}.cmake")

if (EXISTS ${MEMORY_MAP_FILE})
    message(STATUS "Using memory map: ${MEMORY_MAP_FILE}")
    include(${MEMORY_MAP_FILE})
else()
    message(FATAL_ERROR "Memory map not found at: ${MEMORY_MAP_FILE}")
endif()

# Generate memory_map.h
configure_file(
        ${CMAKE_SOURCE_DIR}/include/memory_map.h.in
        ${CMAKE_BINARY_DIR}/generated/memory_map.h
)
include_directories(${CMAKE_BINARY_DIR}/generated)

# Add subdirectories for each project
add_subdirectory(proxy)
add_subdirectory(bypass)
add_subdirectory(no-patcher)
add_subdirectory(redirect)
add_subdirectory(enable-minimize)
add_subdirectory(no-beginner-party-block)
add_subdirectory(no-enter-mts-map-restriction)
add_subdirectory(no-ad-balloon)
add_subdirectory(window-mode)

# Custom target to package DLLs into a tar archive
add_custom_target(package_dlls
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/artifacts"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/artifacts/edits"
        COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:proxy>" "${CMAKE_BINARY_DIR}/artifacts/"
        COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:bypass>" "${CMAKE_BINARY_DIR}/artifacts/edits/"
        COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:no-patcher>" "${CMAKE_BINARY_DIR}/artifacts/edits/"
        COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:redirect>" "${CMAKE_BINARY_DIR}/artifacts/edits/"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/redirect/redirect.ini" "${CMAKE_BINARY_DIR}/artifacts/edits/"
        COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:enable-minimize>" "${CMAKE_BINARY_DIR}/artifacts/edits/"
        COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:no-beginner-party-block>" "${CMAKE_BINARY_DIR}/artifacts/edits/"
        COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:no-enter-mts-map-restriction>" "${CMAKE_BINARY_DIR}/artifacts/edits/"
        COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:no-ad-balloon>" "${CMAKE_BINARY_DIR}/artifacts/edits/"
        COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:window-mode>" "${CMAKE_BINARY_DIR}/artifacts/edits/"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        COMMENT "Creating tar archive of DLL artifacts"
)