cmake_minimum_required(VERSION 3.23)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(shared-x86-core LANGUAGES C)
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

add_library(x86-core-fpu STATIC fpu/fpu.c)
target_include_directories(x86-core-fpu PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_link_libraries(x86-core-fpu PUBLIC types)

add_library(x86-core-cpu STATIC cpu/cpu.c cpu/cpu_bus.c cpu/cpu_instructions.c
    cpu/cpu_timing.c cpu/cpu_timing_model.c)
target_include_directories(x86-core-cpu PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_link_libraries(x86-core-cpu PUBLIC types PRIVATE x86-core-fpu)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    # Retain the existing instruction unit's narrow historical exceptions.
    set_source_files_properties(cpu/cpu_instructions.c PROPERTIES
        COMPILE_OPTIONS "-Wno-unused-parameter;-Wno-unused-variable;-Wno-unused-but-set-variable;-Wno-unused-function;-Wno-empty-body")
endif()
set(x86_core_machine_sources
    machine/clock.c machine/cpu_bus.c machine/debug.c machine/entry_plan_interface.c
    machine/machine_firmware.c machine/machine.c machine/machine_scheduler.c
    machine/memory_interface.c machine/port_interface.c machine/rom_mapping_interface.c
    machine/trace_interface.c machine/retirement_observation_interface.c
    machine/timeline.c machine/port.c machine/memory.c machine/transaction.c)
add_library(x86-core-machine STATIC ${x86_core_machine_sources})
target_include_directories(x86-core-machine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_link_libraries(x86-core-machine PUBLIC types x86-core-cpu x86-core-fpu)
target_compile_definitions(x86-core-machine PUBLIC
    X86_CORE_MACHINE_RUNTIME_TRACE_ENABLED=$<IF:$<CONFIG:Debug>,1,0>)

add_library(x86-core-machine-observable STATIC EXCLUDE_FROM_ALL ${x86_core_machine_sources})
target_include_directories(x86-core-machine-observable PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_link_libraries(x86-core-machine-observable PUBLIC types x86-core-cpu x86-core-fpu)
target_compile_definitions(x86-core-machine-observable PUBLIC
    X86_CORE_MACHINE_RUNTIME_TRACE_ENABLED=1)

add_custom_target(x86-core-verify
    COMMAND "${CMAKE_COMMAND}" -DCORPUS_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P "${CMAKE_CURRENT_SOURCE_DIR}/verify_manifest.cmake"
    COMMAND "${CMAKE_COMMAND}" -DCORPUS_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P "${CMAKE_CURRENT_SOURCE_DIR}/verify_corpus.cmake"
    VERBATIM)
