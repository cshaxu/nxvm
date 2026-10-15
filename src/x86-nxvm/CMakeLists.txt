cmake_minimum_required(VERSION 3.23)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(x86-nxvm LANGUAGES C)
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
if(NOT TARGET emulator-machine)
    add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../emulator" "${CMAKE_CURRENT_BINARY_DIR}/emulator")
endif()

add_library(x86-nxvm-product INTERFACE)
target_include_directories(x86-nxvm-product INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(x86-nxvm-product INTERFACE emulator-product types)

add_custom_target(x86-nxvm-verify
    COMMAND "${CMAKE_COMMAND}" -DCORPUS_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P "${CMAKE_CURRENT_SOURCE_DIR}/verify_manifest.cmake"
    COMMAND "${CMAKE_COMMAND}" -DCORPUS_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P "${CMAKE_CURRENT_SOURCE_DIR}/verify_corpus.cmake"
    VERBATIM)
