# add_edit_dll(<name>
#     SOURCES <src1> [<src2> ...]
#     [VERSION <major>.<minor>.<patch>]   # default 1.0.0
#     [EXTRA_LIBS <lib1> [<lib2> ...]]    # extra PRIVATE link libs
#     [OUTPUT_NAME <name>]                # default <name>-<VERSION>
# )
#
# Defines a SHARED library target <name> for an "edit DLL". The target links
# common_lib + build_config PRIVATE, applies a versioned OUTPUT_NAME, and
# (optionally) any caller-supplied extra PRIVATE libs. Used for every edit
# DLL except proxy (which has no version suffix and ships to a different
# artifact path; see proxy/CMakeLists.txt).
function(add_edit_dll name)
    if (TARGET ${name})
        message(FATAL_ERROR
            "add_edit_dll: target '${name}' already exists. "
            "Helper does not silently shadow.")
    endif()

    set(options)
    set(oneValueArgs VERSION OUTPUT_NAME)
    set(multiValueArgs SOURCES EXTRA_LIBS)
    cmake_parse_arguments(EDIT
        "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if (NOT EDIT_SOURCES)
        message(FATAL_ERROR "add_edit_dll(${name}): SOURCES is required")
    endif()
    if (EDIT_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR
            "add_edit_dll(${name}): unknown arguments: ${EDIT_UNPARSED_ARGUMENTS}")
    endif()

    if (NOT EDIT_VERSION)
        set(EDIT_VERSION "1.0.0")
    endif()
    if (NOT EDIT_OUTPUT_NAME)
        set(EDIT_OUTPUT_NAME "${name}-${EDIT_VERSION}")
    endif()

    add_library(${name} SHARED ${EDIT_SOURCES})
    target_link_libraries(${name} PRIVATE common_lib build_config)
    if (EDIT_EXTRA_LIBS)
        target_link_libraries(${name} PRIVATE ${EDIT_EXTRA_LIBS})
    endif()
    set_target_properties(${name} PROPERTIES OUTPUT_NAME "${EDIT_OUTPUT_NAME}")
endfunction()
