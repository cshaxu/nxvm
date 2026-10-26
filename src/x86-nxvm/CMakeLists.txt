cmake_minimum_required(VERSION 3.23)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(shared-x86-nxvm LANGUAGES C)
endif()
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()
if(NOT TARGET types)
    add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../lib" "${CMAKE_CURRENT_BINARY_DIR}/lib")
endif()
if(NOT TARGET emulator-machine)
    add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../emulator" "${CMAKE_CURRENT_BINARY_DIR}/emulator")
endif()

get_filename_component(X86_NXVM_SOURCE_ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)

function(x86_nxvm_add_chip target source)
    add_library(${target} STATIC ${source})
    target_include_directories(${target} PUBLIC ${X86_NXVM_SOURCE_ROOT})
    target_link_libraries(${target} PUBLIC types)
endfunction()

x86_nxvm_add_chip(x86-nxvm-chip-pit825x chips/pit825x/pit.c)
x86_nxvm_add_chip(x86-nxvm-chip-rtc146818 chips/rtc146818/rtc.c)
x86_nxvm_add_chip(x86-nxvm-chip-pic8259 chips/pic8259/pic.c)
x86_nxvm_add_chip(x86-nxvm-chip-dma8237 chips/dma8237/dma.c)
x86_nxvm_add_chip(x86-nxvm-chip-fdc8272 chips/fdc8272/fdc.c)
x86_nxvm_add_chip(x86-nxvm-chip-hdc chips/hdc/hdc.c)
x86_nxvm_add_chip(x86-nxvm-chip-video chips/video/video.c)
x86_nxvm_add_chip(x86-nxvm-chip-f65535 chips/f65535/f65535.c)
target_link_libraries(x86-nxvm-chip-f65535 PUBLIC x86-nxvm-chip-video)
x86_nxvm_add_chip(x86-nxvm-chip-ps2mouse chips/ps2mouse/mouse.c)
x86_nxvm_add_chip(x86-nxvm-chip-keyboard chips/keyboard/keyboard.c)
x86_nxvm_add_chip(x86-nxvm-chip-ppi8255 chips/ppi8255/ppi.c)
x86_nxvm_add_chip(x86-nxvm-chip-xtkeyboard chips/xtkeyboard/keyboard.c)
x86_nxvm_add_chip(x86-nxvm-chip-kbc8042 chips/kbc8042/controller.c)
target_link_libraries(x86-nxvm-chip-kbc8042 PRIVATE x86-nxvm-chip-keyboard)


if(NOT TARGET x86-core-machine)
    add_subdirectory("${X86_NXVM_SOURCE_ROOT}/x86-core" "${CMAKE_CURRENT_BINARY_DIR}/x86-core")
endif()
if(NOT TARGET x86-ibmpc-product)
    add_subdirectory("${X86_NXVM_SOURCE_ROOT}/x86-ibmpc" "${CMAKE_CURRENT_BINARY_DIR}/x86-ibmpc")
endif()

add_library(x86-nxvm-board-at STATIC board-at/kbc.c board-at/parity.c board-at/wiring.c)
target_include_directories(x86-nxvm-board-at PUBLIC ${X86_NXVM_SOURCE_ROOT})
target_link_libraries(x86-nxvm-board-at PUBLIC types x86-nxvm-chip-kbc8042 x86-nxvm-chip-keyboard
    x86-nxvm-chip-ps2mouse x86-nxvm-chip-pit825x)

add_library(x86-nxvm-board-xt STATIC board-xt/xt_ppi_keyboard.c)
target_include_directories(x86-nxvm-board-xt PUBLIC ${X86_NXVM_SOURCE_ROOT})
target_link_libraries(x86-nxvm-board-xt PUBLIC types x86-nxvm-chip-ppi8255)

set(x86_nxvm_board_base_sources
    board-base/floppy.c board-base/rom_validation.c board-base/rom_mapping.c
    board-base/pc_at_rom.c board-base/pc_at_profile.c
    board-base/profile_contract.c board-base/at_assembly.c
    board-base/media_interface.c board-base/display.c
    board-base/pit_bus.c board-base/pic_bus.c board-base/dma_bus.c
    board-base/fdc.c board-base/hdc.c board-base/vadp.c
    board-base/board_advance.c board-base/board_deadline.c
    board-base/machine_board.c board-base/machine_plan.c
    board-base/machine_display.c)
add_library(x86-nxvm-board-base STATIC ${x86_nxvm_board_base_sources})
target_include_directories(x86-nxvm-board-base PUBLIC ${X86_NXVM_SOURCE_ROOT})
target_link_libraries(x86-nxvm-board-base PUBLIC types x86-nxvm-chip-pit825x x86-nxvm-chip-pic8259
    x86-nxvm-chip-dma8237 x86-nxvm-chip-fdc8272 x86-nxvm-chip-hdc x86-nxvm-chip-video
    x86-nxvm-chip-rtc146818 x86-nxvm-chip-xtkeyboard x86-nxvm-board-at x86-nxvm-board-xt)
target_link_libraries(x86-nxvm-board-base PUBLIC x86-core-machine)
target_compile_definitions(x86-nxvm-board-base PUBLIC
    X86_CORE_MACHINE_RUNTIME_TRACE_ENABLED=$<IF:$<CONFIG:Debug>,1,0>)

add_library(x86-nxvm-board-base-observable STATIC EXCLUDE_FROM_ALL ${x86_nxvm_board_base_sources})
target_include_directories(x86-nxvm-board-base-observable PUBLIC ${X86_NXVM_SOURCE_ROOT})
target_link_libraries(x86-nxvm-board-base-observable PUBLIC types x86-nxvm-chip-pit825x x86-nxvm-chip-pic8259
    x86-nxvm-chip-dma8237 x86-nxvm-chip-fdc8272 x86-nxvm-chip-hdc x86-nxvm-chip-video
    x86-nxvm-chip-rtc146818 x86-nxvm-chip-xtkeyboard x86-nxvm-board-at x86-nxvm-board-xt)
target_link_libraries(x86-nxvm-board-base-observable PUBLIC x86-core-machine-observable)
target_compile_definitions(x86-nxvm-board-base-observable PUBLIC X86_CORE_MACHINE_RUNTIME_TRACE_ENABLED=1)

add_library(x86-nxvm-machine-media STATIC machine/media/fdd.c machine/media/hdd.c)
target_include_directories(x86-nxvm-machine-media PUBLIC ${X86_NXVM_SOURCE_ROOT})
target_link_libraries(x86-nxvm-machine-media PUBLIC types storage x86-nxvm-board-base)

add_library(x86-nxvm-machine-conversion STATIC machine/frame.c machine/keyboard_mapper.c
    machine/mouse_mapper.c)
target_include_directories(x86-nxvm-machine-conversion PUBLIC ${X86_NXVM_SOURCE_ROOT})
target_link_libraries(x86-nxvm-machine-conversion PUBLIC types kvm-base kvm-window
    kvm-console x86-nxvm-board-base)

add_library(x86-nxvm-machine STATIC machine/preparation.c machine/debug.c
    machine/pc_at_preparation.c machine/executor_state.c machine/debug_adapter.c
    machine/display.c machine/lifecycle.c machine/machine.c machine/control.c
    machine/fault.c machine/runner.c machine/waiting.c machine/machine_devices.c
    machine/machine_info.c)
target_include_directories(x86-nxvm-machine PUBLIC ${X86_NXVM_SOURCE_ROOT})
target_link_libraries(x86-nxvm-machine PUBLIC types base storage kvm-base emulator-machine
    x86-nxvm-machine-media x86-nxvm-machine-conversion x86-nxvm-board-base x86-core-machine)

add_library(x86-nxvm-product STATIC product/factory.c product/ini.c product/startup.c)
target_include_directories(x86-nxvm-product PUBLIC ${X86_NXVM_SOURCE_ROOT})
target_link_libraries(x86-nxvm-product PUBLIC x86-nxvm-machine x86-ibmpc-product emulator-product types storage base)

add_custom_target(x86-nxvm-verify
    COMMAND "${CMAKE_COMMAND}" -DCORPUS_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P "${CMAKE_CURRENT_SOURCE_DIR}/verify_manifest.cmake"
    COMMAND "${CMAKE_COMMAND}" -DCORPUS_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P "${CMAKE_CURRENT_SOURCE_DIR}/verify_corpus.cmake"
    VERBATIM)
