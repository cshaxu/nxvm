cmake_minimum_required(VERSION 3.23)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(nxvm_shared_ibmpc LANGUAGES C)
endif()
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()
get_filename_component(IBMPC_COMMON_ROOT "${CMAKE_CURRENT_LIST_DIR}/../common" ABSOLUTE)
if(NOT TARGET x86-core)
    add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../x86" "${CMAKE_CURRENT_BINARY_DIR}/x86")
endif()
if(NOT TARGET x86-debug)
    message(FATAL_ERROR "IBM PC Product requires the existing x86 Debug tools")
endif()

add_library(ibmpc-product-config STATIC product/ini.c product/startup.c)
target_include_directories(ibmpc-product-config PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(ibmpc-product-config PUBLIC types storage base)

add_library(ibmpc-machine-media STATIC machine/media/fdd.c
    machine/media/hdd.c)
target_include_directories(ibmpc-machine-media PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(ibmpc-machine-media PUBLIC types storage ibmpc-board-common)

add_library(ibmpc-machine-conversion STATIC machine/frame.c
    machine/keyboard_mapper.c machine/mouse_mapper.c)
target_include_directories(ibmpc-machine-conversion PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(ibmpc-machine-conversion PUBLIC types ibmpc-board-common)

add_library(ibmpc-machine STATIC machine/preparation.c machine/debug.c
    machine/executor_state.c
    machine/debug_adapter.c
    machine/display.c
    machine/lifecycle.c
    machine/machine.c
    machine/control.c
    machine/fault.c
    machine/runner.c
    machine/waiting.c
    machine/machine_devices.c
    machine/machine_info.c)
target_include_directories(ibmpc-machine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(ibmpc-machine PUBLIC types base storage
    common-machine ibmpc-machine-media ibmpc-machine-conversion
    ibmpc-board-common x86-core)

set(ibmpc_board_common_sources
    board-common/floppy.c board-common/rom_validation.c
    board-common/profile_contract.c
    board-common/media_interface.c board-common/display.c
    board-common/pit_bus.c board-common/pic_bus.c board-common/dma_bus.c
    board-common/fdc.c board-common/hdc.c board-common/vadp.c
    board-common/board_advance.c board-common/board_deadline.c
    board-common/machine_board.c board-common/machine_plan.c
    board-common/machine_display.c)
add_library(ibmpc-board-common STATIC ${ibmpc_board_common_sources})
target_include_directories(ibmpc-board-common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
# Composition selects the Core implementation, including observation tests.
target_link_libraries(ibmpc-board-common PUBLIC types x86-pit825x x86-pic8259
    x86-dma8237 x86-fdc8272 x86-hdc x86-video x86-rtc146818 x86-xtkeyboard
    ibmpc-board-at ibmpc-board-xt)
target_compile_definitions(ibmpc-board-common PUBLIC
    CORE_MACHINE_RUNTIME_TRACE_ENABLED=$<IF:$<CONFIG:Debug>,1,0>)

# Trace-contract tests select this variant instead of the production board.
# Neither variant selects a Core implementation; composition owns that choice.
add_library(ibmpc-board-common-observable STATIC EXCLUDE_FROM_ALL ${ibmpc_board_common_sources})
target_include_directories(ibmpc-board-common-observable PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(ibmpc-board-common-observable PUBLIC types x86-pit825x x86-pic8259
    x86-dma8237 x86-fdc8272 x86-hdc x86-video x86-rtc146818 x86-xtkeyboard
    ibmpc-board-at ibmpc-board-xt)
target_compile_definitions(ibmpc-board-common-observable PUBLIC CORE_MACHINE_RUNTIME_TRACE_ENABLED=1)

add_library(ibmpc-board-at STATIC board-at/kbc.c board-at/parity.c board-at/wiring.c)
target_include_directories(ibmpc-board-at PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(ibmpc-board-at PUBLIC types x86-kbc8042 x86-keyboard x86-ps2mouse x86-pit825x)

add_library(ibmpc-board-xt STATIC board-xt/xt_ppi_keyboard.c)
target_include_directories(ibmpc-board-xt PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(ibmpc-board-xt PUBLIC types x86-ppi8255)

add_library(ibmpc-product STATIC product/command.c product/composition.c product/keyboard.c
    product/entry.c product/factory.c)
target_include_directories(ibmpc-product PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(ibmpc-product PUBLIC ibmpc-product-config common-session common-ui x86-debug ibmpc-machine types)

add_custom_target(ibmpc-verify
    COMMAND "${CMAKE_COMMAND}" -DCOMMON_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P "${IBMPC_COMMON_ROOT}/verify_manifest.cmake"
    COMMAND "${CMAKE_COMMAND}" -DIBMPC_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P "${CMAKE_CURRENT_SOURCE_DIR}/verify_corpus.cmake"
    VERBATIM)
