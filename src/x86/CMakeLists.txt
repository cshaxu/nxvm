cmake_minimum_required(VERSION 3.23)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(nxvm_shared_x86 LANGUAGES C)
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
option(X86_BUILD_TOOLS "Build xasm32 and Debug alongside device components" ON)
if(X86_BUILD_TOOLS AND NOT TARGET common-machine)
    add_subdirectory("${X86_COMMON_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}/common")
endif()

add_library(x86-pit825x STATIC devices/pit825x/pit.c)
target_include_directories(x86-pit825x PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(x86-pit825x PUBLIC types)

add_library(x86-rtc146818 STATIC devices/rtc146818/rtc.c)
target_include_directories(x86-rtc146818 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(x86-rtc146818 PUBLIC types)

add_library(x86-pic8259 STATIC devices/pic8259/pic.c)
target_include_directories(x86-pic8259 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(x86-pic8259 PUBLIC types)

add_library(x86-dma8237 STATIC devices/dma8237/dma.c)
target_include_directories(x86-dma8237 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(x86-dma8237 PUBLIC types)

add_library(x86-fdc8272 STATIC devices/fdc8272/fdc.c)
target_include_directories(x86-fdc8272 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(x86-fdc8272 PUBLIC types)

add_library(x86-hdc STATIC devices/hdc/hdc.c)
target_include_directories(x86-hdc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(x86-hdc PUBLIC types)

add_library(x86-video STATIC devices/video/video.c)
target_include_directories(x86-video PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(x86-video PUBLIC types)

add_library(x86-ps2mouse STATIC devices/ps2mouse/mouse.c)
target_include_directories(x86-ps2mouse PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(x86-ps2mouse PUBLIC types)

add_library(x86-keyboard STATIC devices/keyboard/keyboard.c)
target_include_directories(x86-keyboard PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(x86-keyboard PUBLIC types)

add_library(x86-kbc8042 STATIC devices/kbc8042/controller.c)
target_include_directories(x86-kbc8042 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(x86-kbc8042 PUBLIC types PRIVATE x86-keyboard)

add_library(x86-ppi8255 STATIC devices/ppi8255/ppi.c)
target_include_directories(x86-ppi8255 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(x86-ppi8255 PUBLIC types)

add_library(x86-xtkeyboard STATIC devices/xtkeyboard/keyboard.c)
target_include_directories(x86-xtkeyboard PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(x86-xtkeyboard PUBLIC types)

if(X86_BUILD_TOOLS)
add_library(x86-xasm32 STATIC xasm32/xasm32.c xasm32/aasm32.c xasm32/dasm32.c)
target_include_directories(x86-xasm32 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(x86-xasm32 PUBLIC types)

add_library(x86-debug STATIC debug/command.c)
target_include_directories(x86-debug PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(x86-debug PUBLIC common-machine x86-xasm32 storage)
endif()

add_custom_target(x86-verify
    COMMAND "${CMAKE_COMMAND}" -DCOMMON_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P "${X86_COMMON_ROOT}/verify_manifest.cmake"
    COMMAND "${CMAKE_COMMAND}" -DX86_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P "${CMAKE_CURRENT_SOURCE_DIR}/verify_corpus.cmake"
    VERBATIM)
