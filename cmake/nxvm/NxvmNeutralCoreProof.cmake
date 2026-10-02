# Test-only receiver of the actual production sources, before Shared relocation.
# Object linkage includes every candidate, even when the smoke does not call it.
get_target_property(neutral_runtime_sources core-machine SOURCES)
get_target_property(neutral_primitive_sources core-machine-executor SOURCES)
set(neutral_production_sources ${neutral_runtime_sources} ${neutral_primitive_sources})
set(neutral_core_sources)
foreach(unit IN ITEMS clock cpu_bus debug entry_plan_interface machine_firmware
    machine machine_scheduler memory_interface port_interface rom_mapping_interface
    trace_interface retirement_observation_interface timeline port memory transaction)
    set(source "src/app-nxvm/devices/${unit}.c")
    if(NOT source IN_LIST neutral_production_sources)
        message(FATAL_ERROR "Neutral proof source is not production-owned: ${source}")
    endif()
    list(APPEND neutral_core_sources "${source}")
endforeach()
add_library(core-machine-neutral-proof OBJECT ${neutral_core_sources})
target_include_directories(core-machine-neutral-proof PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(core-machine-neutral-proof PUBLIC x86-cpu x86-fpu types)
get_target_property(neutral_runtime_definitions core-machine COMPILE_DEFINITIONS)
target_compile_definitions(core-machine-neutral-proof PUBLIC ${neutral_runtime_definitions})
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(core-machine-neutral-proof PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(core-machine-neutral-link-smoke
    test/app-nxvm/unit/core/devices/core_machine_neutral_link_smoke.c)
target_link_libraries(core-machine-neutral-link-smoke PRIVATE core-machine-neutral-proof)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(core-machine-neutral-link-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()
get_target_property(neutral_proof_links core-machine-neutral-proof LINK_LIBRARIES)
if(NOT neutral_proof_links STREQUAL "x86-cpu;x86-fpu;types")
    message(FATAL_ERROR "Neutral proof must not inherit the PC board target")
endif()
get_target_property(neutral_smoke_links core-machine-neutral-link-smoke LINK_LIBRARIES)
if(NOT neutral_smoke_links STREQUAL "core-machine-neutral-proof")
    message(FATAL_ERROR "Neutral smoke must link only the independent receiving target")
endif()
