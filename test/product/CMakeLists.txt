cmake_minimum_required(VERSION 3.23)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(shared_product_tests LANGUAGES C)
    enable_testing()
endif()
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()
get_filename_component(PRODUCT_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../src/product" ABSOLUTE)
if(NOT TARGET product-surface)
    add_subdirectory("${PRODUCT_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}/product")
endif()
include("${CMAKE_CURRENT_LIST_DIR}/../register.cmake")

add_test(NAME product.test-boundaries COMMAND "${CMAKE_COMMAND}"
    "-DTEST_ROOT=${CMAKE_CURRENT_SOURCE_DIR}" "-DTEST_LAYER=product"
    -P "${CMAKE_CURRENT_LIST_DIR}/../verify_test_boundaries.cmake")
set_tests_properties(product.test-boundaries PROPERTIES LABELS "unit;product" TIMEOUT 30)
set(SHARED_PRODUCT_TEST_TARGETS)

shared_register_test(product debug_output product-debug)
shared_register_test(product debug_linear product-debug)
if(WIN32)
    shared_register_test(product debug_machine product-debug)
endif()
shared_register_test(product xasm32/xasm32 product-xasm32)
shared_register_test(product xasm32/xasm32_contract product-xasm32)
shared_register_test(product xasm32/xasm32_bounds product-xasm32)
shared_register_test(product surface/keyboard product-surface-keyboard)
add_executable(product-test-entry surface/entry_smoke.c
    "${PRODUCT_ROOT}/surface/entry.c")
target_link_libraries(product-test-entry PRIVATE types)
if(MSVC)
    target_compile_options(product-test-entry PRIVATE /UNDEBUG)
else()
    target_compile_options(product-test-entry PRIVATE -UNDEBUG)
endif()
set(product_entry_work_directory "${CMAKE_CURRENT_BINARY_DIR}/test/product-test-entry")
file(MAKE_DIRECTORY "${product_entry_work_directory}")
add_test(NAME product.surface/entry COMMAND product-test-entry)
set_tests_properties(product.surface/entry PROPERTIES LABELS "unit;product" TIMEOUT 30
    WORKING_DIRECTORY "${product_entry_work_directory}")
list(APPEND SHARED_PRODUCT_TEST_TARGETS product-test-entry)

add_executable(product-surface-command-smoke surface/command_smoke.c)
target_link_libraries(product-surface-command-smoke PRIVATE product-surface-command)
if(MINGW)
    target_link_options(product-surface-command-smoke PRIVATE -mconsole)
endif()
add_test(NAME product.surface/command COMMAND product-surface-command-smoke)
set_tests_properties(product.surface/command PROPERTIES LABELS "unit;product" TIMEOUT 30)
list(APPEND SHARED_PRODUCT_TEST_TARGETS product-surface-command-smoke)

add_test(NAME product.corpus COMMAND "${CMAKE_COMMAND}"
    "-DPRODUCT_ROOT=${PRODUCT_ROOT}" -P "${PRODUCT_ROOT}/verify_corpus.cmake")
set_tests_properties(product.corpus PROPERTIES LABELS "unit;product" TIMEOUT 30)
add_test(NAME product.corpus-negative COMMAND "${CMAKE_COMMAND}"
    "-DPRODUCT_ROOT=${PRODUCT_ROOT}" "-DPROBE_ROOT=${CMAKE_CURRENT_BINARY_DIR}/corpus-negative"
    -P "${CMAKE_CURRENT_LIST_DIR}/verify_negative.cmake")
set_tests_properties(product.corpus-negative PROPERTIES LABELS "unit;product" TIMEOUT 30)
add_test(NAME product.test-manifest COMMAND "${CMAKE_COMMAND}"
    "-DPRODUCT_ROOT=${CMAKE_CURRENT_SOURCE_DIR}"
    -P "${PRODUCT_ROOT}/verify_manifest.cmake")
set_tests_properties(product.test-manifest PROPERTIES LABELS "unit;product" TIMEOUT 30)
add_test(NAME product.test-types-boundary COMMAND "${CMAKE_COMMAND}"
    "-DTEST_ROOTS=${CMAKE_CURRENT_SOURCE_DIR}"
    -P "${CMAKE_CURRENT_LIST_DIR}/../verify_types_boundary.cmake")
set_tests_properties(product.test-types-boundary PROPERTIES LABELS "unit;product" TIMEOUT 30)
add_test(NAME product.manifest COMMAND "${CMAKE_COMMAND}"
    "-DPRODUCT_ROOT=${CMAKE_CURRENT_SOURCE_DIR}"
    -P "${PRODUCT_ROOT}/verify_manifest.cmake")
set_tests_properties(product.manifest PROPERTIES LABELS "unit;product" TIMEOUT 30)
shared_verify_test_registration(product)

foreach(target IN LISTS SHARED_PRODUCT_TEST_TARGETS)
    if(MSVC)
        target_compile_options(${target} PRIVATE /UNDEBUG)
    else()
        target_compile_options(${target} PRIVATE -UNDEBUG)
    endif()
endforeach()
add_custom_target(shared-product-tests DEPENDS ${SHARED_PRODUCT_TEST_TARGETS})
