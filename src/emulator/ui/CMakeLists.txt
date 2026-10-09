add_library(emulator-ui
    ui.c)
target_include_directories(emulator-ui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_link_libraries(emulator-ui PUBLIC
    console console-broker kvm-base kvm-window kvm-console)
