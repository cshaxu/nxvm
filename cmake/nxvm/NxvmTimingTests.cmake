# Repository evidence runners depend on the NXVM manual ledger and tools.
# They are not part of the independently configurable component test package.
set(X86_NXVM_TEST_REPOSITORY_ROOT "${PROJECT_SOURCE_DIR}")
set(X86_NXVM_TEST_ROOT "${PROJECT_SOURCE_DIR}/test/x86-nxvm")
set(X86_NXVM_X86_TEST_TARGETS)
if(NOT TARGET cpu-timing-manifest-catalog)
    set(X86_NXVM_CPU_TIMING_MANIFEST_METADATA_CATALOG
        "${CMAKE_BINARY_DIR}/generated/cpu_timing_manifest_metadata_catalog.inc")
    add_custom_command(
        OUTPUT "${X86_NXVM_CPU_TIMING_MANIFEST_METADATA_CATALOG}"
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoProfile -ExecutionPolicy Bypass
            -File "${X86_NXVM_TEST_REPOSITORY_ROOT}/tools/nxvm/Export-CpuTimingManifestCatalog.ps1"
            -OutPath "${X86_NXVM_CPU_TIMING_MANIFEST_METADATA_CATALOG}"
        DEPENDS "${X86_NXVM_TEST_REPOSITORY_ROOT}/tools/nxvm/Export-CpuTimingManifestCatalog.ps1"
            "${X86_NXVM_TEST_REPOSITORY_ROOT}/tools/nxvm/Verify-CpuTimingManifestContract.ps1"
            "${X86_NXVM_TEST_REPOSITORY_ROOT}/docs/nxvm/etc/cpu-timing/t435-s2-8086-timing-manifest.json"
            "${X86_NXVM_TEST_REPOSITORY_ROOT}/docs/nxvm/etc/cpu-timing/t512-s5-8088-timing-manifest.json"
            "${X86_NXVM_TEST_REPOSITORY_ROOT}/docs/nxvm/etc/cpu-timing/t435-s2-80186-timing-manifest.json"
            "${X86_NXVM_TEST_REPOSITORY_ROOT}/docs/nxvm/etc/cpu-timing/t435-s2-80286-timing-manifest.json"
            "${X86_NXVM_TEST_REPOSITORY_ROOT}/docs/nxvm/etc/cpu-timing/t435-s2-80386-timing-manifest.json"
        VERBATIM)
    add_custom_target(cpu-timing-manifest-catalog
        DEPENDS "${X86_NXVM_CPU_TIMING_MANIFEST_METADATA_CATALOG}")
endif()

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/generated/test-results")
add_executable(machine-8086-timing-manifest-runner
    ${X86_NXVM_TEST_ROOT}/board-base/qualification/machine_8086_timing_manifest_runner.c)
target_link_libraries(machine-8086-timing-manifest-runner PRIVATE x86-core-machine)
target_compile_definitions(machine-8086-timing-manifest-runner PRIVATE
    PROJECT_TEST_8086_RESULTS_PATH="${CMAKE_BINARY_DIR}/generated/test-results/8086-timing-results.json"
    PROJECT_TEST_8086_DECODER_INVENTORY_PATH="${CMAKE_BINARY_DIR}/generated/test-results/8086-decoder-inventory.json")
add_dependencies(machine-8086-timing-manifest-runner cpu-timing-manifest-catalog)
target_include_directories(machine-8086-timing-manifest-runner PRIVATE "${CMAKE_BINARY_DIR}/generated")

add_executable(machine-8088-timing-manifest-runner
    ${X86_NXVM_TEST_ROOT}/board-base/qualification/machine_8086_timing_manifest_runner.c)
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
    ${X86_NXVM_TEST_ROOT}/board-base/qualification/machine_80186_timing_manifest_runner.c)
target_link_libraries(machine-80186-timing-manifest-runner PRIVATE x86-core-machine)
target_compile_definitions(machine-80186-timing-manifest-runner PRIVATE
    PROJECT_TEST_80186_RESULTS_PATH="${CMAKE_BINARY_DIR}/generated/test-results/80186-timing-results.json")
add_dependencies(machine-80186-timing-manifest-runner cpu-timing-manifest-catalog)
target_include_directories(machine-80186-timing-manifest-runner PRIVATE "${CMAKE_BINARY_DIR}/generated")

add_executable(machine-80286-timing-manifest-runner
    ${X86_NXVM_TEST_ROOT}/board-base/qualification/machine_80286_timing_manifest_runner.c)
target_link_libraries(machine-80286-timing-manifest-runner PRIVATE x86-nxvm-board-base x86-core-machine)
target_compile_definitions(machine-80286-timing-manifest-runner PRIVATE
    PROJECT_TEST_80286_RESULTS_PATH="${CMAKE_BINARY_DIR}/generated/test-results/80286-timing-results.json")
add_dependencies(machine-80286-timing-manifest-runner cpu-timing-manifest-catalog)
target_include_directories(machine-80286-timing-manifest-runner PRIVATE "${CMAKE_BINARY_DIR}/generated")

add_executable(machine-80386-timing-manifest-runner
    ${X86_NXVM_TEST_ROOT}/board-base/qualification/machine_80386_timing_manifest_runner.c)
target_link_libraries(machine-80386-timing-manifest-runner PRIVATE x86-nxvm-board-base-observable x86-core-machine-observable)
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
    list(APPEND X86_NXVM_X86_TEST_TARGETS ${target})
endforeach()

foreach(target IN ITEMS
    machine-8086-timing-manifest-runner
    machine-8088-timing-manifest-runner
    machine-80186-timing-manifest-runner
    machine-80286-timing-manifest-runner
    machine-80386-timing-manifest-runner)
    add_test(NAME "unit.${target}" COMMAND ${target})
    set_tests_properties("unit.${target}" PROPERTIES
        LABELS "unit;x86-nxvm" TIMEOUT 30
        WORKING_DIRECTORY "${X86_NXVM_TEST_REPOSITORY_ROOT}")
endforeach()


if(POWERSHELL_EXECUTABLE)
    add_test(NAME unit.core-machine-8086-timing-results
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoProfile -ExecutionPolicy Bypass
            -File "${X86_NXVM_TEST_REPOSITORY_ROOT}/tools/nxvm/Verify-8086TimingResults.ps1"
            -ResultPath "${CMAKE_BINARY_DIR}/generated/test-results/8086-timing-results.json")
    set_tests_properties(unit.core-machine-8086-timing-results PROPERTIES
        DEPENDS "unit.machine-8086-timing-manifest-runner"
        LABELS "unit;x86-nxvm" TIMEOUT 30
        WORKING_DIRECTORY "${X86_NXVM_TEST_REPOSITORY_ROOT}")
    add_test(NAME unit.core-machine-8088-timing-results
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoProfile -ExecutionPolicy Bypass
            -File "${X86_NXVM_TEST_REPOSITORY_ROOT}/tools/nxvm/Verify-8088TimingResults.ps1"
            -BaseResultPath "${CMAKE_BINARY_DIR}/generated/test-results/8086-timing-results.json"
            -ResultPath "${CMAKE_BINARY_DIR}/generated/test-results/8088-timing-results.json")
    set_tests_properties(unit.core-machine-8088-timing-results PROPERTIES
        DEPENDS "unit.machine-8086-timing-manifest-runner;unit.machine-8088-timing-manifest-runner"
        LABELS "unit;x86-nxvm" TIMEOUT 30
        WORKING_DIRECTORY "${X86_NXVM_TEST_REPOSITORY_ROOT}")
    add_test(NAME unit.core-machine-8086-decoder-ledger
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoProfile -ExecutionPolicy Bypass
            -File "${X86_NXVM_TEST_REPOSITORY_ROOT}/tools/nxvm/Verify-8086DecoderLedger.ps1"
            -ResultPath "${CMAKE_BINARY_DIR}/generated/test-results/8086-timing-results.json")
    set_tests_properties(unit.core-machine-8086-decoder-ledger PROPERTIES
        DEPENDS "unit.machine-8086-timing-manifest-runner"
        LABELS "unit;x86-nxvm" TIMEOUT 30
        WORKING_DIRECTORY "${X86_NXVM_TEST_REPOSITORY_ROOT}")
endif()

# Repository-owned qualification build entry.

foreach(target IN LISTS X86_NXVM_X86_TEST_TARGETS)
    if(MSVC)
        target_compile_options(${target} PRIVATE /UNDEBUG)
    else()
        target_compile_options(${target} PRIVATE -UNDEBUG)
    endif()
endforeach()
add_custom_target(x86-nxvm-timing-tests DEPENDS ${X86_NXVM_X86_TEST_TARGETS})
