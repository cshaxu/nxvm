add_library(mynes-machine-driver STATIC
    driver.c
    media.c
    debug.c)
target_include_directories(mynes-machine-driver PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(mynes-machine-driver PUBLIC mynes-core-hardware emulator-machine storage audio)
mynes_enable_strict_warnings(mynes-machine-driver)
