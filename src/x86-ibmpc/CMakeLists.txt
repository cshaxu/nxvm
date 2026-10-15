cmake_minimum_required(VERSION 3.23)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(x86-ibmpc LANGUAGES C)
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

if(NOT TARGET x86-debug-product)
    add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../x86-debug" "${CMAKE_CURRENT_BINARY_DIR}/x86-debug")
endif()
add_library(x86-ibmpc-product-keyboard STATIC product/keyboard.c)
target_include_directories(x86-ibmpc-product-keyboard PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(x86-ibmpc-product-keyboard PUBLIC emulator-machine emulator-session kvm-base types)
add_library(x86-ibmpc-product-command STATIC product/command.c product/command_provider.c)
target_include_directories(x86-ibmpc-product-command PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(x86-ibmpc-product-command PUBLIC x86-ibmpc-product-keyboard
    emulator-product emulator-session x86-debug-product types)
add_library(x86-ibmpc-product-entry STATIC product/entry.c)
target_include_directories(x86-ibmpc-product-entry PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(x86-ibmpc-product-entry PUBLIC emulator-product types)
add_library(x86-ibmpc-product INTERFACE)
target_link_libraries(x86-ibmpc-product INTERFACE x86-ibmpc-product-entry
    x86-ibmpc-product-command x86-ibmpc-product-keyboard emulator-product
    emulator-session emulator-ui x86-debug-product types)

add_custom_target(x86-ibmpc-verify
    COMMAND "${CMAKE_COMMAND}" -DCORPUS_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P "${CMAKE_CURRENT_SOURCE_DIR}/verify_manifest.cmake"
    COMMAND "${CMAKE_COMMAND}" -DCORPUS_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P "${CMAKE_CURRENT_SOURCE_DIR}/verify_corpus.cmake"
    VERBATIM)
