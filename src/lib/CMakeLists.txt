cmake_minimum_required(VERSION 3.23)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(nxvm_shared_library LANGUAGES C)
    set(CMAKE_C_STANDARD 11)
    set(CMAKE_C_STANDARD_REQUIRED ON)
    set(CMAKE_C_EXTENSIONS OFF)
    include(CTest)
endif()

set(LIBRARY_SOURCE_ROOT "${CMAKE_CURRENT_LIST_DIR}/..")

function(library_root target)
    target_include_directories(${target} PUBLIC "${LIBRARY_SOURCE_ROOT}")
endfunction()

set(LIBRARY_UX_SOURCES
    ux/actions.c
    ux/linux/input.c
    ux/mailbox.c
    ux/presenter.c
    ux/router.c)
if(WIN32)
    list(APPEND LIBRARY_UX_SOURCES
        ux/win32/mailbox.c
        ux/win32/actions.c
        ux/win32/console.c
        ux/win32/geometry.c
        ux/win32/input.c
        ux/win32/mouse.c
        ux/win32/runner.c
        ux/win32/window.c)
else()
    list(APPEND LIBRARY_UX_SOURCES ux/linux/mailbox.c ux/linux/console.c)
endif()
add_library(ux STATIC ${LIBRARY_UX_SOURCES})
library_root(ux)
add_library(ux-contract ALIAS ux)
add_library(ux-native ALIAS ux)

if(WIN32)
    set(LIBRARY_HOST_SOURCES host/clock.c host/win32/clock.c host/win32/sync.c)
else()
    set(LIBRARY_HOST_SOURCES host/clock.c host/linux/clock.c host/linux/sync.c)
endif()
add_library(host-sync STATIC ${LIBRARY_HOST_SOURCES})
library_root(host-sync)

add_library(session-lifecycle STATIC session/state.c session/executor.c)
library_root(session-lifecycle)

set(LIBRARY_STORAGE_SOURCES storage/commit.c storage/file.c storage/image.c)
if(WIN32)
    list(APPEND LIBRARY_STORAGE_SOURCES storage/win32/file.c)
else()
    list(APPEND LIBRARY_STORAGE_SOURCES storage/linux/file.c)
endif()
add_library(storage-image STATIC ${LIBRARY_STORAGE_SOURCES})
library_root(storage-image)

add_library(observability-outcome STATIC observability/outcome.c)
library_root(observability-outcome)

if(WIN32)
    add_library(ux-win32-native ALIAS ux)
else()
    add_library(ux-linux-native ALIAS ux)
    find_package(Curses REQUIRED)
    find_package(Threads REQUIRED)
    target_include_directories(ux PUBLIC ${CURSES_INCLUDE_DIRS})
    target_link_libraries(ux PUBLIC ${CURSES_LIBRARIES} Threads::Threads)
endif()

# This build-only aggregate does not create a library-root dependency. Each
# peer root remains independently compilable and depends only on `base`.
add_library(lib-neutral-corpus INTERFACE)
target_link_libraries(lib-neutral-corpus INTERFACE
    ux host-sync session-lifecycle storage-image observability-outcome)

add_custom_target(lib-verify-manifest
    COMMAND "${CMAKE_COMMAND}" -DLIBRARY_ROOT:PATH=${CMAKE_CURRENT_LIST_DIR}
        -P "${CMAKE_CURRENT_LIST_DIR}/verify_manifest.cmake"
    VERBATIM)
if(BUILD_TESTING)
    add_test(NAME library.manifest
        COMMAND "${CMAKE_COMMAND}" -DLIBRARY_ROOT:PATH=${CMAKE_CURRENT_LIST_DIR}
            -P "${CMAKE_CURRENT_LIST_DIR}/verify_manifest.cmake")
endif()
