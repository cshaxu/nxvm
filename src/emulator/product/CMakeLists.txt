add_library(emulator-product STATIC composition.c monitor.c)
target_include_directories(emulator-product PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_link_libraries(emulator-product PUBLIC emulator-session emulator-ui types)
