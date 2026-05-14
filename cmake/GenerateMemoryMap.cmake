# GenerateMemoryMap.cmake
#
# Parses include/memory_map.h.in for every @KEY@ placeholder, verifies the
# matching CMake variable is defined and non-empty, and only then calls
# configure_file to emit the final generated/memory_map.h. If any keys are
# missing, emits a single FATAL_ERROR listing every missing key so a port
# author can fix them all in one round trip.

set(_MMI_INFILE  "${CMAKE_SOURCE_DIR}/include/memory_map.h.in")
set(_MMI_OUTFILE "${CMAKE_BINARY_DIR}/generated/memory_map.h")

if (NOT EXISTS "${_MMI_INFILE}")
    message(FATAL_ERROR "GenerateMemoryMap: input file not found: ${_MMI_INFILE}")
endif()

file(STRINGS "${_MMI_INFILE}" _MMI_LINES)

set(_MMI_KEYS "")
foreach(_LINE IN LISTS _MMI_LINES)
    string(REGEX MATCHALL "@([A-Z0-9_]+)@" _MATCHES "${_LINE}")
    foreach(_M IN LISTS _MATCHES)
        string(REGEX REPLACE "^@(.*)@$" "\\1" _K "${_M}")
        list(APPEND _MMI_KEYS "${_K}")
    endforeach()
endforeach()
list(REMOVE_DUPLICATES _MMI_KEYS)

set(_MMI_MISSING "")
foreach(_K IN LISTS _MMI_KEYS)
    if (NOT DEFINED ${_K})
        list(APPEND _MMI_MISSING "${_K}")
    elseif ("${${_K}}" STREQUAL "")
        list(APPEND _MMI_MISSING "${_K}")
    endif()
endforeach()

if (_MMI_MISSING)
    string(REPLACE ";" "\n  " _MMI_MISSING_JOINED "${_MMI_MISSING}")
    message(FATAL_ERROR
        "Memory map for ${BUILD_REGION} v${BUILD_MAJOR_VERSION}.${BUILD_MINOR_VERSION} "
        "is missing required keys:\n  ${_MMI_MISSING_JOINED}\n"
        "Add the missing set() calls to "
        "memory_maps/${BUILD_REGION}/v${BUILD_MAJOR_VERSION}_${BUILD_MINOR_VERSION}.cmake.")
endif()

configure_file("${_MMI_INFILE}" "${_MMI_OUTFILE}" @ONLY)
