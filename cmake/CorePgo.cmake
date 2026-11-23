set(REPOSITORY_CORE_PGO_DIRECTORY "" CACHE PATH
    "GCC Release profile counters for the Core instruction and memory objects")
if(REPOSITORY_CORE_PGO_DIRECTORY STREQUAL "")
    return()
endif()

if(NOT repository_core_ipo OR NOT CMAKE_GENERATOR STREQUAL "Ninja" OR
   NOT CMAKE_BUILD_TYPE STREQUAL "Release")
    message(FATAL_ERROR "Core PGO requires Windows GCC Release IPO and Ninja.")
endif()

file(REAL_PATH "${REPOSITORY_CORE_PGO_DIRECTORY}" profile_directory
    BASE_DIRECTORY "${CMAKE_BINARY_DIR}")
cmake_path(IS_PREFIX CMAKE_BINARY_DIR "${profile_directory}" NORMALIZE current_tree)
if(NOT current_tree OR profile_directory STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "Core PGO counters must be in a subdirectory of this build tree.")
endif()
cmake_path(NATIVE_PATH CMAKE_BINARY_DIR NORMALIZE profile_prefix)

function(repository_core_profile target source)
    string(REPLACE "/" "#" counter_source "${source}")
    set(counter "${profile_directory}/src#x86-core#CMakeFiles#${target}.dir#${counter_source}.gcda")
    if(NOT EXISTS "${counter}")
        message(FATAL_ERROR "Missing Core PGO counter: ${counter}")
    endif()
    set(path "${CMAKE_SOURCE_DIR}/src/x86-core/${source}")
    set_property(SOURCE "${path}" TARGET_DIRECTORY ${target} APPEND PROPERTY OBJECT_DEPENDS "${counter}")
    set_property(SOURCE "${path}" TARGET_DIRECTORY ${target} APPEND PROPERTY COMPILE_OPTIONS
        "$<$<STREQUAL:$<TARGET_PROPERTY:NAME>,${target}>:-fprofile-use=${profile_directory};-fprofile-prefix-path=${profile_prefix};-fprofile-partial-training;-Werror=missing-profile;-Werror=coverage-mismatch>")
endfunction()

repository_core_profile(x86-core-cpu cpu/cpu_instructions.c)
repository_core_profile(x86-core-machine machine/memory.c)
