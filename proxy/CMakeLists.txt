add_library(proxy SHARED
    ijl15.cpp
    dllmain.cpp)
target_link_libraries(proxy PRIVATE common_lib build_config)
set_target_properties(proxy PROPERTIES OUTPUT_NAME "ijl15")
