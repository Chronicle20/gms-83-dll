cmake_minimum_required(VERSION 3.26)
project(gms_83_dll)

set(CMAKE_CXX_STANDARD 14)

# Add subdirectories for each project
add_subdirectory(proxy)
add_subdirectory(bypass)
add_subdirectory(no-patcher)
add_subdirectory(redirect)
add_subdirectory(enable-minimize)
add_subdirectory(no-beginner-party-block)
add_subdirectory(no-enter-mts-map-restriction)
add_subdirectory(no-ad-balloon)

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
        COMMAND ${CMAKE_COMMAND} -E chdir "artifacts" tar "czf" "${CMAKE_BINARY_DIR}/artifacts.tar" .
        COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_BINARY_DIR}/artifacts"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        COMMENT "Creating tar archive of DLL artifacts"
)