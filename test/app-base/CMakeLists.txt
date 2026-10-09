cmake_minimum_required(VERSION 3.23)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(app_base_product_tests LANGUAGES C)
    enable_testing()
endif()
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

get_filename_component(APP_BASE_SOURCE_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../src/app-base" ABSOLUTE)
if(NOT TARGET app-base-product)
    add_subdirectory("${APP_BASE_SOURCE_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}/app-base")
endif()
set(SHARED_APP-BASE_TEST_TARGETS)
include("${CMAKE_CURRENT_LIST_DIR}/../register.cmake")
shared_register_test(app-base product/ini app-base-product)
shared_register_test(app-base product/factory app-base-product)

add_test(NAME app-base.test-types-boundary COMMAND "${CMAKE_COMMAND}"
    "-DTEST_ROOTS=${CMAKE_CURRENT_LIST_DIR}"
    -P "${CMAKE_CURRENT_LIST_DIR}/../verify_types_boundary.cmake")
add_test(NAME app-base.test-ownership COMMAND "${CMAKE_COMMAND}"
    "-DTEST_ROOT=${CMAKE_CURRENT_LIST_DIR}" "-DTEST_LAYER=app-base"
    -P "${CMAKE_CURRENT_LIST_DIR}/../verify_test_boundaries.cmake")
add_custom_target(app-base-product-tests DEPENDS ${SHARED_APP-BASE_TEST_TARGETS})
