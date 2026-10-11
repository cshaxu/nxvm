cmake_minimum_required(VERSION 3.23)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(nxvm_core LANGUAGES C)
endif()
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

get_filename_component(CORE_SOURCE_ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
get_filename_component(CORE_EMULATOR_ROOT "${CORE_SOURCE_ROOT}/emulator" ABSOLUTE)
if(NOT TARGET types)
    add_subdirectory("${CORE_SOURCE_ROOT}/lib" "${CMAKE_CURRENT_BINARY_DIR}/lib")
endif()
if(NOT TARGET emulator-machine)
    add_subdirectory("${CORE_EMULATOR_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}/emulator")
endif()

function(core_add_chip target source)
    add_library(${target} STATIC ${source})
    target_include_directories(${target} PUBLIC ${CORE_SOURCE_ROOT})
    target_link_libraries(${target} PUBLIC types)
endfunction()

core_add_chip(core-chip-pit825x chips/pit825x/pit.c)
core_add_chip(core-chip-rtc146818 chips/rtc146818/rtc.c)
core_add_chip(core-chip-pic8259 chips/pic8259/pic.c)
core_add_chip(core-chip-dma8237 chips/dma8237/dma.c)
core_add_chip(core-chip-fdc8272 chips/fdc8272/fdc.c)
core_add_chip(core-chip-hdc chips/hdc/hdc.c)
core_add_chip(core-chip-fpu chips/fpu/fpu.c)
core_add_chip(core-chip-video chips/video/video.c)
core_add_chip(core-chip-ps2mouse chips/ps2mouse/mouse.c)
core_add_chip(core-chip-keyboard chips/keyboard/keyboard.c)
core_add_chip(core-chip-ppi8255 chips/ppi8255/ppi.c)
core_add_chip(core-chip-xtkeyboard chips/xtkeyboard/keyboard.c)
core_add_chip(core-chip-kbc8042 chips/kbc8042/controller.c)
target_link_libraries(core-chip-kbc8042 PRIVATE core-chip-keyboard)

add_library(core-chip-cpu STATIC
    chips/cpu/cpu.c
    chips/cpu/cpu_instructions.c
    chips/cpu/cpu_timing.c
    chips/cpu/cpu_timing_model.c)
target_include_directories(core-chip-cpu PUBLIC ${CORE_SOURCE_ROOT})
target_link_libraries(core-chip-cpu PUBLIC types PRIVATE core-chip-fpu)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set_source_files_properties(chips/cpu/cpu_instructions.c PROPERTIES
        COMPILE_OPTIONS "-Wno-unused-parameter;-Wno-unused-variable;-Wno-unused-but-set-variable;-Wno-unused-function;-Wno-empty-body")
endif()

set(core_x86_sources
    x86/clock.c x86/cpu_bus.c x86/debug.c x86/entry_plan_interface.c
    x86/machine_firmware.c x86/machine.c x86/machine_scheduler.c
    x86/memory_interface.c x86/port_interface.c x86/rom_mapping_interface.c
    x86/trace_interface.c x86/retirement_observation_interface.c
    x86/timeline.c x86/port.c x86/memory.c x86/transaction.c)
add_library(core-x86 STATIC ${core_x86_sources})
target_include_directories(core-x86 PUBLIC ${CORE_SOURCE_ROOT})
target_link_libraries(core-x86 PUBLIC types core-chip-cpu core-chip-fpu)
target_compile_definitions(core-x86 PUBLIC
    CORE_MACHINE_RUNTIME_TRACE_ENABLED=$<IF:$<CONFIG:Debug>,1,0>)

add_library(core-x86-observable STATIC EXCLUDE_FROM_ALL ${core_x86_sources})
target_include_directories(core-x86-observable PUBLIC ${CORE_SOURCE_ROOT})
target_link_libraries(core-x86-observable PUBLIC types core-chip-cpu core-chip-fpu)
target_compile_definitions(core-x86-observable PUBLIC CORE_MACHINE_RUNTIME_TRACE_ENABLED=1)

add_library(core-board-at STATIC board-at/kbc.c board-at/parity.c board-at/wiring.c)
target_include_directories(core-board-at PUBLIC ${CORE_SOURCE_ROOT})
target_link_libraries(core-board-at PUBLIC types core-chip-kbc8042 core-chip-keyboard
    core-chip-ps2mouse core-chip-pit825x)

add_library(core-board-xt STATIC board-xt/xt_ppi_keyboard.c)
target_include_directories(core-board-xt PUBLIC ${CORE_SOURCE_ROOT})
target_link_libraries(core-board-xt PUBLIC types core-chip-ppi8255)

set(core_board_base_sources
    board-base/floppy.c board-base/rom_validation.c board-base/rom_mapping.c
    board-base/pc_at_rom.c board-base/pc_at_profile.c
    board-base/profile_contract.c board-base/at_assembly.c
    board-base/media_interface.c board-base/display.c
    board-base/pit_bus.c board-base/pic_bus.c board-base/dma_bus.c
    board-base/fdc.c board-base/hdc.c board-base/vadp.c
    board-base/board_advance.c board-base/board_deadline.c
    board-base/machine_board.c board-base/machine_plan.c
    board-base/machine_display.c)
add_library(core-board-base STATIC ${core_board_base_sources})
target_include_directories(core-board-base PUBLIC ${CORE_SOURCE_ROOT})
target_link_libraries(core-board-base PUBLIC types core-chip-pit825x core-chip-pic8259
    core-chip-dma8237 core-chip-fdc8272 core-chip-hdc core-chip-video
    core-chip-rtc146818 core-chip-xtkeyboard core-board-at core-board-xt)
target_compile_definitions(core-board-base PUBLIC
    CORE_MACHINE_RUNTIME_TRACE_ENABLED=$<IF:$<CONFIG:Debug>,1,0>)

add_library(core-board-base-observable STATIC EXCLUDE_FROM_ALL ${core_board_base_sources})
target_include_directories(core-board-base-observable PUBLIC ${CORE_SOURCE_ROOT})
target_link_libraries(core-board-base-observable PUBLIC types core-chip-pit825x core-chip-pic8259
    core-chip-dma8237 core-chip-fdc8272 core-chip-hdc core-chip-video
    core-chip-rtc146818 core-chip-xtkeyboard core-board-at core-board-xt)
target_compile_definitions(core-board-base-observable PUBLIC CORE_MACHINE_RUNTIME_TRACE_ENABLED=1)

add_library(core-machine-media STATIC machine/media/fdd.c machine/media/hdd.c)
target_include_directories(core-machine-media PUBLIC ${CORE_SOURCE_ROOT})
target_link_libraries(core-machine-media PUBLIC types storage core-board-base)

add_library(core-machine-conversion STATIC machine/frame.c machine/keyboard_mapper.c
    machine/mouse_mapper.c)
target_include_directories(core-machine-conversion PUBLIC ${CORE_SOURCE_ROOT})
target_link_libraries(core-machine-conversion PUBLIC types kvm-base core-board-base)

add_library(core-machine STATIC machine/preparation.c machine/debug.c
    machine/pc_at_preparation.c machine/executor_state.c machine/debug_adapter.c
    machine/display.c machine/lifecycle.c machine/machine.c machine/control.c
    machine/fault.c machine/runner.c machine/waiting.c machine/machine_devices.c
    machine/machine_info.c)
target_include_directories(core-machine PUBLIC ${CORE_SOURCE_ROOT})
target_link_libraries(core-machine PUBLIC types base storage emulator-machine
    core-machine-media core-machine-conversion core-board-base core-x86)

add_custom_target(core-verify
    COMMAND "${CMAKE_COMMAND}" -DCORE_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P "${CMAKE_CURRENT_SOURCE_DIR}/verify_manifest.cmake"
    COMMAND "${CMAKE_COMMAND}" -DCORE_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P "${CMAKE_CURRENT_SOURCE_DIR}/verify_corpus.cmake"
    VERBATIM)
