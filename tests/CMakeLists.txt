# Host-architecture unit tests. Driven by GoogleTest v1.14.0, fetched via
# FetchContent. Builds the extracted host-buildable surfaces directly —
# does NOT link common_lib (which is x86 and Win32-only).

include(FetchContent)
FetchContent_Declare(
    googletest
    URL      https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz
    URL_HASH SHA256=8ad598c73ad796e0d8280b082cebd82a630d73e73cd3c70057938a6501bba5d7
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(parse_ini_tests
    test_parse_ini.cpp
    ${CMAKE_SOURCE_DIR}/common/parse_ini.cpp)
target_include_directories(parse_ini_tests PRIVATE ${CMAKE_SOURCE_DIR}/common)
target_link_libraries(parse_ini_tests PRIVATE gtest_main)

add_executable(byte_ops_tests
    test_byte_ops.cpp
    ${CMAKE_SOURCE_DIR}/common/byte_ops.cpp)
target_include_directories(byte_ops_tests PRIVATE ${CMAKE_SOURCE_DIR}/common)
target_link_libraries(byte_ops_tests PRIVATE gtest_main)

add_executable(hotkey_registry_tests
    test_hotkey_registry.cpp
    ${CMAKE_SOURCE_DIR}/custom-ui-host/registries/hotkey_registry.cpp)
target_include_directories(hotkey_registry_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/custom-ui-host/registries
    ${CMAKE_SOURCE_DIR}/common)
target_link_libraries(hotkey_registry_tests PRIVATE gtest_main)

include(GoogleTest)
gtest_discover_tests(parse_ini_tests)
gtest_discover_tests(byte_ops_tests)
gtest_discover_tests(hotkey_registry_tests)
