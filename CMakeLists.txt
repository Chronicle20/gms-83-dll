cmake_minimum_required(VERSION 3.26)
project(gms_83_dll)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(AddEditDll)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Required build flags
set(BUILD_REGION "" CACHE STRING "Game region (e.g., GMS, JMS)")
set(BUILD_MAJOR_VERSION "" CACHE STRING "Game version (e.g., 83, 185)")
set(BUILD_MINOR_VERSION "" CACHE STRING "Game version (e.g., 1, 2)")

# Validate
if (NOT BUILD_REGION OR NOT BUILD_MAJOR_VERSION OR NOT BUILD_MINOR_VERSION)
    message(FATAL_ERROR "You must set all BUILD_REGION, BUILD_MAJOR_VERSION, BUILD_MINOR_VERSION")
endif()

# Include region+version-specific memory map
set(MEMORY_MAP_FILE "${CMAKE_SOURCE_DIR}/memory_maps/${BUILD_REGION}/v${BUILD_MAJOR_VERSION}_${BUILD_MINOR_VERSION}.cmake")
if (EXISTS ${MEMORY_MAP_FILE})
    message(STATUS "Using memory map: ${MEMORY_MAP_FILE}")
    include(${MEMORY_MAP_FILE})
else()
    message(FATAL_ERROR "Memory map not found at: ${MEMORY_MAP_FILE}")
endif()

# Generate memory_map.h (with validation that every @KEY@ has a defined
# non-empty CMake var; see cmake/GenerateMemoryMap.cmake).
include(GenerateMemoryMap)

# Define shared INTERFACE config for all subprojects
add_library(build_config INTERFACE)

# Set include path
target_include_directories(build_config INTERFACE
        ${CMAKE_BINARY_DIR}/generated
)

# Global compile definitions
target_compile_definitions(build_config INTERFACE
        BUILD_REGION="${BUILD_REGION}"
        BUILD_MAJOR_VERSION=${BUILD_MAJOR_VERSION}
        BUILD_MINOR_VERSION=${BUILD_MINOR_VERSION}
)

# Compile-time log threshold. Debug build: everything (Trace=0).
# Anything else (Release/RelWithDebInfo/MinSizeRel): Warn=3 and above only.
target_compile_definitions(build_config INTERFACE
    $<$<CONFIG:Debug>:LOG_MIN_LEVEL=0>
    $<$<NOT:$<CONFIG:Debug>>:LOG_MIN_LEVEL=3>
)

# Region macro for conditional compilation
if (BUILD_REGION STREQUAL "GMS")
    target_compile_definitions(build_config INTERFACE REGION_GMS)
elseif (BUILD_REGION STREQUAL "JMS")
    target_compile_definitions(build_config INTERFACE REGION_JMS)
else()
    message(FATAL_ERROR "Unsupported region: ${BUILD_REGION}")
endif()

include(CommonLib)

# Add subdirectories for each project
add_subdirectory(bypass)
add_subdirectory(doom-fix)
add_subdirectory(enable-minimize)
add_subdirectory(no-patcher)
add_subdirectory(no-beginner-party-block)
add_subdirectory(no-enter-mts-map-restriction)
add_subdirectory(no-ad-balloon)
add_subdirectory(proxy)
add_subdirectory(redirect)
add_subdirectory(skip-logo)
add_subdirectory(window-mode)
add_subdirectory(custom-ui-host)
add_subdirectory(custom-ui-demo)

# Custom target to package DLLs into the artifacts/ layout
set(EDIT_DLL_TARGETS
    bypass
    doom-fix
    enable-minimize
    no-patcher
    no-beginner-party-block
    no-enter-mts-map-restriction
    no-ad-balloon
    redirect
    skip-logo
    window-mode
    custom-ui-host
    custom-ui-demo
)

set(PACKAGE_COMMANDS
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/artifacts"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/artifacts/edits"
)
foreach(t ${EDIT_DLL_TARGETS})
    if (TARGET ${t})
        list(APPEND PACKAGE_COMMANDS
            COMMAND ${CMAKE_COMMAND} -E copy
                "$<TARGET_FILE:${t}>" "${CMAKE_BINARY_DIR}/artifacts/edits/"
        )
    endif()
endforeach()
list(APPEND PACKAGE_COMMANDS
    COMMAND ${CMAKE_COMMAND} -E copy
        "$<TARGET_FILE:proxy>" "${CMAKE_BINARY_DIR}/artifacts/"
    COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_BINARY_DIR}/redirect/redirect.ini"
        "${CMAKE_BINARY_DIR}/artifacts/edits/"
)

# Custom UI host / demo INIs (only when those edits actually built).
if (TARGET custom-ui-host)
    list(APPEND PACKAGE_COMMANDS
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/custom-ui-host/custom-ui-host.ini"
            "${CMAKE_BINARY_DIR}/artifacts/edits/"
    )
endif()
if (TARGET custom-ui-demo)
    list(APPEND PACKAGE_COMMANDS
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/custom-ui-demo/demo.ini"
            "${CMAKE_BINARY_DIR}/artifacts/edits/custom-ui-demo.ini"
    )
endif()

add_custom_target(package_dlls
    ${PACKAGE_COMMANDS}
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    COMMENT "Packaging DLL artifacts"
)

option(BUILD_TESTS "Build host-side unit tests" OFF)
if (BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()