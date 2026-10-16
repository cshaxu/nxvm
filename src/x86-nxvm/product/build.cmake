# Fixed identity, source and firmware roles are supplied by the consuming App.
include_guard(GLOBAL)

function(ibmpc_embed_firmware target output BIOS_0 BIOS_1 VIDEO CMOS FONT)
    set(inputs)
    set(arguments)
    foreach(role IN ITEMS BIOS_0 BIOS_1 VIDEO CMOS FONT)
        list(APPEND arguments "-DINPUT_${role}=${${role}}")
        if(NOT "${${role}}" STREQUAL "LIB_NULL")
            list(APPEND inputs "${${role}}")
        endif()
    endforeach()
    add_custom_command(OUTPUT "${output}"
        COMMAND "${CMAKE_COMMAND}" ${arguments} "-DOUTPUT=${output}"
            -P "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/embed_firmware.cmake"
        DEPENDS ${inputs} "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/embed_firmware.cmake"
        COMMENT "Embedding selected PC firmware" VERBATIM)
    add_library(${target} STATIC EXCLUDE_FROM_ALL "${output}")
    target_link_libraries(${target} PRIVATE types)
    target_include_directories(${target} PRIVATE "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../..")
endfunction()

function(ibmpc_add_product target entry filename architecture directory)
    if(NOT architecture MATCHES "^(x64|x86)$")
        message(FATAL_ERROR "PC product architecture must be x64 or x86")
    endif()
    add_executable(${target} EXCLUDE_FROM_ALL "${entry}")
    target_link_libraries(${target} PRIVATE ${ARGN})
    if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
    if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
        target_link_options(${target} PRIVATE -Wl,--strip-debug)
    endif()
    if(architecture STREQUAL "x86")
        target_link_options(${target} PRIVATE -Wl,--stack,2097152)
    endif()
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_custom_command(TARGET ${target} PRE_LINK
            COMMAND "${CMAKE_COMMAND}" "-DPROJECT_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
                -P "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/verify_artifact_optimized.cmake"
            VERBATIM)
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND "${CMAKE_COMMAND}" "-DPROJECT_ARTIFACT_PATH=$<TARGET_FILE:${target}>"
                "-DPROJECT_ARTIFACT_ARCHITECTURE=${architecture}"
                "-DPROJECT_ARTIFACT_FILENAME=${filename}"
                "-DPROJECT_ARTIFACT_DIRECTORY=${directory}"
                -P "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/deploy_artifact.cmake"
            VERBATIM)
    endif()
endfunction()
