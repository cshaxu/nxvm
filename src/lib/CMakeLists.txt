cmake_minimum_required(VERSION 3.23)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(nxvm_shared_library LANGUAGES C)
    include(CTest)
endif()
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

set(LIBRARY_SOURCE_ROOT "${CMAKE_CURRENT_LIST_DIR}/..")

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

add_library(base STATIC base/sync.c base/clock.c base/process.c)
if(WIN32)
    target_sources(base PRIVATE base/win32/sync.c base/win32/clock.c base/win32/process.c)
else()
    target_sources(base PRIVATE base/linux/sync.c base/linux/clock.c base/linux/process.c)
endif()
library_root(base)
target_link_libraries(base PUBLIC types)

add_library(console STATIC console/console.c)
library_root(console)
target_link_libraries(console PUBLIC types base)

set(LIBRARY_KVM_BASE_SOURCES
    kvm-base/component.c
    kvm-base/hotkey.c
    kvm-base/input.c
    kvm-base/mailbox.c)
if(WIN32)
    list(APPEND LIBRARY_KVM_BASE_SOURCES
        kvm-base/win32/input.c)
else()
    list(APPEND LIBRARY_KVM_BASE_SOURCES
        kvm-base/linux/input.c)
endif()
add_library(kvm-base STATIC ${LIBRARY_KVM_BASE_SOURCES})
library_root(kvm-base)
target_link_libraries(kvm-base PUBLIC types base)

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
target_link_libraries(kvm-console PUBLIC types base console kvm-base)

add_library(console-broker STATIC console-broker/console.c)
if(WIN32)
    target_sources(console-broker PRIVATE console-broker/win32/console.c)
else()
    target_sources(console-broker PRIVATE console-broker/linux/console.c)
endif()
library_root(console-broker)
target_link_libraries(console-broker PUBLIC types base console)

set(LIBRARY_STORAGE_SOURCES storage/file.c storage/medium.c)
if(WIN32)
    list(APPEND LIBRARY_STORAGE_SOURCES storage/win32/file.c)
else()
    list(APPEND LIBRARY_STORAGE_SOURCES storage/linux/file.c)
endif()
add_library(storage STATIC ${LIBRARY_STORAGE_SOURCES})
library_root(storage)
target_link_libraries(storage PUBLIC types)

add_library(audio STATIC audio/stream.c)
if(WIN32)
    target_sources(audio PRIVATE audio/win32/stream.c)
else()
    target_sources(audio PRIVATE audio/linux/stream.c)
endif()
library_root(audio)
target_link_libraries(audio PUBLIC types base)

if(WIN32)
    target_link_libraries(audio PRIVATE ole32)
    target_link_libraries(kvm-base PRIVATE user32)
    target_link_libraries(kvm-window PRIVATE user32 gdi32)
    target_link_libraries(console-broker PRIVATE user32)
else()
    find_package(Threads REQUIRED)
    target_link_libraries(base PRIVATE Threads::Threads)
endif()

# This build-only aggregate does not create a library-root dependency. Each
# peer root remains independently compilable and names its exact dependencies.
add_library(lib-neutral-corpus INTERFACE)
target_link_libraries(lib-neutral-corpus INTERFACE
    types base console console-broker storage audio kvm-base kvm-window kvm-console)

add_custom_target(lib-verify-manifest
    COMMAND "${CMAKE_COMMAND}" -DLIBRARY_ROOT:PATH=${CMAKE_CURRENT_LIST_DIR}
        -P "${CMAKE_CURRENT_LIST_DIR}/verify_manifest.cmake"
    VERBATIM)
