if (NOT (BUILD_REGION STREQUAL "GMS"
         AND BUILD_MAJOR_VERSION EQUAL 83
         AND BUILD_MINOR_VERSION EQUAL 1))
    message(STATUS
        "custom-ui-demo: not building for ${BUILD_REGION}_v${BUILD_MAJOR_VERSION}_${BUILD_MINOR_VERSION}")
    return()
endif()

add_edit_dll(custom-ui-demo SOURCES
    dllmain.cpp
    demo_main.cpp
    handlers.cpp
)

target_include_directories(custom-ui-demo PRIVATE
    ${CMAKE_SOURCE_DIR}/custom-ui-host
)
