add_edit_dll(redirect SOURCES dllmain.cpp)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/redirect.ini
     DESTINATION ${CMAKE_BINARY_DIR}/redirect)