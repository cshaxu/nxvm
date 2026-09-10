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
    target_include_directories(${target} PUBLIC "${LIBRARY_SOURCE_ROOT}")
endfunction()

add_library(base-console STATIC base/console.c)
library_root(base-console)

set(LIBRARY_UX_BASE_SOURCES
    ux-base/component.c
    ux-base/hotkey.c
    ux-base/mailbox.c
    ux-base/linux/input.c)
if(WIN32)
    list(APPEND LIBRARY_UX_BASE_SOURCES
        ux-base/win32/mailbox.c
        ux-base/win32/color.c
        ux-base/win32/actions.c
        ux-base/win32/input.c)
else()
    list(APPEND LIBRARY_UX_BASE_SOURCES ux-base/linux/mailbox.c)
endif()
add_library(ux-base STATIC ${LIBRARY_UX_BASE_SOURCES})
library_root(ux-base)
target_link_libraries(ux-base PUBLIC base-console)

add_library(ux-window STATIC ux-window/window.c)
if(WIN32)
    target_sources(ux-window PRIVATE ux-window/win32/geometry.c
        ux-window/win32/mouse.c ux-window/win32/component.c)
else()
    target_sources(ux-window PRIVATE ux-window/linux/component.c)
endif()
library_root(ux-window)
target_link_libraries(ux-window PUBLIC base-console ux-base)

add_library(ux-console STATIC ux-console/console.c)
if(WIN32)
    target_sources(ux-console PRIVATE ux-console/win32/component.c)
else()
    target_sources(ux-console PRIVATE ux-console/linux/component.c)
endif()
library_root(ux-console)
target_link_libraries(ux-console PUBLIC base-console ux-base)

add_library(ux INTERFACE)
target_link_libraries(ux INTERFACE ux-base ux-window ux-console)
add_library(ux-contract ALIAS ux-base)
add_library(ux-native ALIAS ux)

if(WIN32)
    set(LIBRARY_HOST_SOURCES host/clock.c host/console.c host/win32/clock.c host/win32/sync.c host/win32/console.c)
else()
    set(LIBRARY_HOST_SOURCES host/clock.c host/console.c host/linux/clock.c host/linux/sync.c host/linux/console.c)
endif()
add_library(host-sync STATIC ${LIBRARY_HOST_SOURCES})
library_root(host-sync)
target_link_libraries(host-sync PUBLIC base-console)

set(LIBRARY_STORAGE_SOURCES storage/file.c storage/medium.c)
if(WIN32)
    list(APPEND LIBRARY_STORAGE_SOURCES storage/win32/file.c)
else()
    list(APPEND LIBRARY_STORAGE_SOURCES storage/linux/file.c)
endif()
add_library(storage-medium STATIC ${LIBRARY_STORAGE_SOURCES})
library_root(storage-medium)

if(WIN32)
    add_library(ux-win32-native ALIAS ux)
    target_link_libraries(ux-window PRIVATE user32 gdi32)
else()
    add_library(ux-linux-native ALIAS ux)
    find_package(Threads REQUIRED)
    target_link_libraries(host-sync PUBLIC Threads::Threads)
endif()

# This build-only aggregate does not create a library-root dependency. Each
# peer root remains independently compilable and depends only on `base`.
add_library(lib-neutral-corpus INTERFACE)
target_link_libraries(lib-neutral-corpus INTERFACE base-console ux host-sync storage-medium)

if(LIBRARY_STRICT_WARNINGS AND CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    foreach(library_target IN ITEMS
            base-console ux-base ux-window ux-console host-sync storage-medium)
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
