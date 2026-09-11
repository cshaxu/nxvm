cmake_minimum_required(VERSION 3.23)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(nxvm_shared_library LANGUAGES C)
    set(CMAKE_C_STANDARD 11)
    set(CMAKE_C_STANDARD_REQUIRED ON)
    set(CMAKE_C_EXTENSIONS OFF)
    include(CTest)
endif()

set(LIBRARY_SOURCE_ROOT "${CMAKE_CURRENT_LIST_DIR}/..")

set(LIBRARY_STRICT_WARNINGS_DEFAULT OFF)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(LIBRARY_STRICT_WARNINGS_DEFAULT ON)
endif()
option(LIBRARY_STRICT_WARNINGS
    "Build shared-library implementation targets with strict compiler warnings"
    ${LIBRARY_STRICT_WARNINGS_DEFAULT})

function(library_root target)
    get_target_property(target_type ${target} TYPE)
    if(target_type STREQUAL "INTERFACE_LIBRARY")
        target_include_directories(${target} INTERFACE "${LIBRARY_SOURCE_ROOT}")
    else()
        target_include_directories(${target} PUBLIC "${LIBRARY_SOURCE_ROOT}")
    endif()
endfunction()

add_library(types INTERFACE)
library_root(types)

add_library(console STATIC console/console.c)
library_root(console)
target_link_libraries(console PUBLIC types)

set(LIBRARY_UI_BASE_SOURCES
    ui-base/component.c
    ui-base/hotkey.c
    ui-base/mailbox.c
    ui-base/linux/input.c)
if(WIN32)
    list(APPEND LIBRARY_UI_BASE_SOURCES
        ui-base/win32/mailbox.c
        ui-base/win32/color.c
        ui-base/win32/actions.c
        ui-base/win32/input.c)
else()
    list(APPEND LIBRARY_UI_BASE_SOURCES ui-base/linux/mailbox.c)
endif()
add_library(ui-base STATIC ${LIBRARY_UI_BASE_SOURCES})
library_root(ui-base)
target_link_libraries(ui-base PUBLIC types)

add_library(ui-window STATIC ui-window/window.c)
if(WIN32)
    target_sources(ui-window PRIVATE ui-window/win32/geometry.c
        ui-window/win32/mouse.c ui-window/win32/component.c)
else()
    target_sources(ui-window PRIVATE ui-window/linux/component.c)
endif()
library_root(ui-window)
target_link_libraries(ui-window PUBLIC types ui-base)

add_library(ui-console STATIC ui-console/console.c)
if(WIN32)
    target_sources(ui-console PRIVATE ui-console/win32/component.c)
else()
    target_sources(ui-console PRIVATE ui-console/linux/component.c)
endif()
library_root(ui-console)
target_link_libraries(ui-console PUBLIC types console ui-base)

if(WIN32)
    set(LIBRARY_HOST_SOURCES host/clock.c host/console.c host/win32/clock.c host/win32/sync.c host/win32/console.c)
else()
    set(LIBRARY_HOST_SOURCES host/clock.c host/console.c host/linux/clock.c host/linux/sync.c host/linux/console.c)
endif()
add_library(host STATIC ${LIBRARY_HOST_SOURCES})
library_root(host)
target_link_libraries(host PUBLIC types console)

set(LIBRARY_STORAGE_SOURCES storage/file.c storage/medium.c)
if(WIN32)
    list(APPEND LIBRARY_STORAGE_SOURCES storage/win32/file.c)
else()
    list(APPEND LIBRARY_STORAGE_SOURCES storage/linux/file.c)
endif()
add_library(storage STATIC ${LIBRARY_STORAGE_SOURCES})
library_root(storage)
target_link_libraries(storage PUBLIC types)

if(WIN32)
    target_link_libraries(ui-window PRIVATE user32 gdi32)
else()
    find_package(Threads REQUIRED)
    target_link_libraries(host PUBLIC Threads::Threads)
endif()

# This build-only aggregate does not create a library-root dependency. Each
# peer root remains independently compilable and names its exact dependencies.
add_library(lib-neutral-corpus INTERFACE)
target_link_libraries(lib-neutral-corpus INTERFACE
    types console host storage ui-base ui-window ui-console)

if(LIBRARY_STRICT_WARNINGS AND CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    foreach(library_target IN ITEMS
            console ui-base ui-window ui-console host storage)
        target_compile_options(${library_target} PRIVATE
            -Wall -Wextra -Wpedantic -Werror)
    endforeach()
endif()

add_custom_target(lib-verify-manifest
    COMMAND "${CMAKE_COMMAND}" -DLIBRARY_ROOT:PATH=${CMAKE_CURRENT_LIST_DIR}
        -P "${CMAKE_CURRENT_LIST_DIR}/verify_manifest.cmake"
    VERBATIM)
if(BUILD_TESTING)
    add_test(NAME library.manifest
        COMMAND "${CMAKE_COMMAND}" -DLIBRARY_ROOT:PATH=${CMAKE_CURRENT_LIST_DIR}
            -P "${CMAKE_CURRENT_LIST_DIR}/verify_manifest.cmake")
    add_test(NAME library.linux-build-contract
        COMMAND "${CMAKE_COMMAND}" -DLIBRARY_CMAKE:FILEPATH=${CMAKE_CURRENT_LIST_FILE}
            -P "${CMAKE_CURRENT_LIST_DIR}/verify_linux_build_contract.cmake")
endif()
