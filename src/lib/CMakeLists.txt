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
if(WIN32)
    target_sources(console PRIVATE console/win32/mutex.c)
else()
    target_sources(console PRIVATE console/linux/mutex.c)
endif()
library_root(console)
target_link_libraries(console PUBLIC types)

set(LIBRARY_KVM_BASE_SOURCES
    kvm-base/component.c
    kvm-base/hotkey.c
    kvm-base/input.c
    kvm-base/mailbox.c)
if(WIN32)
    list(APPEND LIBRARY_KVM_BASE_SOURCES
        kvm-base/win32/mailbox.c
        kvm-base/win32/input.c)
else()
    list(APPEND LIBRARY_KVM_BASE_SOURCES
        kvm-base/linux/mailbox.c
        kvm-base/linux/input.c)
endif()
add_library(kvm-base STATIC ${LIBRARY_KVM_BASE_SOURCES})
library_root(kvm-base)
target_link_libraries(kvm-base PUBLIC types)

add_library(kvm-window STATIC kvm-window/window.c kvm-window/geometry.c kvm-window/render.c kvm-window/motion.c)
if(WIN32)
    target_sources(kvm-window PRIVATE kvm-window/win32/geometry.c kvm-window/win32/input.c
        kvm-window/win32/mouse.c kvm-window/win32/component.c)
else()
    target_sources(kvm-window PRIVATE kvm-window/linux/component.c)
endif()
library_root(kvm-window)
target_link_libraries(kvm-window PUBLIC types kvm-base)

add_library(kvm-console STATIC kvm-console/console.c)
if(WIN32)
    target_sources(kvm-console PRIVATE kvm-console/win32/component.c)
else()
    target_sources(kvm-console PRIVATE kvm-console/linux/component.c)
endif()
library_root(kvm-console)
target_link_libraries(kvm-console PUBLIC types console kvm-base)

if(WIN32)
    set(LIBRARY_HOST_SOURCES host/clock.c host/console.c host/sync.c
        host/win32/clock.c host/win32/console.c host/win32/sync.c)
else()
    set(LIBRARY_HOST_SOURCES host/clock.c host/console.c host/sync.c
        host/linux/clock.c host/linux/console.c host/linux/sync.c)
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
    target_link_libraries(kvm-base PRIVATE user32)
    target_link_libraries(kvm-window PRIVATE user32 gdi32)
    target_link_libraries(host PRIVATE user32)
else()
    find_package(Threads REQUIRED)
    target_link_libraries(console PRIVATE Threads::Threads)
    target_link_libraries(host PRIVATE Threads::Threads)
    target_link_libraries(kvm-base PRIVATE Threads::Threads)
endif()

# This build-only aggregate does not create a library-root dependency. Each
# peer root remains independently compilable and names its exact dependencies.
add_library(lib-neutral-corpus INTERFACE)
target_link_libraries(lib-neutral-corpus INTERFACE
    types console host storage kvm-base kvm-window kvm-console)

if(LIBRARY_STRICT_WARNINGS AND CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    foreach(library_target IN ITEMS
            console kvm-base kvm-window kvm-console host storage)
        target_compile_options(${library_target} PRIVATE
            -Wall -Wextra -Wpedantic -Werror)
    endforeach()
endif()

add_custom_target(lib-verify-manifest
    COMMAND "${CMAKE_COMMAND}" -DLIBRARY_ROOT:PATH=${CMAKE_CURRENT_LIST_DIR}
        -P "${CMAKE_CURRENT_LIST_DIR}/verify_manifest.cmake"
    VERBATIM)
