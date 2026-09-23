cmake_minimum_required(VERSION 3.23)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(nxvm_shared_common LANGUAGES C)
    set(COMMON_LIB_ROOT "" CACHE PATH "Explicit path to the shared Lib corpus")
    if(NOT EXISTS "${COMMON_LIB_ROOT}/CMakeLists.txt")
        message(FATAL_ERROR "COMMON_LIB_ROOT must name the shared Lib corpus")
    endif()
    add_subdirectory("${COMMON_LIB_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}/lib")
endif()
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

add_subdirectory(ui)
add_subdirectory(machine)
add_subdirectory(session)

add_custom_target(common-verify
    COMMAND "${CMAKE_COMMAND}" -DCOMMON_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P "${CMAKE_CURRENT_SOURCE_DIR}/verify_manifest.cmake"
    COMMAND "${CMAKE_COMMAND}" -DCOMMON_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P "${CMAKE_CURRENT_SOURCE_DIR}/verify_corpus.cmake"
    VERBATIM)
