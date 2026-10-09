cmake_minimum_required(VERSION 3.23)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(shared_product LANGUAGES C)
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
if(NOT TARGET common-machine)
    add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../common" "${CMAKE_CURRENT_BINARY_DIR}/common")
endif()

add_library(product-xasm32 STATIC xasm32/xasm32.c xasm32/aasm32.c xasm32/dasm32.c)
target_include_directories(product-xasm32 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(product-xasm32 PUBLIC types)

add_library(product-debug STATIC debug/command.c)
target_include_directories(product-debug PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(product-debug PUBLIC common-machine product-xasm32 storage)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(product-debug PRIVATE -Wno-error=format-truncation)
endif()

add_library(product-surface-keyboard STATIC surface/keyboard.c)
target_include_directories(product-surface-keyboard PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(product-surface-keyboard PUBLIC common-machine common-session
    kvm-base types)

add_library(product-surface-command STATIC surface/command.c surface/command_provider.c)
target_include_directories(product-surface-command PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(product-surface-command PUBLIC product-surface-keyboard
    common-session product-debug types)

add_library(product-surface-composition STATIC surface/composition.c)
target_include_directories(product-surface-composition PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(product-surface-composition PUBLIC common-session common-ui types)

add_library(product-surface-entry STATIC surface/entry.c)
target_include_directories(product-surface-entry PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)

add_library(product-surface INTERFACE)
target_link_libraries(product-surface INTERFACE product-surface-entry
    product-surface-command product-surface-composition product-surface-keyboard
    common-session common-ui product-debug types)

add_custom_target(product-verify
    COMMAND "${CMAKE_COMMAND}" -DPRODUCT_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P "${CMAKE_CURRENT_SOURCE_DIR}/verify_manifest.cmake"
    COMMAND "${CMAKE_COMMAND}" -DPRODUCT_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P "${CMAKE_CURRENT_SOURCE_DIR}/verify_corpus.cmake"
    VERBATIM)
