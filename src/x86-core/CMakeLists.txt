cmake_minimum_required(VERSION 3.23)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(nxvm_shared_x86_core LANGUAGES C)
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

add_library(x86-core-cpu STATIC cpu/cpu.c cpu/cpu_instructions.c
    cpu/cpu_timing.c cpu/cpu_timing_model.c)
target_include_directories(x86-core-cpu PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_link_libraries(x86-core-cpu PUBLIC types PRIVATE x86-core-fpu)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    # Retain the existing instruction unit's narrow historical exceptions.
    set_source_files_properties(cpu/cpu_instructions.c PROPERTIES
        COMPILE_OPTIONS "-Wno-unused-parameter;-Wno-unused-variable;-Wno-unused-but-set-variable;-Wno-unused-function;-Wno-empty-body")
endif()
add_custom_target(x86-core-verify
    COMMAND "${CMAKE_COMMAND}" -DCORPUS_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P "${CMAKE_CURRENT_SOURCE_DIR}/verify_manifest.cmake"
    COMMAND "${CMAKE_COMMAND}" -DCORPUS_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P "${CMAKE_CURRENT_SOURCE_DIR}/verify_corpus.cmake"
    VERBATIM)
