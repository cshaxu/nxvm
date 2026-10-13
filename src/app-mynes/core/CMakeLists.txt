add_library(mynes-core-protocol STATIC opcode.c)
target_include_directories(mynes-core-protocol PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(mynes-core-protocol PUBLIC types)
mynes_enable_strict_warnings(mynes-core-protocol)

add_library(mynes-core-hardware STATIC
    apu.c
    cartridge.c
    controller.c
    snapshot.c
    ppu.c
    bus.c
    cpu.c
    machine.c)
target_include_directories(mynes-core-hardware PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(mynes-core-hardware PUBLIC mynes-core-protocol)
mynes_enable_strict_warnings(mynes-core-hardware)
