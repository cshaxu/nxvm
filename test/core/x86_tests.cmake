cmake_minimum_required(VERSION 3.23)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(core_x86_tests LANGUAGES C)
    enable_testing()
endif()
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()
get_filename_component(CORE_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../src/core" ABSOLUTE)
get_filename_component(X86_DEBUG_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../src/x86-debug" ABSOLUTE)
get_filename_component(CORE_TEST_REPOSITORY_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
if(NOT TARGET core-chip-pit825x)
    add_subdirectory("${CORE_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}/core")
endif()
if(NOT TARGET x86-debug-product)
    add_subdirectory("${X86_DEBUG_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}/x86-debug")
endif()

# Tests consume their own fixtures and inward production targets only.
set(CORE_X86_TEST_TARGETS)
include("${CMAKE_CURRENT_LIST_DIR}/../register.cmake")
set(CORE_TEST_ROOT "${CMAKE_CURRENT_LIST_DIR}")

if(NOT TARGET cpu-timing-manifest-catalog)
    set(CORE_CPU_TIMING_MANIFEST_METADATA_CATALOG
        "${CMAKE_BINARY_DIR}/generated/cpu_timing_manifest_metadata_catalog.inc")
    add_custom_command(
        OUTPUT "${CORE_CPU_TIMING_MANIFEST_METADATA_CATALOG}"
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoProfile -ExecutionPolicy Bypass
            -File "${CORE_TEST_REPOSITORY_ROOT}/tools/nxvm/Export-CpuTimingManifestCatalog.ps1"
            -OutPath "${CORE_CPU_TIMING_MANIFEST_METADATA_CATALOG}"
        DEPENDS "${CORE_TEST_REPOSITORY_ROOT}/tools/nxvm/Export-CpuTimingManifestCatalog.ps1"
            "${CORE_TEST_REPOSITORY_ROOT}/tools/nxvm/Verify-CpuTimingManifestContract.ps1"
            "${CORE_TEST_REPOSITORY_ROOT}/docs/nxvm/etc/cpu-timing/t435-s2-8086-timing-manifest.json"
            "${CORE_TEST_REPOSITORY_ROOT}/docs/nxvm/etc/cpu-timing/t512-s5-8088-timing-manifest.json"
            "${CORE_TEST_REPOSITORY_ROOT}/docs/nxvm/etc/cpu-timing/t435-s2-80186-timing-manifest.json"
            "${CORE_TEST_REPOSITORY_ROOT}/docs/nxvm/etc/cpu-timing/t435-s2-80286-timing-manifest.json"
            "${CORE_TEST_REPOSITORY_ROOT}/docs/nxvm/etc/cpu-timing/t435-s2-80386-timing-manifest.json"
        VERBATIM)
    add_custom_target(cpu-timing-manifest-catalog
        DEPENDS "${CORE_CPU_TIMING_MANIFEST_METADATA_CATALOG}")
endif()

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/generated/test-results")
add_executable(machine-8086-timing-manifest-runner
    ${CORE_TEST_ROOT}/board-base/qualification/machine_8086_timing_manifest_runner.c)
target_link_libraries(machine-8086-timing-manifest-runner PRIVATE x86-core-machine)
target_compile_definitions(machine-8086-timing-manifest-runner PRIVATE
    PROJECT_TEST_8086_RESULTS_PATH="${CMAKE_BINARY_DIR}/generated/test-results/8086-timing-results.json"
    PROJECT_TEST_8086_DECODER_INVENTORY_PATH="${CMAKE_BINARY_DIR}/generated/test-results/8086-decoder-inventory.json")
add_dependencies(machine-8086-timing-manifest-runner cpu-timing-manifest-catalog)
target_include_directories(machine-8086-timing-manifest-runner PRIVATE "${CMAKE_BINARY_DIR}/generated")

add_executable(machine-8088-timing-manifest-runner
    ${CORE_TEST_ROOT}/board-base/qualification/machine_8086_timing_manifest_runner.c)
target_link_libraries(machine-8088-timing-manifest-runner PRIVATE x86-core-machine)
target_compile_definitions(machine-8088-timing-manifest-runner PRIVATE
    PROJECT_TEST_TIMING_MANIFEST_CPU_PROFILE=X86_CORE_CPU_PROFILE_8088
    PROJECT_TEST_TIMING_MANIFEST_PROFILE_NAME="8088"
    PROJECT_TEST_TIMING_MANIFEST_KEY_PREFIX="I88-"
    PROJECT_TEST_TIMING_MANIFEST_RESULTS_PATH="${CMAKE_BINARY_DIR}/generated/test-results/8088-timing-results.json"
    PROJECT_TEST_TIMING_MANIFEST_DECODER_INVENTORY_PATH="${CMAKE_BINARY_DIR}/generated/test-results/8088-decoder-inventory.json")
add_dependencies(machine-8088-timing-manifest-runner cpu-timing-manifest-catalog)
target_include_directories(machine-8088-timing-manifest-runner PRIVATE "${CMAKE_BINARY_DIR}/generated")

add_executable(machine-80186-timing-manifest-runner
    ${CORE_TEST_ROOT}/board-base/qualification/machine_80186_timing_manifest_runner.c)
target_link_libraries(machine-80186-timing-manifest-runner PRIVATE x86-core-machine)
target_compile_definitions(machine-80186-timing-manifest-runner PRIVATE
    PROJECT_TEST_80186_RESULTS_PATH="${CMAKE_BINARY_DIR}/generated/test-results/80186-timing-results.json")
add_dependencies(machine-80186-timing-manifest-runner cpu-timing-manifest-catalog)
target_include_directories(machine-80186-timing-manifest-runner PRIVATE "${CMAKE_BINARY_DIR}/generated")

add_executable(machine-80286-timing-manifest-runner
    ${CORE_TEST_ROOT}/board-base/qualification/machine_80286_timing_manifest_runner.c)
target_link_libraries(machine-80286-timing-manifest-runner PRIVATE core-board-base x86-core-machine)
target_compile_definitions(machine-80286-timing-manifest-runner PRIVATE
    PROJECT_TEST_80286_RESULTS_PATH="${CMAKE_BINARY_DIR}/generated/test-results/80286-timing-results.json")
add_dependencies(machine-80286-timing-manifest-runner cpu-timing-manifest-catalog)
target_include_directories(machine-80286-timing-manifest-runner PRIVATE "${CMAKE_BINARY_DIR}/generated")

add_executable(machine-80386-timing-manifest-runner
    ${CORE_TEST_ROOT}/board-base/qualification/machine_80386_timing_manifest_runner.c)
target_link_libraries(machine-80386-timing-manifest-runner PRIVATE core-board-base-observable x86-core-machine-observable)
target_compile_definitions(machine-80386-timing-manifest-runner PRIVATE
    PROJECT_TEST_80386_RESULTS_PATH="${CMAKE_BINARY_DIR}/generated/test-results/80386-timing-results.json")
add_dependencies(machine-80386-timing-manifest-runner cpu-timing-manifest-catalog)
target_include_directories(machine-80386-timing-manifest-runner PRIVATE "${CMAKE_BINARY_DIR}/generated")


foreach(target IN ITEMS
    machine-8086-timing-manifest-runner
    machine-8088-timing-manifest-runner
    machine-80186-timing-manifest-runner
    machine-80286-timing-manifest-runner
    machine-80386-timing-manifest-runner)
    if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
    list(APPEND CORE_X86_TEST_TARGETS ${target})
endforeach()

foreach(target IN ITEMS
    machine-8086-timing-manifest-runner
    machine-8088-timing-manifest-runner
    machine-80186-timing-manifest-runner
    machine-80286-timing-manifest-runner
    machine-80386-timing-manifest-runner)
    add_test(NAME "unit.${target}" COMMAND ${target})
    set_tests_properties("unit.${target}" PROPERTIES
        LABELS "unit;core" TIMEOUT 30
        WORKING_DIRECTORY "${CORE_TEST_REPOSITORY_ROOT}")
endforeach()


if(POWERSHELL_EXECUTABLE)
    add_test(NAME unit.core-machine-8086-timing-results
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoProfile -ExecutionPolicy Bypass
            -File "${CORE_TEST_REPOSITORY_ROOT}/tools/nxvm/Verify-8086TimingResults.ps1"
            -ResultPath "${CMAKE_BINARY_DIR}/generated/test-results/8086-timing-results.json")
    set_tests_properties(unit.core-machine-8086-timing-results PROPERTIES
        DEPENDS "unit.machine-8086-timing-manifest-runner"
        LABELS "unit;core" TIMEOUT 30
        WORKING_DIRECTORY "${CORE_TEST_REPOSITORY_ROOT}")
    add_test(NAME unit.core-machine-8088-timing-results
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoProfile -ExecutionPolicy Bypass
            -File "${CORE_TEST_REPOSITORY_ROOT}/tools/nxvm/Verify-8088TimingResults.ps1"
            -BaseResultPath "${CMAKE_BINARY_DIR}/generated/test-results/8086-timing-results.json"
            -ResultPath "${CMAKE_BINARY_DIR}/generated/test-results/8088-timing-results.json")
    set_tests_properties(unit.core-machine-8088-timing-results PROPERTIES
        DEPENDS "unit.machine-8086-timing-manifest-runner;unit.machine-8088-timing-manifest-runner"
        LABELS "unit;core" TIMEOUT 30
        WORKING_DIRECTORY "${CORE_TEST_REPOSITORY_ROOT}")
    add_test(NAME unit.core-machine-8086-decoder-ledger
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoProfile -ExecutionPolicy Bypass
            -File "${CORE_TEST_REPOSITORY_ROOT}/tools/nxvm/Verify-8086DecoderLedger.ps1"
            -ResultPath "${CMAKE_BINARY_DIR}/generated/test-results/8086-timing-results.json")
    set_tests_properties(unit.core-machine-8086-decoder-ledger PROPERTIES
        DEPENDS "unit.machine-8086-timing-manifest-runner"
        LABELS "unit;core" TIMEOUT 30
        WORKING_DIRECTORY "${CORE_TEST_REPOSITORY_ROOT}")
endif()

shared_register_test(core chips/pit825x/pit_8253 core-chip-pit825x)
shared_register_test(core chips/pit825x/pit_readback core-chip-pit825x)
shared_register_test(core chips/pit825x/pit_waveform core-chip-pit825x)
shared_register_test(core chips/pit825x/pit_contract core-chip-pit825x)
shared_register_test(core chips/rtc146818/rtc_calendar core-chip-rtc146818)
shared_register_test(core chips/rtc146818/rtc_contract core-chip-rtc146818)
shared_register_test(core chips/pic8259/pic_contract core-chip-pic8259)
shared_register_test(core chips/pic8259/pic_commands core-chip-pic8259)
shared_register_test(core chips/dma8237/dma_contract core-chip-dma8237)
shared_register_test(core chips/fdc8272/fdc_contract core-chip-fdc8272)
shared_register_test(core chips/fdc8272/fdc_records core-chip-fdc8272)
shared_register_test(core chips/fdc8272/fdc_causes core-chip-fdc8272)
shared_register_test(core chips/fdc8272/fdc_allocation core-chip-fdc8272)
shared_register_test(core chips/hdc/hdc_contract core-chip-hdc)
shared_register_test(core chips/hdc/hdc_taskfile core-chip-hdc)
shared_register_test(core chips/hdc/hdc_allocation core-chip-hdc)
shared_register_test(core chips/video/video_contract core-chip-video)
shared_register_test(core chips/video/video_inspection core-chip-video)
shared_register_test(core chips/video/video_allocation core-chip-video)
shared_register_test(core chips/video/video_text core-chip-video)
shared_register_test(core chips/video/video_text_status core-chip-video)
shared_register_test(core chips/video/compaq_cecg_contract core-chip-video)
shared_register_test(core chips/video/compaq_cecg_feature_environment core-chip-video)
shared_register_test(core chips/video/compaq_cecg_io_base core-chip-video)
shared_register_test(core chips/video/compaq_cecg_input_status core-chip-video)
shared_register_test(core chips/video/ega_crtc_boundary_port core-chip-video)
shared_register_test(core chips/video/ega_sequencer core-chip-video)
shared_register_test(core chips/video/ega_controller core-chip-video)
shared_register_test(core chips/video/ega_external core-chip-video)
shared_register_test(core chips/video/cga_graphics core-chip-video)
shared_register_test(core chips/video/cga_640 core-chip-video)
shared_register_test(core chips/video/ega_mode10 core-chip-video)
shared_register_test(core chips/video/compaq_ega_personality core-chip-video)
shared_register_test(core chips/video/compaq_cecg_odd_even_page core-chip-video)
shared_register_test(core chips/video/ega_planar core-chip-video)
shared_register_test(core chips/ps2mouse/mouse_contract core-chip-ps2mouse)
shared_register_test(core chips/keyboard/keyboard_contract core-chip-keyboard)
shared_register_test(core chips/kbc8042/controller_contract "core-chip-kbc8042;core-chip-keyboard;core-chip-ps2mouse")
shared_register_test(core chips/ppi8255/ppi_contract core-chip-ppi8255)
shared_register_test(core chips/xtkeyboard/xt_keyboard_contract core-chip-xtkeyboard)
# One suite-owned build entry for importing products.  CTest registration and
# standalone configuration remain unchanged.
list(APPEND CORE_X86_TEST_TARGETS ${SHARED_CORE_TEST_TARGETS})
foreach(target IN LISTS CORE_X86_TEST_TARGETS)
    if(MSVC)
        target_compile_options(${target} PRIVATE /UNDEBUG)
    else()
        target_compile_options(${target} PRIVATE -UNDEBUG)
    endif()
endforeach()
add_custom_target(core-chip-tests DEPENDS ${CORE_X86_TEST_TARGETS})
