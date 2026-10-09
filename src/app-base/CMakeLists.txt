cmake_minimum_required(VERSION 3.23)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(app_base_product_support LANGUAGES C)
endif()
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

get_filename_component(APP_BASE_SOURCE_ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
if(NOT TARGET ibmpc-machine)
    add_subdirectory("${APP_BASE_SOURCE_ROOT}/ibmpc" "${CMAKE_CURRENT_BINARY_DIR}/ibmpc")
endif()

add_library(app-base-product STATIC product/factory.c product/ini.c product/startup.c)
target_include_directories(app-base-product PUBLIC "${APP_BASE_SOURCE_ROOT}")
target_link_libraries(app-base-product PUBLIC ibmpc-machine ibmpc-product-composition
    types storage base)

add_custom_target(app-base-product-verify
    COMMAND "${CMAKE_COMMAND}" -DAPP_BASE_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P "${CMAKE_CURRENT_SOURCE_DIR}/verify_manifest.cmake"
    VERBATIM)
