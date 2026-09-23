cmake_minimum_required(VERSION 3.23)

# The root owns only the toolchain contract and neutral shared corpora.
# Products select their own CMake graph below; no product cache setting is a
# prerequisite for another product's configure path.
project(nxvm LANGUAGES C)

enable_testing()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

if(NOT CMAKE_C_COMPILER_ID MATCHES "^(GNU|MSVC|Clang)$")
    message(FATAL_ERROR "This repository requires GCC, MSVC, or Clang C11 support.")
endif()

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 4 AND NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "This repository requires a 32-bit or 64-bit C toolchain.")
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(project_artifact_architecture "x64")
else()
    set(project_artifact_architecture "x86")
endif()
if(DEFINED PROJECT_ARTIFACT_ARCHITECTURE AND
   NOT PROJECT_ARTIFACT_ARCHITECTURE STREQUAL "${project_artifact_architecture}")
    message(FATAL_ERROR
        "PROJECT_ARTIFACT_ARCHITECTURE does not match the selected C toolchain.")
endif()
set(PROJECT_ARTIFACT_ARCHITECTURE "${project_artifact_architecture}")

find_program(POWERSHELL_EXECUTABLE NAMES powershell pwsh)
if(NOT WIN32)
    add_compile_definitions(_POSIX_C_SOURCE=200809L _DEFAULT_SOURCE=1)
endif()

option(REPOSITORY_BUILD_NXVM "Configure NXVM product targets" ON)
option(REPOSITORY_BUILD_MYNES "Configure MyNES product targets" ON)

add_subdirectory(src/lib)
add_subdirectory(src/common)
add_subdirectory(src/x86)
add_subdirectory(test/lib)
add_subdirectory(test/common)
add_subdirectory(test/x86)

# The frozen upstream x86 corpus is strict under GNU/Clang. GCC 16 also emits
# a bounded-format false positive for two retained Debug display buffers.
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(x86-debug PRIVATE -Wno-error=format-truncation)
    target_compile_options(x86-test-debug_output PRIVATE -Wno-error=format-truncation)
    target_compile_options(x86-test-debug_linear PRIVATE -Wno-error=format-truncation)
    target_link_options(lib-test-kvm_window_capture_contract PRIVATE
        -Wl,--stack,8388608)
elseif(MSVC)
    target_link_options(lib-test-kvm_window_capture_contract PRIVATE /STACK:8388608)
endif()

if(REPOSITORY_BUILD_NXVM)
    include(cmake/nxvm/NxvmProduct.cmake)
endif()

if(REPOSITORY_BUILD_MYNES)
    include(cmake/mynes/MyNesProduct.cmake)
endif()
