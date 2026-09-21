cmake_minimum_required(VERSION 3.23)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(x86_corpus LANGUAGES C)
endif()
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()
get_filename_component(X86_COMMON_ROOT "${CMAKE_CURRENT_LIST_DIR}/../common" ABSOLUTE)
if(NOT TARGET types)
    add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../lib" "${CMAKE_CURRENT_BINARY_DIR}/lib")
endif()
if(NOT TARGET common-machine)
    add_subdirectory("${X86_COMMON_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}/common")
endif()

add_library(x86-xasm32 STATIC xasm32/xasm32.c xasm32/aasm32.c xasm32/dasm32.c)
target_include_directories(x86-xasm32 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(x86-xasm32 PUBLIC types)

add_library(x86-debug STATIC debug/command.c)
target_include_directories(x86-debug PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(x86-debug PUBLIC common-machine x86-xasm32 storage)

add_custom_target(x86-verify
    COMMAND "${CMAKE_COMMAND}" -DCOMMON_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P "${X86_COMMON_ROOT}/verify_manifest.cmake"
    COMMAND "${CMAKE_COMMAND}" -DX86_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P "${CMAKE_CURRENT_SOURCE_DIR}/verify_corpus.cmake"
    VERBATIM)
