# custom-ui-host: reusable in-DLL UI framework. Hosts the C ABI, installs
# the three Detours hooks, and owns the window/hotkey/packet registries.
# Compile-gated to GMS v83.1 only for this milestone; non-target builds
# silently return so the CI matrix keeps passing across every region.
if (NOT (BUILD_REGION STREQUAL "GMS"
         AND BUILD_MAJOR_VERSION EQUAL 83
         AND BUILD_MINOR_VERSION EQUAL 1))
    message(STATUS
        "custom-ui-host: not building for ${BUILD_REGION}_v${BUILD_MAJOR_VERSION}_${BUILD_MINOR_VERSION}")
    return()
endif()

# OUTPUT_NAME is the STABLE, unversioned "custom-ui-host.dll" — consumer edits
# resolve it by that literal name (LoadLibrary("custom-ui-host.dll") +
# GetProcAddress). ABI versioning is exposed via CustomUI_GetAbiVersion(), not
# the filename, so the host must NOT carry the default "-1.0.0" suffix.
add_edit_dll(custom-ui-host OUTPUT_NAME custom-ui-host SOURCES
    dllmain.cpp
    host_main.cpp
    abi/custom_ui_abi.cpp
    hooks/process_packet_hook.cpp
    hooks/process_key_hook.cpp
    hooks/stage_dtor_hook.cpp
    hooks/s_update_hook.cpp
    registries/window_registry.cpp
    registries/hotkey_registry.cpp
    registries/packet_registry.cpp
    runtime/custom_ui_wnd.cpp
    runtime/vtable_patch.cpp
    runtime/text_draw.cpp
    runtime/ini_config.cpp
)

# The public ABI header is part of the source tree; downstream edits
# include it directly (no install step yet).
target_include_directories(custom-ui-host PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
