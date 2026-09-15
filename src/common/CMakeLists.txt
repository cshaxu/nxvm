cmake_minimum_required(VERSION 3.23)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(common_corpus LANGUAGES C)
    set(CMAKE_C_STANDARD 17)
    set(CMAKE_C_STANDARD_REQUIRED ON)
    set(CMAKE_C_EXTENSIONS OFF)
    set(COMMON_LIB_ROOT "" CACHE PATH "Explicit path to the shared Lib corpus")
    if(NOT EXISTS "${COMMON_LIB_ROOT}/CMakeLists.txt")
        message(FATAL_ERROR "COMMON_LIB_ROOT must name the shared Lib corpus")
    endif()
    add_subdirectory("${COMMON_LIB_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}/lib")
endif()

add_subdirectory(ui)
add_subdirectory(machine)
add_subdirectory(session)

add_library(common-xasm32 STATIC
    xasm32/xasm32.c
    xasm32/aasm32.c
    xasm32/dasm32.c)
target_include_directories(common-xasm32 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(common-xasm32 PUBLIC types)

add_library(common-debug STATIC
    debug/command.c)
target_include_directories(common-debug PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(common-debug PUBLIC common-machine common-xasm32 storage)

add_custom_target(common-verify
    COMMAND "${CMAKE_COMMAND}" -DCOMMON_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P "${CMAKE_CURRENT_SOURCE_DIR}/verify_manifest.cmake"
    COMMAND "${CMAKE_COMMAND}" -DCOMMON_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P "${CMAKE_CURRENT_SOURCE_DIR}/verify_corpus.cmake"
    VERBATIM)
