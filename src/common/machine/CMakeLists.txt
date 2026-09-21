add_library(common-machine
    machine.c
    input_queue.c)
target_include_directories(common-machine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_link_libraries(common-machine PUBLIC base kvm-base kvm-window kvm-console storage)
