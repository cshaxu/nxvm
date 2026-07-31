cmake_minimum_required(VERSION 3.23)

# M0 validates the selected C toolchain. M1 adds the imported baseline target;
# M3 later adds the refactored machine target after M2 design approval.
project(ntvdm64 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

if(NOT CMAKE_C_COMPILER_ID MATCHES "^(GNU|MSVC|Clang)$")
    message(FATAL_ERROR "ntvdm64 requires GCC, MSVC, or Clang C11 support.")
endif()

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "ntvdm64 requires a 64-bit Windows C toolchain.")
endif()

find_program(POWERSHELL_EXECUTABLE NAMES powershell pwsh REQUIRED)

set(NTVDM64_PROBE_DIR "${CMAKE_BINARY_DIR}/probes")

add_library(nxvm-core STATIC
    src/core/cpu.c
    src/core/debug.c
    src/core/machine.c
    src/core/memory.c
    src/core/port.c
    src/core/trace.c
)
target_include_directories(nxvm-core PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)

add_executable(nxvm-core-contract-smoke
    tests/core/core_contract_smoke.c
)
target_link_libraries(nxvm-core-contract-smoke PRIVATE nxvm-core)

add_executable(nxvm-core-machine-instance-smoke
    tests/core/machine_instance_smoke.c
)
target_link_libraries(nxvm-core-machine-instance-smoke PRIVATE nxvm-core)

add_executable(nxvm-core-machine-lifecycle-smoke
    tests/core/machine_lifecycle_smoke.c
)
target_link_libraries(nxvm-core-machine-lifecycle-smoke PRIVATE nxvm-core)

add_executable(nxvm-core-machine-trace-smoke
    tests/core/machine_trace_smoke.c
)
target_link_libraries(nxvm-core-machine-trace-smoke PRIVATE nxvm-core)

add_executable(nxvm-core-machine-debug-smoke
    tests/core/machine_debug_smoke.c
)
target_link_libraries(nxvm-core-machine-debug-smoke PRIVATE nxvm-core)

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(nxvm-core PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-core-contract-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-core-machine-instance-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-core-machine-lifecycle-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-core-machine-trace-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-core-machine-debug-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

add_custom_target(generate-m1-probe
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${NTVDM64_PROBE_DIR}"
    COMMAND "${POWERSHELL_EXECUTABLE}" -NoProfile -ExecutionPolicy Bypass
        -File "${CMAKE_SOURCE_DIR}/tools/New-DosProbe.ps1"
        -OutputDirectory "${NTVDM64_PROBE_DIR}"
        -Name "m1-text-exit"
        -Marker "NTVDM64:M1:TEXT:OK"
        -ExitCode 42
    BYPRODUCTS
        "${NTVDM64_PROBE_DIR}/m1-text-exit.com"
        "${NTVDM64_PROBE_DIR}/m1-text-exit.json"
    COMMENT "Generating deterministic M1 DOS COM probe"
    VERBATIM
)

# M1 retains the imported source tree as a behavior baseline. The Linux source
# remains present for future portability work but is not part of this Windows
# executable target.
file(GLOB_RECURSE NXVM_BASELINE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/nxvm-baseline/*.c"
)
list(FILTER NXVM_BASELINE_SOURCES EXCLUDE REGEX ".*/platform/linux/.*")

set(NXVM_BASELINE_RUNTIME_SOURCES ${NXVM_BASELINE_SOURCES})
list(FILTER NXVM_BASELINE_RUNTIME_SOURCES EXCLUDE REGEX ".*/main\\.c$")

add_library(nxvm-baseline-runtime STATIC ${NXVM_BASELINE_RUNTIME_SOURCES})
target_link_libraries(nxvm-baseline-runtime PUBLIC user32 gdi32)

add_library(nxvm-baseline-full-pc-adapter STATIC
    src/adapters/nxvm_baseline/full_pc_profile.c
)
target_include_directories(nxvm-baseline-full-pc-adapter PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(nxvm-baseline-full-pc-adapter PUBLIC nxvm-baseline-runtime)

add_library(nxvm-runtime STATIC
    src/runtime/dos_minimal.c
    src/runtime/profile.c
)
target_include_directories(nxvm-runtime PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(nxvm-runtime PUBLIC nxvm-core)

add_library(nxvm-dos-minimal-presentation-adapter STATIC
    src/adapters/dos_minimal_presentation.c
)
target_include_directories(nxvm-dos-minimal-presentation-adapter PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(nxvm-dos-minimal-presentation-adapter PUBLIC nxvm-runtime)

add_executable(nxvm-dos-minimal-presentation-smoke
    tests/adapters/dos_minimal_presentation_smoke.c
)
target_link_libraries(nxvm-dos-minimal-presentation-smoke PRIVATE
    nxvm-dos-minimal-presentation-adapter
)

add_executable(nxvm-dos-minimal-profile-smoke
    tests/runtime/dos_minimal_profile_smoke.c
)
target_link_libraries(nxvm-dos-minimal-profile-smoke PRIVATE nxvm-runtime)

add_executable(nxvm-full-pc-profile-smoke
    tests/adapters/nxvm_full_pc_profile_smoke.c
)
target_link_libraries(nxvm-full-pc-profile-smoke PRIVATE
    nxvm-runtime
    nxvm-baseline-full-pc-adapter
)

add_executable(nxvm-baseline ${NXVM_BASELINE_SOURCES})
target_link_libraries(nxvm-baseline PRIVATE user32 gdi32)

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(nxvm-baseline PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(nxvm-runtime PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-dos-minimal-profile-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-dos-minimal-presentation-adapter PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-dos-minimal-presentation-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-baseline-full-pc-adapter PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-full-pc-profile-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
