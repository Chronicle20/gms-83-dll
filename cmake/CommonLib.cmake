# common_lib: a single OBJECT library carrying every translation unit under
# common/. Edit DLLs link this PRIVATE and rely on MSVC's /OPT:REF (Release
# default) to drop unreferenced object files at link time.
#
# detours.lib + the Win32 system libs are PUBLIC because OBJECT libraries
# only propagate PUBLIC/INTERFACE link deps to consumers; symbols referenced
# from common_lib's TUs (e.g. hooker.cpp uses Detours, winhooks.cpp uses
# Ws2_32) must reach the consuming edit DLL's link line.
#
# build_config is PRIVATE because it carries only compile defs and an include
# path — both bake into common_lib's .obj files at compile time. Consumers
# also link build_config themselves via add_edit_dll, so they don't need it
# propagated from here.
file(GLOB COMMON_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/common/*.cpp")

# pch.cpp and ZAllocEx.cpp are leftovers from the original Visual Studio
# project's PCH setup — neither was ever compiled by the pre-refactor CMake
# build, and neither is self-contained without /FI pch.h. ZAllocEx.cpp's
# global operator new/delete overrides are unreferenced by any edit DLL.
list(REMOVE_ITEM COMMON_SOURCES
    "${CMAKE_SOURCE_DIR}/common/pch.cpp"
    "${CMAKE_SOURCE_DIR}/common/ZAllocEx.cpp"
)

add_library(common_lib OBJECT ${COMMON_SOURCES})

target_include_directories(common_lib PUBLIC "${CMAKE_SOURCE_DIR}/common")

target_link_libraries(common_lib
    PRIVATE build_config
    PUBLIC  "${CMAKE_SOURCE_DIR}/common/detours.lib"
    PUBLIC  Ws2_32.lib
    PUBLIC  winmm.lib
    PUBLIC  comsuppw.lib
)
