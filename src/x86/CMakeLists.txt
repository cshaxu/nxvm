cmake_minimum_required(VERSION 3.23)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(nxvm_shared_x86 LANGUAGES C)
endif()
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()
get_filename_component(X86_COMMON_ROOT "${CMAKE_CURRENT_LIST_DIR}/../common" ABSOLUTE)
if(NOT TARGET types)
    add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../lib" "${CMAKE_CURRENT_BINARY_DIR}/lib")
endif()
option(X86_BUILD_TOOLS "Build xasm32 and Debug alongside device components" ON)
if(X86_BUILD_TOOLS AND NOT TARGET common-machine)
    add_subdirectory("${X86_COMMON_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}/common")
endif()

add_library(x86-product-config STATIC product/ini.c product/startup.c)
target_include_directories(x86-product-config PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(x86-product-config PUBLIC types storage base)

add_library(x86-pit825x STATIC chips/pit825x/pit.c)
target_include_directories(x86-pit825x PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(x86-pit825x PUBLIC types)

add_library(x86-rtc146818 STATIC chips/rtc146818/rtc.c)
target_include_directories(x86-rtc146818 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(x86-rtc146818 PUBLIC types)

add_library(x86-pic8259 STATIC chips/pic8259/pic.c)
target_include_directories(x86-pic8259 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(x86-pic8259 PUBLIC types)

add_library(x86-dma8237 STATIC chips/dma8237/dma.c)
target_include_directories(x86-dma8237 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(x86-dma8237 PUBLIC types)

add_library(x86-fdc8272 STATIC chips/fdc8272/fdc.c)
target_include_directories(x86-fdc8272 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(x86-fdc8272 PUBLIC types)

add_library(x86-hdc STATIC chips/hdc/hdc.c)
target_include_directories(x86-hdc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(x86-hdc PUBLIC types)

add_library(x86-fpu STATIC chips/fpu/fpu.c)
target_include_directories(x86-fpu PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(x86-fpu PUBLIC types)

add_library(x86-video STATIC chips/video/video.c)
target_include_directories(x86-video PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(x86-video PUBLIC types)

add_library(x86-ps2mouse STATIC chips/ps2mouse/mouse.c)
target_include_directories(x86-ps2mouse PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(x86-ps2mouse PUBLIC types)

add_library(x86-keyboard STATIC chips/keyboard/keyboard.c)
target_include_directories(x86-keyboard PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(x86-keyboard PUBLIC types)

add_library(x86-kbc8042 STATIC chips/kbc8042/controller.c)
target_include_directories(x86-kbc8042 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(x86-kbc8042 PUBLIC types PRIVATE x86-keyboard)

add_library(x86-ppi8255 STATIC chips/ppi8255/ppi.c)
target_include_directories(x86-ppi8255 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(x86-ppi8255 PUBLIC types)

add_library(x86-xtkeyboard STATIC chips/xtkeyboard/keyboard.c)
target_include_directories(x86-xtkeyboard PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(x86-xtkeyboard PUBLIC types)

# The canonical CPU corpus has one target, used by Core and independent tests.
add_library(x86-cpu-shared STATIC
    chips/cpu/cpu.c
    chips/cpu/cpu_instructions.c
    chips/cpu/cpu_timing.c
    chips/cpu/cpu_timing_model.c)
target_include_directories(x86-cpu-shared PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(x86-cpu-shared PUBLIC types PRIVATE x86-fpu)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    # The legacy instruction engine was deliberately compiled without the
    # shared strict-warning profile; its timing sources alone were strict.
    # Preserve that diagnostic contract; extraction does not change warnings.
    set_source_files_properties(
        chips/cpu/cpu.c
        chips/cpu/cpu_instructions.c
        PROPERTIES COMPILE_OPTIONS "-w")
endif()

set(x86_core_sources
    core/clock.c core/cpu_bus.c core/debug.c core/entry_plan_interface.c
    core/machine_firmware.c core/machine.c core/machine_scheduler.c
    core/memory_interface.c core/port_interface.c core/rom_mapping_interface.c
    core/trace_interface.c core/retirement_observation_interface.c
    core/timeline.c core/port.c core/memory.c core/transaction.c)
add_library(x86-core STATIC ${x86_core_sources})
target_include_directories(x86-core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(x86-core PUBLIC types x86-cpu-shared x86-fpu)
target_compile_definitions(x86-core PUBLIC
    CORE_MACHINE_RUNTIME_TRACE_ENABLED=$<IF:$<CONFIG:Debug>,1,0>)

# The same implementation with observation enabled is for trace-contract tests.
# Production consumers link x86-core, never both variants.
add_library(x86-core-observable STATIC EXCLUDE_FROM_ALL ${x86_core_sources})
target_include_directories(x86-core-observable PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(x86-core-observable PUBLIC types x86-cpu-shared x86-fpu)
target_compile_definitions(x86-core-observable PUBLIC
    CORE_MACHINE_RUNTIME_TRACE_ENABLED=1)

set(x86_ibmpc_common_sources
    ibmpc-common/media_interface.c ibmpc-common/display.c
    ibmpc-common/pit_bus.c ibmpc-common/pic_bus.c ibmpc-common/dma_bus.c
    ibmpc-common/fdc.c ibmpc-common/hdc.c ibmpc-common/vadp.c
    ibmpc-common/board_advance.c ibmpc-common/board_deadline.c
    ibmpc-common/machine_board.c ibmpc-common/machine_plan.c
    ibmpc-common/machine_display.c)
add_library(x86-ibmpc-common STATIC ${x86_ibmpc_common_sources})
target_include_directories(x86-ibmpc-common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
# Composition selects the Core implementation, including observation tests.
target_link_libraries(x86-ibmpc-common PUBLIC types x86-pit825x x86-pic8259
    x86-dma8237 x86-fdc8272 x86-hdc x86-video x86-rtc146818 x86-xtkeyboard
    x86-ibmpc-at x86-ibmpc-xt)
target_compile_definitions(x86-ibmpc-common PUBLIC
    CORE_MACHINE_RUNTIME_TRACE_ENABLED=$<IF:$<CONFIG:Debug>,1,0>)

# Trace-contract tests select this variant instead of the production board.
# Neither variant selects a Core implementation; composition owns that choice.
add_library(x86-ibmpc-common-observable STATIC EXCLUDE_FROM_ALL ${x86_ibmpc_common_sources})
target_include_directories(x86-ibmpc-common-observable PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(x86-ibmpc-common-observable PUBLIC types x86-pit825x x86-pic8259
    x86-dma8237 x86-fdc8272 x86-hdc x86-video x86-rtc146818 x86-xtkeyboard
    x86-ibmpc-at x86-ibmpc-xt)
target_compile_definitions(x86-ibmpc-common-observable PUBLIC CORE_MACHINE_RUNTIME_TRACE_ENABLED=1)

add_library(x86-ibmpc-at STATIC ibmpc-at/kbc.c ibmpc-at/parity.c)
target_include_directories(x86-ibmpc-at PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(x86-ibmpc-at PUBLIC types x86-kbc8042 x86-keyboard x86-ps2mouse x86-pit825x)

add_library(x86-ibmpc-xt STATIC ibmpc-xt/xt_ppi_keyboard.c)
target_include_directories(x86-ibmpc-xt PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(x86-ibmpc-xt PUBLIC types x86-ppi8255)

if(X86_BUILD_TOOLS)
add_library(x86-xasm32 STATIC xasm32/xasm32.c xasm32/aasm32.c xasm32/dasm32.c)
target_include_directories(x86-xasm32 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(x86-xasm32 PUBLIC types)

add_library(x86-debug STATIC debug/command.c)
target_include_directories(x86-debug PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(x86-debug PUBLIC common-machine x86-xasm32 storage)

add_library(x86-product STATIC product/command.c product/composition.c product/keyboard.c)
target_include_directories(x86-product PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(x86-product PUBLIC x86-product-config common-session common-ui x86-debug types)
endif()

add_custom_target(x86-verify
    COMMAND "${CMAKE_COMMAND}" -DCOMMON_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P "${X86_COMMON_ROOT}/verify_manifest.cmake"
    COMMAND "${CMAKE_COMMAND}" -DX86_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P "${CMAKE_CURRENT_SOURCE_DIR}/verify_corpus.cmake"
    VERBATIM)
