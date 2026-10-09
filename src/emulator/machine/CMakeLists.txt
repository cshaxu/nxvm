add_library(emulator-machine
    machine.c
    input_queue.c)
target_include_directories(emulator-machine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_link_libraries(emulator-machine PUBLIC base kvm-base kvm-window kvm-console storage)
