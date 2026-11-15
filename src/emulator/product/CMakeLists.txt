# Compiler date/time describe this build, not a cached composition object.
add_custom_target(emulator-product-build-stamp
    COMMAND "${CMAKE_COMMAND}" -E touch "${CMAKE_CURRENT_BINARY_DIR}/build.stamp"
    BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/build.stamp"
    VERBATIM)
add_library(emulator-product-build OBJECT composition.c)
set_target_properties(emulator-product-build PROPERTIES C_COMPILER_LAUNCHER "")
set_source_files_properties(composition.c PROPERTIES
    OBJECT_DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/build.stamp")
add_dependencies(emulator-product-build emulator-product-build-stamp)
target_link_libraries(emulator-product-build PRIVATE emulator-session emulator-ui types)
add_library(emulator-product STATIC monitor.c $<TARGET_OBJECTS:emulator-product-build>)
target_include_directories(emulator-product PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_link_libraries(emulator-product PUBLIC emulator-session emulator-ui types)
