cmake_minimum_required(VERSION 3.23)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(nxvm_common LANGUAGES C)
    set(CMAKE_C_STANDARD 11)
    set(CMAKE_C_STANDARD_REQUIRED ON)
    set(CMAKE_C_EXTENSIONS OFF)
    include(CTest)
    add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../lib" "${CMAKE_BINARY_DIR}/lib")
endif()

set(COMMON_SOURCE_ROOT "${CMAKE_CURRENT_LIST_DIR}/..")

function(common_root target)
    target_include_directories(${target} PUBLIC "${COMMON_SOURCE_ROOT}")
endfunction()

add_library(common-machine STATIC machine/machine.c)
common_root(common-machine)
target_link_libraries(common-machine PUBLIC host)

add_library(common-xasm32 STATIC
    xasm32/xasm32.c
    xasm32/aasm32.c
    xasm32/dasm32.c)
common_root(common-xasm32)
target_link_libraries(common-xasm32 PUBLIC types)

add_library(common-debug STATIC
    debug/command.c
    debug/debug.c)
common_root(common-debug)
target_link_libraries(common-debug PUBLIC common-machine common-xasm32 storage)

add_library(common-session STATIC session/session.c)
common_root(common-session)
target_link_libraries(common-session PUBLIC host ui-base)

add_library(common-ui STATIC
    ui/console_host.c
    ui/presentation.c)
common_root(common-ui)
target_link_libraries(common-ui PUBLIC host ui-window ui-console ui-base)

add_custom_target(common-verify-corpus
    COMMAND "${CMAKE_COMMAND}"
        -DCOMMON_ROOT:PATH=${CMAKE_CURRENT_LIST_DIR}
        -P "${CMAKE_CURRENT_LIST_DIR}/verify_corpus.cmake"
    VERBATIM)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    add_dependencies(common-machine common-verify-corpus)
    add_dependencies(common-xasm32 common-verify-corpus)
    add_dependencies(common-debug common-verify-corpus)
    add_dependencies(common-session common-verify-corpus)
    add_dependencies(common-ui common-verify-corpus)
endif()
