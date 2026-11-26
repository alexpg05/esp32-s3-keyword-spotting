idf_component_register(INCLUDE_DIRS include)

add_prebuilt_library(tinykw_prebuilt "lib/en/libtinykw.a")
target_link_libraries(${COMPONENT_LIB} INTERFACE tinykw_prebuilt)
