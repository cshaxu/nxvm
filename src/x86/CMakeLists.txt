cmake_minimum_required(VERSION 3.23)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(shared_x86 LANGUAGES C)
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

add_library(x86-xasm32 STATIC xasm32/xasm32.c xasm32/aasm32.c xasm32/dasm32.c)
target_include_directories(x86-xasm32 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(x86-xasm32 PUBLIC types)

add_library(x86-debug STATIC debug/command.c)
target_include_directories(x86-debug PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(x86-debug PUBLIC emulator-machine x86-xasm32 storage)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(x86-debug PRIVATE -Wno-error=format-truncation)
endif()

add_library(x86-product-keyboard STATIC product/keyboard.c)
target_include_directories(x86-product-keyboard PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(x86-product-keyboard PUBLIC emulator-machine emulator-session
    kvm-base types)

add_library(x86-product-command STATIC product/command.c product/command_provider.c)
target_include_directories(x86-product-command PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(x86-product-command PUBLIC x86-product-keyboard
    emulator-product emulator-session x86-debug types)

add_library(x86-product-entry STATIC product/entry.c)
target_include_directories(x86-product-entry PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(x86-product-entry PUBLIC emulator-product types)

add_library(x86-product INTERFACE)
target_link_libraries(x86-product INTERFACE x86-product-entry
    x86-product-command x86-product-keyboard emulator-product
    emulator-session emulator-ui x86-debug types)

add_custom_target(x86-verify
    COMMAND "${CMAKE_COMMAND}" -DX86_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P "${CMAKE_CURRENT_SOURCE_DIR}/verify_manifest.cmake"
    COMMAND "${CMAKE_COMMAND}" -DX86_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P "${CMAKE_CURRENT_SOURCE_DIR}/verify_corpus.cmake"
    VERBATIM)
